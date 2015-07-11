/*
 * leds.c
 * (c) 2015 George Louthan
 * 3-clause BSD license; see license.md.
 */

/*
 *   LED controller (TLC5948A)
 *        (write on rise, change on fall,
 *         clock inactive low, MSB first)
 *        eUSCI_A0 - LEDs  (shared)
 *        somi, miso, clk
 *        GSCLK     P1.2 (timer TA1.1)
 *        LAT       P1.4
 */

#include "qc12.h"
#include "leds.h"
#include <string.h>

// Current TLC sending state:
uint8_t tlc_send_type = TLC_SEND_IDLE;

uint8_t tlc_tx_index = 0;   // Index of currently sending buffer
uint8_t tlc_ok_to_send = 1; // SPI bus busy or not?
uint8_t tlc_shift_px = 0;   // Number of channels to shift gs_data by

// Index of the current GS entry to send (counts down):
volatile uint8_t gs_index = 14;

// Buffers containing actual data to send to the TLC:

// Utility light:
uint16_t tlc_tx_light = 0xffff;

uint16_t gs_data[15] = {
		// Mid-right (front view)
		0x1fff, // R
		0x0000, // G
		0x0000, // B
		// Bottom-right
		0x1f00, // R
		0x1f00, // G
		0x0000, // B
		// Bottom-left
		0x0000, // R
		0x1fff, // G
		0x0000, // B
		// Mid-left
		0x0000, // R
		0x0000, // G
		0x1ff0, // B
		// Top-middle
		0x1f00, // R
		0x0f00, // G
		0x0a00, // B
};

uint8_t fun_base[] = {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00, // ...reserved...
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        // B135 / PSM(D1)       0
        // B134 / PSM(D0)       0
        // B133 / OLDENA        0
        // B132 / IDMCUR(D1)    0
        // B131 / IDMCUR(D0)    0
        // B130 / IDMRPT(D0)    0
        // B129 / IDMENA        0
        // B128 / LATTMG(D1)    1:
        0x01,
        // B127 / LATTMG(D0)    1
        // B126 / LSDVLT(D1)    0
        // B125 / LSDVLT(D0)    0
        // B124 / LODVLT(D1)    0
        // B123 / LODVLT(D0)    0
        // B122 / ESPWM         1
        // B121 / TMGRST        1
        // B120 / DSPRPT        1:
        0x87,
        // B119 / BLANK
        // and 7 bits of global brightness correction:
        0x08,
        // HERE WE SWITCH TO 7-BIT SPI.
        // The following index is 18:
        0x4F, // rgb0
        0x7F,
        0x7F,
        0x4F, // rgb1
        0x7F,
        0x7F,
        0x4F, // rgb2
        0x7F,
        0x7F,
        0x4F, // rgb3
        0x7F,
        0x7F,
        0x4F, // rgb4
        0x7F,
        0x7F,
        0x7F, // tx
};

void init_tlc() {
    // TODO: Shouldn't really need to do this:
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;

    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    tlc_stage_blank(1);
//    tlc_set_fun();
}

void tlc_set_gs(uint8_t shift_amt) {
    while (tlc_send_type != TLC_SEND_IDLE); // shouldn't ever actually have to block on this please.
    tlc_shift_px = shift_amt;
    tlc_send_type = TLC_SEND_TYPE_GS;
    tlc_tx_index = 0;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, 0x00);
}

void tlc_set_fun() {
    while (tlc_send_type != TLC_SEND_IDLE); // shouldn't ever actually have to block on this please.
    tlc_send_type = TLC_SEND_TYPE_FUN;
    tlc_tx_index = 0;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, 0x01);
}

// Stage the blank bit:
void tlc_stage_blank(uint8_t blank) {
    if (blank) {
        fun_base[17] |= BIT7;
        fun_base[16] &= ~BIT1;
    } else {
        fun_base[17] &= ~BIT7;
        fun_base[16] |= BIT1;
    }
}

// Stage global brightness if different from default:
void tlc_stage_bc(uint8_t bc) {
    bc = bc & 0b01111111; // Mask out BLANK just in case.
    fun_base[17] &= 0b10000000;
    fun_base[17] |= bc;
}

void tlc_timestep() {
    static uint8_t shift = 0;
    tlc_set_gs(shift);
    shift = (shift + 3) % 15;
}

#pragma vector=USCI_A0_VECTOR
__interrupt void EUSCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, 4)) {
    //Vector 2 - RXIFG
    case 2:
        // We received some garbage sent to us while we were sending.
        EUSCI_B_SPI_receiveData(EUSCI_A0_BASE); // Throw it away.
        break; // End of RXIFG ///////////////////////////////////////////////////////

    case 4: // Vector 4 - TXIFG : I just sent a byte.
        if (tlc_send_type == TLC_SEND_TYPE_GS) {
            if (tlc_tx_index == 0) { // txl, msb
                EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_tx_light >> 8));
            } else if (tlc_tx_index == 1) { // txl, lsb
                EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_tx_light & 0x00ff));
                gs_index = 14;
            } else if (tlc_tx_index == 32) { // done
                GPIO_pulse(TLC_LATPORT, TLC_LATPIN); // LATCH.
                tlc_send_type = TLC_SEND_IDLE;
                break;
            } else { // gs
                // gs_index here starts at 14
                // tx_index here starts at 2
                // gs_index can get inced when tx_index is odd.

                // So if gs_index=14 and tx_index=2 we send
                // gs_data[14] >> 8
                // and gs_index=15 and tx_index=3 we send
                // gs_data[14] & 0xff

                if (tlc_tx_index & 0x01) { // it's odd (LSB)
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (gs_data[(gs_index + (15-tlc_shift_px)) % 15] & 0x00ff));
                    gs_index--;
                } else { // it's even:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (gs_data[(gs_index + (15-tlc_shift_px)) % 15] >> 8));
                }
            }
            tlc_tx_index++;
        } else if (tlc_send_type == TLC_SEND_TYPE_FUN) {
            if (tlc_tx_index == 18) { // after 18 we have to switch to 7-bit mode.
                UCA0CTLW0 |= UCSWRST;
                UCA0CTLW0 |= UC7BIT;
                UCA0CTLW0 &= ~UCSWRST;
                EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
            } else if (tlc_tx_index == 33) {
                GPIO_pulse(TLC_LATPORT, TLC_LATPIN); // LATCH.
                UCA0CTLW0 |= UCSWRST;
                UCA0CTLW0 &= ~UC7BIT;
                UCA0CTLW0 &= ~UCSWRST;
                EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                tlc_send_type = TLC_SEND_IDLE;
                break;
            }
            EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, fun_base[tlc_tx_index]);
            tlc_tx_index++;
        } else {
            tlc_send_type = TLC_SEND_IDLE; // TODO: probably shouldn't reach.
        }
        break; // End of TXIFG /////////////////////////////////////////////////////

    default: break;
    } // End of ISR flag switch ////////////////////////////////////////////////////
}

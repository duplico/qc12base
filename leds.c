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

#define TLC_THISISGS    0x00
#define TLC_THISISFUN   0x01

// Current TLC sending state:
uint8_t tlc_send_type = TLC_SEND_IDLE;

uint8_t tlc_tx_index = 0;   // Index of currently sending buffer
uint8_t tlc_ok_to_send = 1; // SPI bus busy or not?
uint8_t tlc_shift_px = 0;   // Number of channels to shift gs_data by

// Index of the current GS entry to send (counts down):
volatile uint8_t rgb_element_index = 14;

#define TLC_ANIM_MODE_IDLE  0
#define TLC_ANIM_MODE_SHIFT 1
#define TLC_ANIM_MODE_SAME  2

uint8_t led_anim_mode = TLC_ANIM_MODE_IDLE;
uint8_t led_anim_index = 0;

// Utility light:
uint16_t tlc_tx_light = 0xffff;

rgbcolor_t *tlc_curr_colors;
uint8_t tlc_curr_colors_len;

rgbcolor_t rainbow1[5] = {
        // rainbow colors
        { 0xff00, 0x00, 0x00 },
        { 0x00, 0xff00, 0x00 },
        { 0x00, 0x00, 0xff00 },
        { 0xe00, 0xe00, 0xe00 },
        { 0x00, 0x00, 0x000 },
};

// Buffers containing actual data to send to the TLC:

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
    tlc_set_fun();
}

// TODO: badly needs to be simplified:
uint8_t tlc_color_index = 0;

// This is the rolling one:
void tlc_set_gs(uint8_t starting_frame) {
    while (tlc_send_type != TLC_SEND_IDLE);
    tlc_shift_px = starting_frame;
    led_anim_mode = TLC_ANIM_MODE_SHIFT;
//    led_anim_mode = TLC_ANIM_MODE_SAME;
    tlc_send_type = TLC_SEND_TYPE_GS;
    tlc_tx_index = 0;
    tlc_curr_colors = rainbow1;
    tlc_curr_colors_len = 5;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, TLC_THISISGS);
}

//void tlc_set_gs(uint8_t shift_amt) {
//    while (tlc_send_type != TLC_SEND_IDLE); // shouldn't ever actually have to block on this please.
//    tlc_shift_px = shift_amt;
//    tlc_send_type = TLC_SEND_TYPE_GS;
//    tlc_tx_index = 0;
//    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, TLC_THISISGS);
//}

void tlc_set_fun() {
    while (tlc_send_type != TLC_SEND_IDLE); // shouldn't ever actually have to block on this please.
    tlc_send_type = TLC_SEND_TYPE_FUN;
    tlc_tx_index = 0;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, TLC_THISISFUN);
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
    shift = (shift+1) % tlc_curr_colors_len;
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
                rgb_element_index = 0;
                tlc_color_index = tlc_shift_px % tlc_curr_colors_len;
            } else if (tlc_tx_index == 32) { // done
                GPIO_pulse(TLC_LATPORT, TLC_LATPIN); // LATCH.
                tlc_send_type = TLC_SEND_IDLE;
                break;
            } else { // gs
                switch(rgb_element_index % 6) { // WATCH OUT! Weird order below:
                case 1:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_curr_colors[tlc_color_index].blue & 0x00ff));
                    break;
                case 0:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_curr_colors[tlc_color_index].blue >> 8));
                    break;
                case 3:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_curr_colors[tlc_color_index].green & 0x00ff));
                    break;
                case 2:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_curr_colors[tlc_color_index].green >> 8));
                    break;
                case 4:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_curr_colors[tlc_color_index].red >> 8));
                    break;
                case 5:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_curr_colors[tlc_color_index].red & 0x00ff));

                    if (led_anim_mode == TLC_ANIM_MODE_SHIFT) {
                        tlc_color_index = (tlc_color_index + 1) % tlc_curr_colors_len;
                    } // otherwise we keep the same color for all the RGB LEDs.
                    break;
                }
                rgb_element_index++;
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

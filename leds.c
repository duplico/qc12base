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

// Defines for the TLC:
#define LATPORT     GPIO_PORT_P1
#define LATPIN      GPIO_PIN4

uint16_t led_tx_light = 0xffff;

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
		0x0000,
		0x0000,
		0x1ff0,
		// Top-middle
		0x1f00,
		0x0f00,
		0x0a00,
};

#define SEND_TYPE_GS  1
#define SEND_TYPE_DAT 2
uint8_t send_type = SEND_TYPE_DAT;
uint8_t tx_index = 0;

uint8_t led_ok_to_send = 1;

void usci_a_send(uint16_t base, uint8_t data) {
    while (!led_ok_to_send);
    led_ok_to_send = 0;
    EUSCI_A_SPI_transmitData(base, data);
    while (!led_ok_to_send);
}

uint8_t shift = 0;

void tlc_set_gs(uint8_t shift_amt) {
    shift = shift_amt;
    send_type = SEND_TYPE_GS;
    tx_index = 0;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, 0x00);
}

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
        // B121 / TMGRST        0
        // B120 / DSPRPT        1:
        0x85,
        // B119 / BLANK
        // remainder are BC:
        0x88,
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

void tlc_stage_blank(uint8_t blank) {
}

void tlc_stage_bc(uint8_t bc) {
}

void tlc_set_fun(uint8_t blank) {
    send_type = SEND_TYPE_DAT;

    usci_a_send(EUSCI_A0_BASE, 0x01); // 1 for Function

    for (uint8_t i=0; i<14; i++) {
        usci_a_send(EUSCI_A0_BASE, 0x00);
    }

    usci_a_send(EUSCI_A0_BASE, 0x00); // LSB of this is PSM(D2)


    usci_a_send(EUSCI_A0_BASE, 0x01);


    usci_a_send(EUSCI_A0_BASE, 0b10000101); // 0x85 (ESPWM on)

    // B119 / BLANK
    // MSB is BLANK; remainder are BC:
    usci_a_send(EUSCI_A0_BASE, 0x08 + (blank << 7));


    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;
    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    // 5 dot-correct 3x 7-tets for the RGBs:
    for (uint8_t i=0; i<5; i++) {
    	usci_a_send(EUSCI_A0_BASE, 0x4F); // RED
    	usci_a_send(EUSCI_A0_BASE, 0x7F); // BLUE
    	usci_a_send(EUSCI_A0_BASE, 0x7F); // GREEN
    }

    // Plus one for the TX light:
    usci_a_send(EUSCI_A0_BASE, 0x7F);








    // WHEN FINISHED:
    // LATCH:
    GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4);

    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;
    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
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
        if (send_type == SEND_TYPE_GS) {
            if (tx_index == 0) { // txl, msb
                EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (led_tx_light >> 8));
            } else if (tx_index == 1) { // txl, lsb
                EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (led_tx_light & 0x00ff));
            } else if (tx_index == 32) { // done
                GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4); // LATCH.
                led_ok_to_send = 1;
            } else { // gs
                // ((tx_index/2) + shift) % 15
                if (tx_index & 0x01) { // if it's odd (LSB)
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (gs_data[((tx_index/2) + shift) % 15] & 0x00ff));
                } else { // even (MSB)
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (gs_data[((tx_index/2) + shift) % 15] >> 8));
                }
            }
        } else {
            led_ok_to_send = 1;
            break;
        }
        tx_index++;
        break; // End of TXIFG /////////////////////////////////////////////////////

    default: break;
    } // End of ISR flag switch ////////////////////////////////////////////////////
}

void init_leds() {

    // TODO: Shouldn't really need to do this:
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;

    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    tlc_set_fun(1);
    tlc_set_gs(0);
    tlc_set_fun(0);
}

void led_enable(uint16_t duty_cycle) {
}

void led_disable( void )
{
}

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

void tlc_set_gs(uint8_t shift) {
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT));

    // We need a 0 to show it's GS:
    usci_a_send(EUSCI_A0_BASE, 0x00);
    // Now the GS data itself.

    // Status light:
    usci_a_send(EUSCI_A0_BASE, (uint8_t) (led_tx_light >> 8));
    usci_a_send(EUSCI_A0_BASE, (uint8_t) (led_tx_light & 0x00ff));

    // 5 RGB LEDs:
    for (uint8_t channel=15; channel; channel--) {
    	usci_a_send(EUSCI_A0_BASE, (uint8_t) (gs_data[(shift+channel-1) % 15] >> 8));
    	usci_a_send(EUSCI_A0_BASE, (uint8_t) (gs_data[(shift+channel-1) % 15] & 0x00ff));
    }

    // LATCH:
    GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4);
}

void tlc_set_fun(uint8_t blank) {
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT));

    usci_a_send(EUSCI_A0_BASE, 0x01); // 1 for Function

    for (uint8_t i=0; i<14; i++) {
        usci_a_send(EUSCI_A0_BASE, 0x00);
    }

    usci_a_send(EUSCI_A0_BASE, 0x00); // LSB of this is PSM(D2)

    // B135 / PSM(D1)
    // B134 / PSM(D0)
    // B133 / OLDENA
    // B132 / IDMCUR(D1)
    // B131 / IDMCUR(D0)
    // B130 / IDMRPT(D0)
    // B129 / IDMENA
    // B128 / LATTMG(D1)

    usci_a_send(EUSCI_A0_BASE, 0x01);

    // B127 / LATTMG(D0)
    // B126 / LSDVLT(D1)
    // B125 / LSDVLT(D0)
    // B124 / LODVLT(D1)
    // B123 / LODVLT(D0)
    // B122 / ESPWM
    // B121 / TMGRST
    // B120 / DSPRPT

    usci_a_send(EUSCI_A0_BASE, 0x85);

    // B119 / BLANK
    // MSB is BLANK; remainder are BC:
    usci_a_send(EUSCI_A0_BASE, 0x08 + (blank << 7));


    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 |= UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;

    // 5 dot-correct 3x 7-tets for the RGBs:
    for (uint8_t i=0; i<5; i++) {
    	usci_a_send(EUSCI_A0_BASE, 0x4F); // RED
    	usci_a_send(EUSCI_A0_BASE, 0x7F); // BLUE
    	usci_a_send(EUSCI_A0_BASE, 0x7F); // GREEN
    }

    // Plus one for the TX light:
    usci_a_send(EUSCI_A0_BASE, 0x7F);

    // 16 dot-correct 7-tets in 14 octets:
//    for (uint8_t i=0; i<14; i++) {
//        usci_a_send(EUSCI_A0_BASE, 0xFF);
//    }

    // LATCH:
    GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4);

    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;
}

void init_leds() {

    // TODO: Shouldn't really need to do this:
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;

    tlc_set_fun(1);
    tlc_set_gs(0);
    tlc_set_fun(0);
}

void led_enable(uint16_t duty_cycle) {
}

void led_disable( void )
{
}

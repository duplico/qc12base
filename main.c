/*
 * ======== Standard MSP430 includes ========
 */
#include <msp430fr5949.h>
#include <driverlib/MSP430FR5xx_6xx/driverlib.h>
#include <stdint.h>
#include <grlib.h>
#include <qc12_oled.h>
#include <stdio.h>

// Grace includes:
#include <ti/mcu/msp430/Grace.h>

// Project includes:
#include "img.h"
#include "qc12.h"
#include "radio.h"

/*
 * Peripherals:
 *
 *  Radio (RFM69CW)
 *        (MSB first, clock inactive low,
 *         write on rise, change on fall, MSB first)
 *        eUSCI_B0 - radio
 *        somi, miso, clk, ste
 *        DIO0      P3.1
 *        RESET     P3.2
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

/*
 *   Flash chip (S25FL216K)
 *        (write on rise, change on fall,
 *         CS active low, clock inactive low, MSB first)
 *        eUSCI_A0 (shared)
 *        somi, miso, clk
 *        CS        P1.1
 *        WP        P3.0
 *        HOLD      P1.0
 */

/*
 *   OLED (OLED_0.96)
 *        (write on rise, change on fall,
 *         CS active low, MSB first)
 *        eUSCI_A1
 *        ste, miso, clk
 *        DC        P2.6
 *        RES       P2.7
 */

/*
 *   Buttons
 *   (active low)
 *   BTN1      P3.6
 *   BTN2      P3.5
 *   BTN3      P3.4
 */
    // CS   1.3 (STE)
    // RES  1.5 (pull low to reset, otherwise high)
    // D/C  1.7
    // D0   2.2 (CLK)
    // D1   1.6 (MOSI)

// Defines for the TLC:
#define LATPORT     GPIO_PORT_P1
#define LATPIN      GPIO_PIN4

tContext g_sContext;


void tlc_bit(uint8_t bit) {
     // CLK should be low.
     P1OUT &= ~BIT5;
     // Set it to the bit:
     if (bit) {
          P2OUT |= BIT0;
     } else {
          P2OUT &= ~BIT0;
     }
     // Pull CLK high
     P1OUT |= BIT5;
}

uint8_t tlc_in_bit() {
     // CLK should be low.
     P1OUT &= ~BIT5;
     // Pulse CLK high to clock it in.
     P1OUT |= BIT5;
     return P2IN & BIT1;
}

void usci_a_send(uint16_t base, uint8_t data) {
	while (EUSCI_A_SPI_isBusy(base));
	while (!EUSCI_A_SPI_getInterruptStatus(base,
			EUSCI_A_SPI_TRANSMIT_INTERRUPT));
	EUSCI_A_SPI_transmitData(base, data);
	while (!EUSCI_A_SPI_getInterruptStatus(base,
			EUSCI_A_SPI_TRANSMIT_INTERRUPT));
	while (EUSCI_A_SPI_isBusy(base));
}

void tlc_set_gs() {
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT));

    // We need a 0 to show it's GS:
    usci_a_send(EUSCI_A0_BASE, 0x00);
    // Now the GS data itself. There are 16 channels;
    for (uint8_t channel=0; channel<16; channel++) {
        // ...with 16 bits each:
        usci_a_send(EUSCI_A0_BASE, 0x00); // MSByte
        usci_a_send(EUSCI_A0_BASE, (channel % 3) ? 0xff : 0x00); // LSByte
    }

    // LATCH:
    GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4);
}

void tlc_set_fun() {
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
    usci_a_send(EUSCI_A0_BASE, 0x7F);

    for (uint8_t i=0; i<14; i++) {
        // 16 DC 7-tets in 14 octets/bytes:
        usci_a_send(EUSCI_A0_BASE, 0xFF);
    }

    // LATCH:
    GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4);
}

void tlc_set_gs_bb() {
     // We need a 0 to show it's GS:
     tlc_bit(0);
     tlc_bit(0);
     tlc_bit(0);
     tlc_bit(0);
     tlc_bit(0);
     tlc_bit(0);
     tlc_bit(0);
     tlc_bit(0);
     // Now the GS data itself. There are 16 channels;
     for (uint8_t channel=0; channel<16; channel++) {
          // ...with 16 bits each:
          for (uint8_t bit=16; bit; bit--) {
               tlc_bit(bit<9);
          }
     }
     // That means the shift register will be full. Time to latch.
     // !SCLK:
     P1OUT &= ~BIT5;
     // LAT
     GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4);
}

void tlc_set_fun_bb() {
    tlc_bit(1); // 1 to show it's function.
    for (uint8_t b = 255; b>136; b--) { // For all the reserved bits, write a 0.
        tlc_bit(0);
    }

    tlc_bit(0);    // B136 / PSM(D2)
    tlc_bit(0);    // B135 / PSM(D1)
    tlc_bit(0);    // B134 / PSM(D0)
    tlc_bit(0);    // B133 / OLDENA
    tlc_bit(0);    // B132 / IDMCUR(D1)
    tlc_bit(0);    // B131 / IDMCUR(D0)
    tlc_bit(0);    // B130 / IDMRPT(D0)
    tlc_bit(0);    // B129 / IDMENA
    tlc_bit(1);    // B128 / LATTMG(D1)
    tlc_bit(1);    // B127 / LATTMG(D0)
    tlc_bit(0);    // B126 / LSDVLT(D1)
    tlc_bit(0);    // B125 / LSDVLT(D0)
    tlc_bit(0);    // B124 / LODVLT(D1)
    tlc_bit(0);    // B123 / LODVLT(D0)
    tlc_bit(1);    // B122 / ESPWM
    tlc_bit(0);    // B121 / TMGRST
    tlc_bit(1);    // B120 / DSPRPT

    tlc_bit(0);    // B119 / BLANK

    // Global brightness: 7 bits:
    for (uint8_t b=0; b<7; b++) {
        tlc_bit(1);
    }

    // 16 channels of dot-correct:
    for (uint8_t c=0; c<16; c++) {
        // 7 bits each:
        for (uint8_t b=0; b<7; b++) {
            tlc_bit(1);
        }
    }

    // That means the shift register will be full. Time to latch.
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5); // Set CLK low.
    GPIO_pulse(GPIO_PORT_P1, GPIO_PIN4);
}


void init() {
    Grace_init(); // Activate Grace-generated configuration

    /*
     * Peripherals:
     *
     *  Radio (RFM69CW)
     *        (MSB first, clock inactive low,
     *         write on rise, change on fall, MSB first)
     *        eUSCI_B0 - radio
     *        somi, miso, clk, ste
     *        DIO0      P3.1
     *        RESET     P3.2
     */
    init_radio();

    /*
     *   OLED (OLED_0.96)
     *        (write on rise, change on fall,
     *         CS active low, MSB first)
     *        eUSCI_A1
     *        ste, miso, clk
     *        DC        P2.6
     *        RES       P2.7
     */
    qc12_oledInit();
    GrContextInit(&g_sContext, &g_sqc12_oled);
    GrContextBackgroundSet(&g_sContext, ClrBlack);
    GrContextForegroundSet(&g_sContext, ClrWhite);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    GrClearDisplay(&g_sContext);
    GrFlush(&g_sContext);

    // TA1.1 (P1.2)
//    Timer_A_outputPWMParam ta_param = {
//    		TIMER_A_CLOCKSOURCE_SMCLK,
//			TIMER_A_CLOCKSOURCE_DIVIDER_1,
//			TIMER_A_CAPTURECOMPARE_REGISTER_1,
//			TIMER_A_OUTPUTMODE_OUTBITVALUE,
//
//    };
//
//    Timer_A_outputPWM(TIMER_A1_BASE)
}

int main(void)
{
    // TODO: Remove
    volatile uint8_t in = 0;
    char str[16] = "";

    init();
    // TODO:
    //    post();

    tlc_set_gs();
    tlc_set_fun();

    // TESTING THE FLASH:

    // Flash:
    // CS#          P1.1
    // HOLD#   P1.0
    // WP#          P3.0

    // CS# high normally
    // HOLD# high normally
    // WP# high normally (write protect)

    // Read status register:

    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);

    P1OUT &= ~BIT1; // CS low, select.
    usci_a_send(EUSCI_A0_BASE, 0x05); // Read status.
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
    EUSCI_A_SPI_receiveData(EUSCI_A0_BASE); // Throw away the stale garbage we got while sending.

    usci_a_send(EUSCI_A0_BASE, 0xFF); // Meaningless crap.
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
    in = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);

    P1OUT |= BIT1; // CS high, deselect.

    sprintf(str, "status = %d", in);
    GrStringDraw(&g_sContext, str, -1, 0, 0, 1);
    GrFlush(&g_sContext);

    // WRITE ENABLE:
    P1OUT &= ~BIT1; // CS low, select.
    usci_a_send(EUSCI_A0_BASE, 0x06); // WREN
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
    EUSCI_A_SPI_receiveData(EUSCI_A0_BASE); // Throw away the stale garbage we got while sending.
    P1OUT |= BIT1; // CS high, deselect.

    // Read status register:

    P1OUT &= ~BIT1; // CS low, select.
    usci_a_send(EUSCI_A0_BASE, 0x05); // Read status.
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
    EUSCI_A_SPI_receiveData(EUSCI_A0_BASE); // Throw away the stale garbage we got while sending.

    usci_a_send(EUSCI_A0_BASE, 0xFF); // Meaningless crap.
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
    in = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);

    P1OUT |= BIT1; // CS high, deselect.

    sprintf(str, "status = %d", in);
    GrStringDraw(&g_sContext, str, -1, 0, 10, 1);
    GrFlush(&g_sContext);


    while (1);
}

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
#include "leds.h"

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
 *   Flash chip (S25FL216K)
 *        (write on rise, change on fall,
 *         CS active low, clock inactive low, MSB first)
 *        eUSCI_A0 (shared)
 *        somi, miso, clk
 *        CS        P1.1
 *        WP        P3.0
 *        HOLD      P1.0
 *        TODO: DMA: https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/235583
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

void usci_a_send(uint16_t base, uint8_t data) {
	while (EUSCI_A_SPI_isBusy(base));
	while (!EUSCI_A_SPI_getInterruptStatus(base,
			EUSCI_A_SPI_TRANSMIT_INTERRUPT));
	EUSCI_A_SPI_transmitData(base, data);
	while (!EUSCI_A_SPI_getInterruptStatus(base,
			EUSCI_A_SPI_TRANSMIT_INTERRUPT));
	while (EUSCI_A_SPI_isBusy(base));
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

    init_leds();

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
    GrContextFontSet(&g_sContext, &g_sFontCmsc12); // &g_sFontFixed6x8);
    GrClearDisplay(&g_sContext);
    GrImageDraw(&g_sContext, &standing_1, 0, 32);
    GrStringDraw(&g_sContext, "DUPLiCO", -1, 0, 0, 1);
//    GrCircleDraw(&g_sContext, 2, 3, 6);
    GrStringDraw(&g_sContext, "  the gay", -1, 0, 9, 0);
    GrStringDraw(&g_sContext, "<  Play! >", -1, 0, 116, 1);
    GrFlush(&g_sContext);
}

int main(void)
{
    // TODO: Remove
    volatile uint8_t in = 0;
    char str[16] = "";

    init();
    // TODO:
    //    post();

    uint8_t shift = 0;
    while (1) {
    	tlc_set_fun(1);
    	tlc_set_gs(shift);
    	tlc_set_fun(0);
    	shift = (shift + 3) % 15;
    	__delay_cycles(2000000);
    }
}

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
#include "oled.h"

volatile uint8_t f_time_loop = 0;
volatile uint8_t f_new_second = 0;

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

void usci_a_send(uint16_t base, uint8_t data) {
	while (EUSCI_A_SPI_isBusy(base));
	while (!EUSCI_A_SPI_getInterruptStatus(base,
			EUSCI_A_SPI_TRANSMIT_INTERRUPT));
	EUSCI_A_SPI_transmitData(base, data);
	while (!EUSCI_A_SPI_getInterruptStatus(base,
			EUSCI_A_SPI_TRANSMIT_INTERRUPT));
	while (EUSCI_A_SPI_isBusy(base));
}

void init_rtc() {
    RTC_B_definePrescaleEvent(RTC_B_BASE, RTC_B_PRESCALE_1, RTC_B_PSEVENTDIVIDER_4); // 32 Hz
	RTC_B_clearInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
	RTC_B_enableInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
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
    init_oled();
    init_rtc();
}

#define ANIM_START 1
#define ANIM_DONE  0

uint8_t anim_state = ANIM_DONE;
uint8_t anim_index = 0;
uint8_t anim_loops = 0;
qc12_anim_t anim_data;

void anim_next_frame() {

    if (anim_state == ANIM_DONE)
        return;

    GrImageDraw(&g_sContext, anim_data.images[anim_index], 0, 51);
    GrFlush(&g_sContext);

    anim_index++;

    // If we need to loop, loop:
    if (anim_loops && anim_data.looped) {
        if (anim_index == anim_data.loop_end) {
            anim_index = anim_data.loop_start;
            anim_loops--;
        }
    } else if (anim_loops && anim_index == anim_data.len) {
        anim_index = 0;
        anim_loops--;
    }

    if (anim_index == anim_data.len)
        anim_state = ANIM_DONE;
}

void play_animation(qc12_anim_t anim, uint8_t loops) {
    anim_index = 0;
    anim_loops = loops;
    anim_data = anim;
    anim_state = ANIM_START;
}

// We need SMCLK in order to light our LEDs, so LPM1 is the
// sleepiest we can get.
#define SLEEP_BITS LPM1_bits

int main(void)
{
    // TODO: Remove
    volatile uint8_t in = 0;
    char str[16] = "";

    init();
    // TODO:
    //    post();

    oled_draw_pane();

    uint8_t shift = 0;

    play_animation(waving, 5);

    uint8_t rainbow_interval = 3;

    while (1) {
    	if (f_new_second) {
            anim_next_frame();
            f_new_second = 0;
    	}
    	if (f_time_loop) {
    	    if (!--rainbow_interval) {
    	        rainbow_interval = 3;

    	        tlc_set_fun(1);
    	        tlc_set_gs(shift);
    	        tlc_set_fun(0);
    	        shift = (shift + 3) % 15;
    	    }
    	    f_time_loop = 0;
    	}

    	__bis_SR_register(SLEEP_BITS);
    }
}

#pragma vector=RTC_VECTOR
__interrupt
void RTC_A_ISR(void) {
    switch (__even_in_range(RTCIV, 16)) {
    case 0: break;  //No interrupts
    case 2:         //RTCRDYIFG
        f_new_second = 1;
        __bic_SR_register_on_exit(SLEEP_BITS);
        break;
    case 4:         //RTCEVIFG
        //Interrupts every minute // We don't use this.
        __bic_SR_register_on_exit(SLEEP_BITS);
        break;
    case 6:         //RTCAIFG
                    // Alarm!
        break;
    case 8: break;  //RT0PSIFG
    case 10:		// Rollover of prescale counter
        f_time_loop = 1; // We know what it does! It's a TIME LOOP MACHINE.
        // ...who would build a device that loops time every 32 milliseconds?
        // WHO KNOWS. But that's what it does.
        __bic_SR_register_on_exit(SLEEP_BITS);
        break; //RT1PSIFG
    case 12: break; //Reserved
    case 14: break; //Reserved
    case 16: break; //Reserved
    default: break;
    }
}

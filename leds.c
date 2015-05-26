/*
 * leds.c
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

#include "qc12.h"
#include "leds.h"
#include <string.h>

#define DISPLAY_OFF 	0
#define DISPLAY_ON  	1

volatile uint8_t f_time_loop = 0;

#define DISP_MODE_ANIM  -1
#define DISP_MODE_SCROLL 0
#define DISP_MODE_TEXT   1

void led_init() {

	// Setup LED module pins //////////////////////////////////////////////////
	// LED_PORT.LED_DATA, LED_CLOCK, LED_LATCH, LED_BLANK
	P1DIR |= LED_DATA + LED_CLOCK + LED_LATCH + LED_BLANK;

	// Shift register input from LED controllers:
	// LEDPORT.PIN6 as input:
	P1DIR &= ~BIT6;
}

void led_enable(uint16_t duty_cycle) {
//	GPIO_setAsPeripheralModuleFunctionOutputPin(LED_PORT, LED_BLANK);
//	P1SEL |= LED_BLANK; // TODO

//	TIMER_A_generatePWM(
//		TIMER_A0_BASE,
//		TIMER_A_CLOCKSOURCE_ACLK,
//		TIMER_A_CLOCKSOURCE_DIVIDER_1,
//		LED_PERIOD, // period
//		TIMER_A_CAPTURECOMPARE_REGISTER_2,
//		TIMER_A_OUTPUTMODE_RESET_SET,
//		LED_PERIOD - duty_cycle // duty cycle
//	);

	TA0CTL &= ~( TIMER_A_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
	           TIMER_A_UPDOWN_MODE + TIMER_A_DO_CLEAR +
	           TIMER_A_TAIE_INTERRUPT_ENABLE
	           );

	TA0CTL |= ( TIMER_A_CLOCKSOURCE_ACLK +
				TIMER_A_UP_MODE +
				TIMER_A_DO_CLEAR
			  );

	TA0CCR0 = LED_PERIOD;

	TA0CCTL0 &= ~(TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE +
	          	  TIMER_A_OUTPUTMODE_RESET_SET
	          	 );

	TA0CCTL2 |= TIMER_A_OUTPUTMODE_RESET_SET;
	TA0CCR2 = LED_PERIOD - duty_cycle;
	TA1CTL |= TIMER_A_UP_MODE;

}

void led_disable( void )
{
//	GPIO_setAsOutputPin(
//			LED_PORT,
//			LED_BLANK
//	); // TODO
}

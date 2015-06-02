/*
 * clocks.c
 *
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

#include "qc12.h"
#include "clocks.h"
#include <string.h>

volatile Calendar currentTime;


#pragma vector=RTC_VECTOR
__interrupt
void RTC_A_ISR(void)
{
	switch (__even_in_range(RTCIV, 16)) {
	case 0: break;  //No interrupts
	case 2:
//		f_new_second = 1; TODO
//		__bic_SR_register_on_exit(LPM3_bits);
		break;
	case 4:         //RTCEVIFG
		//Interrupts every minute // We don't use this.
		__bic_SR_register_on_exit(LPM3_bits);
		break;
	case 6:         //RTCAIFG
//		f_alarm = next_event_flag;
		break;
	case 8: break;  //RT0PSIFG
	case 10:		// Rollover of prescale counter
//		f_time_loop = 1; // We know what it does! It's a TIME LOOP MACHINE.
		// ...who would build a device that loops time every 32 milliseconds?
		// WHO KNOWS. But that's what it does.
//		__bic_SR_register_on_exit(LPM3_bits); TODO
		break; //RT1PSIFG
	case 12: break; //Reserved
	case 14: break; //Reserved
	case 16: break; //Reserved
	default: break;
	}
}

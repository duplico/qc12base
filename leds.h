/*
 * leds.h
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

#ifndef LEDS_H_
#define LEDS_H_

void init_leds();

void tlc_set_gs(uint8_t shift);
void tlc_set_fun(uint8_t blank);

//void led_timestep();

void led_enable(uint16_t);
void led_disable( void );
//uint8_t led_post();

#endif /* LEDS_H_ */

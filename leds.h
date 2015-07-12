/*
 * leds.h
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

#ifndef LEDS_H_
#define LEDS_H_

void init_tlc();

void tlc_set_gs(uint8_t);
void tlc_set_fun();
void tlc_stage_blank(uint8_t);

void tlc_timestep();

typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} rgbcolor_t;

#endif /* LEDS_H_ */

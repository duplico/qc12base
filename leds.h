/*
 * leds.h
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

#ifndef LEDS_H_
#define LEDS_H_

typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} rgbcolor_t;

typedef struct {
    int_fast32_t red;
    int_fast32_t green;
    int_fast32_t blue;
} rgbdelta_t;

typedef struct {
    rgbcolor_t * colors;
    uint8_t len;
} tlc_animation_t;

extern rgbcolor_t rainbow2[];

extern tlc_animation_t flag_rainbow;
extern tlc_animation_t flag_bi;
extern tlc_animation_t flag_pan;
extern tlc_animation_t flag_trans;
extern tlc_animation_t flag_ace;
extern tlc_animation_t flag_ally;
extern tlc_animation_t flag_leather;
extern tlc_animation_t flag_bear;
extern tlc_animation_t flag_blue;
extern tlc_animation_t flag_lblue;
extern tlc_animation_t flag_green;
extern tlc_animation_t flag_red;
extern tlc_animation_t flag_yellow;
extern tlc_animation_t flag_pink;

void init_tlc();

uint8_t tlc_test_loopback(uint8_t);

void tlc_set_gs();
void tlc_set_fun();
void tlc_stage_blank(uint8_t);
void tlc_start_anim(tlc_animation_t *, uint8_t, uint8_t, uint8_t, uint8_t);
void tlc_stop_anim(uint8_t blank);

void tlc_timestep();

#endif /* LEDS_H_ */

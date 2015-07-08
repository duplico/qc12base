/*
 * oled.h
 *
 *  Created on: Jul 6, 2015
 *      Author: George
 */

#ifndef OLED_H_
#define OLED_H_

#include <qc12.h>
#include <grlib.h>
#include <img.h>

#define OLED_ANIM_START 1
#define OLED_ANIM_DONE  0

tContext g_sContext;

void init_oled();
void oled_draw_pane();
void oled_anim_next_frame();
void oled_play_animation(qc12_anim_t, uint8_t);

#endif /* OLED_H_ */

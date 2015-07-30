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

extern uint8_t oled_anim_state;

tContext g_sContext;

void init_oled();
void oled_draw_pane_and_flush(uint8_t);
void oled_anim_next_frame();
void oled_anim_disp_frame(const qc12_anim_t *animation_data, uint8_t frame_no);
void oled_play_animation(const qc12_anim_t*, uint8_t);
void oled_set_overhead_image(tImage *image, uint8_t len);
void oled_set_overhead_text(char *text, uint8_t len);

void oled_timestep();

#endif /* OLED_H_ */

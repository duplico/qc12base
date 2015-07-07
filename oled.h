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

tContext g_sContext;

void init_oled();
void oled_draw_pane();


#endif /* OLED_H_ */

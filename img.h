/*
 * img.h
 *
 *  Created on: Jun 24, 2015
 *      Author: glouthan
 */

#ifndef IMG_H_
#define IMG_H_

#include <grlib.h>

typedef struct {
    uint8_t looped;
    uint8_t loop_start;
    uint8_t loop_end;
    uint8_t len;
    tImage* images[8];
} qc12_anim_t;

extern uint8_t persistent_sprite_bank_pixels[][];
//extern uint8_t flash_sprite_bank_pixels[][];

extern tImage persistent_sprite_bank[];
extern tImage flash_sprite_bank[];

extern qc12_anim_t standing;
extern qc12_anim_t walking;
extern qc12_anim_t waving;


#endif /* IMG_H_ */

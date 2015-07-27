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
    const tImage* images[10];
} qc12_anim_t;

extern const tImage  fingerprint_1BPP_UNCOMP;
extern const tImage flag1;

extern const uint8_t persistent_sprite_bank_pixels[][];
extern const uint8_t flash_sprite_bank_pixels[][];

extern const tImage persistent_sprite_bank[];
extern const tImage flash_sprite_bank[];

extern const qc12_anim_t standing;
extern const qc12_anim_t walking;
extern const qc12_anim_t wave_right;

extern const qc12_anim_t *demo_anims[];
extern const uint8_t demo_anim_count;

#endif /* IMG_H_ */

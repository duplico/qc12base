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
    uint8_t speed;
    const uint8_t* heads_indices;
    const uint8_t* bodies_indices;
    const uint8_t* legs_indices;
    const int8_t* head_tops;
    const int8_t* body_tops;
    const int8_t* legs_tops;
    const uint8_t* movement;
} qc12_anim_t;

extern const uint8_t legs_clip_offset;

extern const tImage  fingerprint_1BPP_UNCOMP;
extern const tImage flag1;

extern const tImage cloud, heart, empty_heart, idea, lightning, check, nocheck, jakethedog, puppy, bed;

extern const uint8_t persistent_sprite_bank_pixels[][];
extern const uint8_t flash_sprite_bank_pixels[][];

extern const tImage heads[];
extern const tImage bodies[];
extern const tImage legs[];

extern const qc12_anim_t standing, bored_standing;
extern const qc12_anim_t walking, zombie;
extern const qc12_anim_t walking_left, zombie_left;
extern const qc12_anim_t wave_right;
extern const qc12_anim_t flap_arms;
extern const qc12_anim_t infant_standing, infant_moving, infant_walk, infant_walk_left, infant_play, infant_grow;

extern const qc12_anim_t *demo_anims[];
extern const uint8_t demo_anim_count;
extern const uint8_t play_anim_count;

extern const qc12_anim_t *infant_idle_anims[];
extern const qc12_anim_t *idle_anims[];
extern const qc12_anim_t *moody_idle_anims[];
extern const uint8_t idle_anim_count, moody_idle_anim_count;
extern const uint8_t infant_idle_anim_count;


extern const qc12_anim_t *play_cause[];

extern const qc12_anim_t *play_effect[];

extern const qc12_anim_t *play_observe[];

extern const uint8_t my_sprite_id;

#endif /* IMG_H_ */

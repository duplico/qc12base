/*
 * oled.c
 *
 *  Created on: Jul 6, 2015
 *      Author: George
 */

/*
 *   OLED (OLED_0.96)
 *        (write on rise, change on fall,
 *         CS active low, MSB first)
 *        eUSCI_A1
 *        ste, miso, clk
 *        DC        P2.6
 *        RES       P2.7
 */

#include <oled.h>
#include <grlib.h>
#include <qc12_oled.h>
#include <qc12.h>
#include <string.h>
#include <stdlib.h>

uint8_t oled_anim_state = OLED_ANIM_DONE;
uint8_t anim_index = 0;
uint8_t anim_loops = 0;
uint8_t anim_frame_skip = 0;
const qc12_anim_t *anim_data;

uint8_t oled_overhead_type = OLED_OVERHEAD_OFF;
uint8_t oled_overhead_loops = 0;
uint8_t oled_overhead_half_width = 0;
const tImage *oled_overhead_image;
char oled_overhead_text[NAME_MAX_LEN+1] = "";

int8_t char_pos_x = 0;
int8_t char_pos_y = 0;

void init_oled() {
    qc12_oledInit();
    GrContextInit(&g_sContext, &g_sqc12_oled);
    GrContextBackgroundSet(&g_sContext, ClrBlack);
    GrContextForegroundSet(&g_sContext, ClrWhite);
    GrContextFontSet(&g_sContext, &SYS_FONT);
    GrClearDisplay(&g_sContext);
    GrFlush(&g_sContext);
}

void oled_draw_pane_and_flush(uint8_t softkey_sel) {
    static tRectangle erase_rect_top = {0, 0, 64, 2*NAME_FONT_HEIGHT+1};
    static tRectangle erase_rect_btm = {0, SPRITE_Y + 64, 64, 127};

    GrContextForegroundSet(&g_sContext, ClrBlack);
    GrRectFill(&g_sContext, &erase_rect_btm);
    GrRectFill(&g_sContext, &erase_rect_top);
    GrContextForegroundSet(&g_sContext, ClrWhite);

    GrContextFontSet(&g_sContext, &NAME_FONT);
    GrStringDraw(&g_sContext, my_conf.handle, -1, 0, 0, 0);
    uint8_t title_width = GrStringWidthGet(&g_sContext, titles[my_conf.title_index], -1);
    uint8_t the_width = GrStringWidthGet(&g_sContext, "the", -1);
    GrStringDraw(&g_sContext, "the", -1, 65 - title_width - the_width - 3, NAME_FONT_HEIGHT, 0);
    GrStringDraw(&g_sContext, titles[my_conf.title_index], -1, 65 - title_width, NAME_FONT_HEIGHT, 0);
    GrLineDrawH(&g_sContext, 0, 64, 2*NAME_FONT_HEIGHT+1);
    GrContextFontSet(&g_sContext, &SOFTKEY_LABEL_FONT);
    if (!idle_mode_softkey_dis)
        GrStringDrawCentered(&g_sContext, sk_labels[softkey_sel], -1, 32,  SPRITE_Y + 64 + SOFTKEY_FONT_HEIGHT/2, 0);
    GrLineDrawH(&g_sContext, 0, 64, SPRITE_Y + 64);
    GrFlush(&g_sContext);
}

void draw_overhead() {
    if (oled_overhead_type == OLED_OVERHEAD_TXT) {
        GrContextFontSet(&g_sContext, &SYS_FONT);
        int8_t text_x = char_pos_x + 32;
        if (text_x + oled_overhead_half_width > 63) {
            text_x = 63 - oled_overhead_half_width;
        } else if (text_x - oled_overhead_half_width < 0) {
            text_x = oled_overhead_half_width;
        }
        GrStringDrawCentered(&g_sContext, oled_overhead_text, -1, text_x, SPRITE_Y-SYS_FONT_HEIGHT/2, 0);
    } else if (oled_overhead_type == OLED_OVERHEAD_IMG) {
        GrImageDraw(&g_sContext, oled_overhead_image, char_pos_x + (32 - oled_overhead_image->XSize/2), SPRITE_Y - oled_overhead_image->YSize - char_pos_y);
    }
}

void oled_set_overhead_image(const tImage *image, uint8_t loop_len) {
    oled_overhead_loops = loop_len;
    oled_overhead_type = OLED_OVERHEAD_IMG;
    oled_overhead_image = image;
    if (oled_anim_state == OLED_ANIM_DONE) {
        s_oled_needs_redrawn_idle = 1;
    }
}

void oled_set_overhead_text(char *text, uint8_t loop_len) {
    oled_overhead_loops = loop_len;
    oled_overhead_type = OLED_OVERHEAD_TXT;
    oled_overhead_half_width = GrStringWidthGet(&g_sContext, text, -1)/2;
    if (strlen(text) > NAME_MAX_LEN) {
        return; // PROBLEM
    }
    strcpy(oled_overhead_text, text);
    if (oled_anim_state == OLED_ANIM_DONE) {
        s_oled_needs_redrawn_idle = 1;
    }
}

void oled_set_overhead_off() {
    s_overhead_done = 1;
    oled_overhead_loops = 0;
    oled_overhead_type = OLED_OVERHEAD_OFF;
    if (oled_anim_state == OLED_ANIM_DONE) {
        s_oled_needs_redrawn_idle = 1;
    }
}

void do_move(uint8_t move_signal) {
    if (!move_signal)
        return;
    uint8_t move_flag = move_signal >> 6;
    uint8_t move_amt = move_signal & 0b00111111;
    switch (move_flag) {
    case 0: // up
        char_pos_y += move_amt;
        break;
    case 1: // down
        char_pos_y -= move_amt;
        break;
    case 2: // left
        char_pos_x -= move_amt;
        if (char_pos_x < -64) {
            char_pos_x+=128;
        }
        break;
    case 3: // right
        char_pos_x += move_amt;
        if (char_pos_x > 64) {
            char_pos_x-=128;
        }
        break;
    default:
        __never_executed();
    }
}

void oled_anim_disp_frame(const qc12_anim_t *animation_data, uint8_t frame_no) {
    GrClearDisplay(&g_sContext);
    GrImageDraw(&g_sContext, &legs[animation_data->legs_indices[frame_no]], char_pos_x, legs_clip_offset + animation_data->legs_tops[frame_no] + SPRITE_Y - char_pos_y);
    GrImageDraw(&g_sContext, &bodies[animation_data->bodies_indices[frame_no]], char_pos_x, animation_data->body_tops[frame_no] + SPRITE_Y - char_pos_y);
    GrImageDraw(&g_sContext, &heads[animation_data->heads_indices[frame_no]], char_pos_x, animation_data->head_tops[frame_no] + SPRITE_Y - char_pos_y);
    draw_overhead();
    oled_draw_pane_and_flush(idle_mode_softkey_sel); // This flushes.
}

void oled_consider_walking_back() {
    if (char_pos_y != 0) {
        char_pos_x = 64;
        char_pos_y = 0;
    }

    // Determine whether we need to walk back onto the screen after anim.
    if (my_conf.mood < MOOD_THRESH_SAD) {
        // zombie walk back
        if (char_pos_x < -12) {
            oled_play_animation(&zombie, (uint8_t) (-char_pos_x/12) - 1);
        } else if (char_pos_x > 12) {
            oled_play_animation(&zombie_left, (uint8_t) (char_pos_x/12) - 1);
        }
    } else {
        // happy walk back
        if (char_pos_x < -16) {
            oled_play_animation(&walking, (uint8_t) (-char_pos_x/16) - 1);
        } else if (char_pos_x > 16) {
            oled_play_animation(&walking_left, (uint8_t) (char_pos_x/16) - 1);
        }
    }
}

void oled_anim_next_frame() {
    if (oled_anim_state == OLED_ANIM_DONE)
        return;

    do_move(anim_data->movement[anim_index]);
    oled_anim_disp_frame(anim_data, anim_index);

    anim_index++;

    // If we need to loop, loop:
    if (anim_loops && anim_data->looped) {
        if (anim_index == anim_data->loop_end) {
            anim_index = anim_data->loop_start;
            anim_loops--;
        }
    } else if (anim_loops && anim_index == anim_data->len) {
        anim_index = 0;
        anim_loops--;
    }

    if (anim_index == anim_data->len) {
        s_oled_needs_redrawn_idle = 1;
        oled_anim_state = OLED_ANIM_DONE;
        oled_consider_walking_back();
    }
}

void oled_play_animation(const qc12_anim_t *anim, uint8_t loops) {
    anim_index = 0;
    anim_loops = loops;
    anim_data = anim;
    anim_frame_skip = anim->speed;
    oled_anim_state = OLED_ANIM_START;
    s_oled_needs_redrawn_idle = 0;
}

void oled_print(uint8_t x, uint8_t y, const char str[], uint8_t opaque, uint8_t centered) {
    GrContextFontSet(&g_sContext, &SYS_FONT);
    uint8_t curr_y = y;
    uint8_t str_start = 0;
    uint8_t str_end = 0;
    uint8_t str_end_candidate = 0;
    uint8_t str_len = strlen(str);
    // Note: This could spin forever if we use long words.
    while (str_end < str_len) {
        // find the next space, call it str_end_candidate
        while (str_end_candidate < str_len && str[str_end_candidate] != ' ') {
            str_end_candidate++;
        }
        // If the string, from start to end, fits, we continue (find next space).
        // str_end becomes str_end_candidate.
        if (x + GrStringWidthGet(&g_sContext, &str[str_start], str_end_candidate - str_start) < 63) {
            str_end = str_end_candidate;
            str_end_candidate++; // Skip the space.
        } else {
            // If it doesn't fit, we use str_end. If str_end == str_start, then I don't know??? TODO.
            if (centered) {
                GrStringDrawCentered(&g_sContext, &str[str_start], str_end - str_start, (64+x)/2, curr_y+SYS_FONT_HEIGHT/2, opaque);
            } else {
                GrStringDraw(&g_sContext, &str[str_start], str_end - str_start, x, curr_y, opaque);
            }
            curr_y += SYS_FONT_HEIGHT+1;
            str_start = str_end+1; // Skip the space.
        }
    }

    // If there's still something to draw:
    if (str_start < str_len) {
        if (centered) {
            GrStringDrawCentered(&g_sContext, &str[str_start], -1, (64+x)/2, curr_y+SYS_FONT_HEIGHT/2, opaque);
        }
        else {
            GrStringDraw(&g_sContext, &str[str_start], -1, x, curr_y, opaque);
        }
    }
}

void oled_timestep() {
    if (oled_overhead_loops && oled_overhead_type != OLED_OVERHEAD_OFF) {
        if (oled_overhead_loops != 0xFF) {
            oled_overhead_loops--;
        }
        if (!oled_overhead_loops) {
            oled_overhead_type = OLED_OVERHEAD_OFF;
            s_overhead_done = 1;
            if (!oled_anim_state) {
                s_oled_needs_redrawn_idle = 1;
            }
        }
    }

    if (s_overhead_done) {
        s_overhead_done = 0;
        if (my_conf.mood < MOOD_THRESH_SAD) { // We don't really do anything if we're moody.
            if (rand() % 2) {
                oled_set_overhead_image(&lightning, 10);
            } else {
                oled_set_overhead_image(&cloud, 100);
            }
        } else if (am_puppy) {
            oled_set_overhead_image(&jakethedog, 100);
        } else {
            for (uint8_t i=0; i<FAVORITE_COUNT; i++) {
                if (neighbor_badges[fav_badges_ids[i]]) {
                    // favorite nearby.
                    if ((BADGE_FRIEND_BIT & badges_seen[fav_badges_ids[i]])) {
                        // Favorite friend nearby:
                        oled_set_overhead_image(&heart, 100);
                        break;
                    } else {
                        // Favorite non-friend nearby:
                        oled_set_overhead_image(&empty_heart, 100);
                    }
                }
            }
        }
    }


    if (oled_anim_state) {
        if (anim_frame_skip) {
            anim_frame_skip--;
        } else {
            anim_frame_skip = anim_data->speed;
            oled_anim_next_frame();
        }
    }
}

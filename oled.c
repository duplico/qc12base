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

uint8_t oled_anim_state = OLED_ANIM_DONE;
uint8_t anim_index = 0;
uint8_t anim_loops = 0;
uint8_t anim_frame_skip = 0;
const qc12_anim_t *anim_data;

#define OLED_OVERHEAD_OFF 0
#define OLED_OVERHEAD_TXT 1
#define OLED_OVERHEAD_IMG 2

uint8_t oled_overhead_type = OLED_OVERHEAD_OFF;
uint8_t oled_overhead_loops = 0;
tImage *oled_overhead_image;
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
    GrStringDraw(&g_sContext, "the", -1, 63 - title_width - the_width - 3, NAME_FONT_HEIGHT, 0);
    GrStringDraw(&g_sContext, titles[my_conf.title_index], -1, 63 - title_width, NAME_FONT_HEIGHT, 0);
    GrLineDrawH(&g_sContext, 0, 64, 2*NAME_FONT_HEIGHT+1);
    GrContextFontSet(&g_sContext, &SOFTKEY_LABEL_FONT);
    GrStringDrawCentered(&g_sContext, sk_labels[softkey_sel], -1, 32,  SPRITE_Y + 64 + SOFTKEY_FONT_HEIGHT/2, 0);
    GrLineDrawH(&g_sContext, 0, 64, SPRITE_Y + 64);
    GrFlush(&g_sContext);
}

void draw_overhead() {
    if (oled_overhead_type == OLED_OVERHEAD_TXT) {
        GrContextFontSet(&g_sContext, &SYS_FONT);
        GrStringDrawCentered(&g_sContext, oled_overhead_text, -1, 32, SPRITE_Y-SYS_FONT_HEIGHT/2, 0);
    } else if (oled_overhead_type == OLED_OVERHEAD_IMG) {
        GrImageDraw(&g_sContext, oled_overhead_image, char_pos_x, SPRITE_Y-20);
    }
}

void oled_set_overhead_image(tImage *image, uint8_t loop_len) {
    oled_overhead_loops = loop_len;
    oled_overhead_type = OLED_OVERHEAD_IMG;
    oled_overhead_image = image;
    if (oled_anim_state == OLED_ANIM_DONE) {
        s_oled_anim_finished = 1;
    }
}

void oled_set_overhead_text(char *text, uint8_t loop_len) {
    oled_overhead_loops = loop_len;
    oled_overhead_type = OLED_OVERHEAD_TXT;
    if (strlen(text) > NAME_MAX_LEN) {
        return; // PROBLEM
    }
    strcpy(oled_overhead_text, text);
    if (oled_anim_state == OLED_ANIM_DONE) {
        s_oled_anim_finished = 1;
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
    // Determine whether we need to walk back onto the screen after anim.
    if (char_pos_x < -16) {
        oled_play_animation(&walking, (uint8_t) (-char_pos_x/16) - 1);
    } else if (char_pos_x > 16) {
        oled_play_animation(&walking_left, (uint8_t) (char_pos_x/16) - 1);
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
        s_oled_anim_finished = 1;
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
    s_oled_anim_finished = 0;
}

void oled_timestep() {
    if (oled_overhead_loops && oled_overhead_type != OLED_OVERHEAD_OFF) {
        if (oled_overhead_loops != 0xFF) {
            oled_overhead_loops--;
        }
        if (!oled_overhead_loops) {
            oled_overhead_type = OLED_OVERHEAD_OFF;
            if (!oled_anim_state) {
                s_oled_anim_finished = 1;
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

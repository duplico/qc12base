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

uint8_t anim_state = OLED_ANIM_DONE;
uint8_t anim_index = 0;
uint8_t anim_loops = 0;
qc12_anim_t anim_data;

void init_oled() {
    qc12_oledInit();
	GrContextInit(&g_sContext, &g_sqc12_oled);
	GrContextBackgroundSet(&g_sContext, ClrBlack);
	GrContextForegroundSet(&g_sContext, ClrWhite);
	GrContextFontSet(&g_sContext, &g_sFontCmsc12); // TODO: Demeter: default font?
	GrClearDisplay(&g_sContext);
	GrFlush(&g_sContext);
}

void oled_draw_pane() {
	GrContextFontSet(&g_sContext, &g_sFontCmsc12);
	GrStringDraw(&g_sContext, "DUPLiCO", -1, 0, 0, 1);
	GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
	GrStringDraw(&g_sContext, "  the", -1, 0, 12, 0);
	GrContextFontSet(&g_sContext, &g_sFontCmsc12); // &g_sFontFixed6x8);
	GrStringDraw(&g_sContext, "    n00b", -1, 0, 18, 0);
	GrLineDrawH(&g_sContext, 0, 64, 0);
	GrLineDrawH(&g_sContext, 0, 64, 30);
	GrLineDrawH(&g_sContext, 0, 64, 32);
	GrLineDrawH(&g_sContext, 0, 64, 117);
	GrLineDrawH(&g_sContext, 0, 64, 119);
	GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
	GrStringDraw(&g_sContext, "   Play!  ", -1, 0, 120, 1);
	GrFlush(&g_sContext);
}

void oled_anim_next_frame() {

    if (anim_state == OLED_ANIM_DONE)
        return;

    GrImageDraw(&g_sContext, anim_data.images[anim_index], 0, 51);
    GrFlush(&g_sContext);

    anim_index++;

    // If we need to loop, loop:
    if (anim_loops && anim_data.looped) {
        if (anim_index == anim_data.loop_end) {
            anim_index = anim_data.loop_start;
            anim_loops--;
        }
    } else if (anim_loops && anim_index == anim_data.len) {
        anim_index = 0;
        anim_loops--;
    }

    if (anim_index == anim_data.len)
        anim_state = OLED_ANIM_DONE;
}

void oled_play_animation(qc12_anim_t anim, uint8_t loops) {
    anim_index = 0;
    anim_loops = loops;
    anim_data = anim;
    anim_state = OLED_ANIM_START;
}


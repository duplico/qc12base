/*
 * oled.c
 *
 *  Created on: Jul 6, 2015
 *      Author: George
 */

#include <oled.h>
#include <grlib.h>
#include <qc12_oled.h>

/*
 *   OLED (OLED_0.96)
 *        (write on rise, change on fall,
 *         CS active low, MSB first)
 *        eUSCI_A1
 *        ste, miso, clk
 *        DC        P2.6
 *        RES       P2.7
 */

void init_oled() {
    qc12_oledInit();
	GrContextInit(&g_sContext, &g_sqc12_oled);
	GrContextBackgroundSet(&g_sContext, ClrBlack);
	GrContextForegroundSet(&g_sContext, ClrWhite);
	GrContextFontSet(&g_sContext, &g_sFontCmsc12); // TODO: Demeter: default font?
	GrClearDisplay(&g_sContext);
	GrFlush(&g_sContext);
}

void oled_draw_frame() {
	GrContextFontSet(&g_sContext, &g_sFontCmsc12);
	GrStringDraw(&g_sContext, "DUPLiCO", -1, 0, 0, 1);
	GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
	GrStringDraw(&g_sContext, "  the", -1, 0, 12, 0);
	GrContextFontSet(&g_sContext, &g_sFontCmsc12); // &g_sFontFixed6x8);
	GrStringDraw(&g_sContext, "    n00b", -1, 0, 18, 0);
	GrLineDrawH(&g_sContext, 0, 64, 0);
	GrLineDrawH(&g_sContext, 0, 64, 32);
	GrLineDrawH(&g_sContext, 0, 64, 34);
	GrLineDrawH(&g_sContext, 0, 64, 113);
	GrLineDrawH(&g_sContext, 0, 64, 115);
	GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
	GrStringDraw(&g_sContext, "   Play!  ", -1, 0, 116, 1);
	GrFlush(&g_sContext);
}

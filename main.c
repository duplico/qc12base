/*
 * ======== Standard MSP430 includes ========
 */
#include <msp430fr5949.h>
#include <driverlib.h>
#include <stdint.h>
#include <grlib.h>
#include <Template_Driver.h>

/*
 * ======== Grace related includes ========
 */
#include <ti/mcu/msp430/Grace.h>

/*
 *  ======== main ========
 */


/*
 * Peripherals:
 *
 *  Radio (RFM69CW)
 *        eUSCI_B0 - radio
 *        somi, miso, clk, ste
 *        DIO0      P3.1
 *        RESET     P3.2
 */

/*
 *   LED controller
 *        eUSCI_A0 - LEDs  (shared)
 *        somi, miso, clk, ste
 *        GSCLK     P1.2
 */

/*
 *   Flash chip
 *        eUSCI_A0 (shared)
 *        somi, miso, clk
 *        CS        P1.1
 *        WP        P3.0
 *        HOLD      P1.0
 */

/*
 *   OLED
 *        eUSCI_A1
 *        ste, miso, clk
 *        DC        P2.6
 *        RES       P2.7
 */

/*
 *   Buttons
 *   (active low)
 *   BTN1      P3.6
 *   BTN2      P3.5
 *   BTN3      P3.4
 */
    // CS   1.3 (STE)
    // RES  1.5 (pull low to reset, otherwise high)
    // D/C  1.7
    // D0   2.2 (CLK)
    // D1   1.6 (MOSI)

#define RESPORT	GPIO_PORT_P1
#define RESPIN	GPIO_PIN5

#define DCPORT	GPIO_PORT_P1
#define DCPIN	GPIO_PIN7

static const unsigned char pixel_fingerprint_badge_thinned1BPP_UNCOMP[] =
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x34, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x08, 0x5a, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x05, 0x25, 0x45, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0x40, 0x1a, 0xa2, 0xa5, 0x40, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03, 0x5f, 0xe8, 0x49, 0x55, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x0a, 0xa0, 0x12, 0x24, 0xaa, 0xac, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x49, 0x58, 0xe3, 0x5a, 0x94, 0xd4, 0x80, 0x00, 0x00, 0x00,
0x00, 0x02, 0x48, 0x83, 0x1c, 0xa5, 0x55, 0x5a, 0x90, 0x00, 0x00, 0x00,
0x00, 0x22, 0x94, 0x30, 0x61, 0x42, 0xaa, 0xab, 0x50, 0x00, 0x00, 0x00,
0x00, 0x20, 0x82, 0x4a, 0xc4, 0x51, 0x55, 0x55, 0x29, 0x00, 0x00, 0x00,
0x00, 0x45, 0x48, 0x33, 0x08, 0x21, 0x69, 0x56, 0xa5, 0x80, 0x00, 0x00,
0x00, 0x05, 0x29, 0x59, 0xf1, 0x4a, 0xae, 0xaa, 0xd6, 0x80, 0x00, 0x00,
0x00, 0x05, 0x24, 0xa6, 0x00, 0xc1, 0x55, 0x55, 0x6a, 0x10, 0x00, 0x00,
0x00, 0x0a, 0x19, 0x1b, 0xf8, 0x01, 0x2a, 0xaa, 0xaa, 0x58, 0x00, 0x00,
0x04, 0x1a, 0x08, 0xa6, 0x01, 0x00, 0x95, 0x5a, 0x15, 0x28, 0x00, 0x00,
0x04, 0x51, 0x11, 0x53, 0x80, 0x08, 0x62, 0xad, 0x55, 0x05, 0x00, 0x00,
0x09, 0x22, 0x92, 0xa8, 0x3c, 0xc4, 0x29, 0x54, 0xaa, 0xd4, 0x00, 0x00,
0x0a, 0x20, 0x4a, 0x16, 0x00, 0x92, 0x94, 0x2a, 0xaa, 0xaa, 0xa0, 0x00,
0x02, 0x09, 0x25, 0x29, 0xc3, 0x04, 0xca, 0x55, 0x55, 0x2a, 0x50, 0x00,
0x14, 0x48, 0x05, 0x50, 0x00, 0x00, 0x65, 0x4a, 0xaa, 0x15, 0x40, 0x00,
0x15, 0x54, 0x45, 0xa0, 0x00, 0x00, 0x10, 0x92, 0xa9, 0x54, 0x28, 0x00,
0x10, 0x16, 0x06, 0x90, 0x00, 0x00, 0x29, 0x49, 0x52, 0xaa, 0xa0, 0x00,
0x00, 0xa0, 0x02, 0x90, 0x00, 0x00, 0x00, 0xa2, 0x94, 0xaa, 0x34, 0x00,
0x02, 0xa2, 0x82, 0x40, 0x3c, 0xf0, 0x00, 0x41, 0x69, 0x55, 0x56, 0x00,
0x02, 0x02, 0x01, 0x20, 0x3c, 0xf0, 0x09, 0x26, 0xca, 0xab, 0x2a, 0x00,
0x22, 0x40, 0x02, 0x60, 0x3c, 0xf0, 0x14, 0x09, 0xb6, 0xaa, 0xa9, 0x40,
0x22, 0x40, 0x02, 0x50, 0x3c, 0xf0, 0x28, 0x12, 0x65, 0xd5, 0x0a, 0x80,
0x6a, 0x80, 0x32, 0x50, 0x3c, 0xf0, 0x0c, 0x04, 0xcb, 0x17, 0x55, 0x00,
0x6a, 0x84, 0x34, 0xd0, 0x3c, 0xf0, 0x35, 0x29, 0x32, 0x6a, 0x55, 0x00,
0x6a, 0x84, 0x95, 0x20, 0x3c, 0xf0, 0x2a, 0x25, 0x24, 0x48, 0xa9, 0x00,
0x20, 0x00, 0xa9, 0x20, 0x3c, 0xf0, 0x0a, 0x92, 0x59, 0x51, 0x2a, 0x80,
0x35, 0x09, 0x2b, 0x50, 0x3c, 0xf0, 0x35, 0x81, 0x92, 0x96, 0x02, 0xa0,
0x15, 0x08, 0x52, 0xa0, 0x3c, 0xf0, 0x25, 0x11, 0x34, 0xa4, 0x84, 0x40,
0x15, 0x08, 0x55, 0x40, 0x3c, 0xf0, 0x09, 0x48, 0x2d, 0x29, 0x28, 0x80,
0x15, 0x20, 0xaa, 0x20, 0x3c, 0xf0, 0x08, 0x8a, 0x49, 0x49, 0x01, 0x20,
0x15, 0x28, 0x2a, 0x90, 0x3c, 0xf0, 0x18, 0x88, 0xda, 0x52, 0x55, 0x40,
0x11, 0x24, 0x55, 0x40, 0x00, 0x00, 0x50, 0x08, 0x92, 0x52, 0xd5, 0x40,
0x11, 0x26, 0x44, 0xb0, 0x00, 0x00, 0x10, 0x48, 0x83, 0x86, 0xa8, 0x40,
0x01, 0x63, 0x28, 0x40, 0x00, 0x00, 0x10, 0x48, 0xc0, 0x44, 0xaa, 0x40,
0x09, 0x46, 0x15, 0xb0, 0x00, 0x00, 0x50, 0x68, 0xd1, 0x15, 0xaa, 0xc0,
0x09, 0x44, 0x02, 0xd8, 0x00, 0x60, 0x54, 0x2c, 0x11, 0x15, 0x22, 0x80,
0x00, 0x82, 0xa5, 0x27, 0x30, 0x1d, 0x80, 0x26, 0x40, 0x95, 0x02, 0x80,
0x00, 0x22, 0x04, 0x99, 0xfe, 0x6a, 0x8a, 0x22, 0x49, 0x95, 0x46, 0x80,
0x02, 0x16, 0x21, 0x20, 0x0e, 0x15, 0x42, 0x13, 0x04, 0x95, 0x46, 0x00,
0x02, 0x12, 0x12, 0x9c, 0x39, 0x28, 0x00, 0x29, 0xa4, 0x55, 0x06, 0x00,
0x00, 0x12, 0x09, 0x02, 0x08, 0x06, 0x96, 0x04, 0xa6, 0x52, 0x46, 0x00,
0x00, 0x10, 0x52, 0x7f, 0xe6, 0x42, 0x4b, 0x04, 0x00, 0x92, 0x46, 0x00,
0x00, 0x00, 0x8a, 0xc0, 0x13, 0xa8, 0xa5, 0x0b, 0x48, 0x92, 0x04, 0x00,
0x00, 0x20, 0xb5, 0x0f, 0xc4, 0x94, 0x52, 0x4d, 0x4a, 0x9a, 0x04, 0x00,
0x00, 0x00, 0x88, 0x21, 0x33, 0x6a, 0xaa, 0x95, 0x09, 0x52, 0x10, 0x00,
0x00, 0x00, 0x83, 0x00, 0x09, 0xd5, 0x54, 0x52, 0xa8, 0xa9, 0x00, 0x00,
0x00, 0x00, 0x3f, 0x3e, 0x76, 0x29, 0x59, 0x4b, 0x02, 0xac, 0x00, 0x00,
0x00, 0x00, 0x40, 0x00, 0x89, 0xd2, 0xa4, 0xa5, 0x54, 0x54, 0x00, 0x00,
0x00, 0x00, 0x31, 0xf0, 0xa6, 0x95, 0x50, 0x51, 0x15, 0x40, 0x00, 0x00,
0x00, 0x00, 0x08, 0x74, 0x09, 0x2a, 0x29, 0x6a, 0xaa, 0xa8, 0x00, 0x00,
0x00, 0x00, 0x03, 0x1f, 0x86, 0xd4, 0x82, 0xa0, 0x02, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0xcc, 0xd1, 0x29, 0x00, 0x54, 0x14, 0x40, 0x00, 0x00,
0x00, 0x00, 0x00, 0x02, 0x04, 0x82, 0x82, 0x28, 0xa8, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0xb3, 0x21, 0x6a, 0x14, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x4c, 0x92, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned long palette_fingerprint_badge_thinned1BPP_UNCOMP[]=
{
	0x000000, 	0xffffff
};

const tImage  fingerprint_badge_thinned1BPP_UNCOMP=
{
	IMAGE_FMT_1BPP_UNCOMP,
	93,
	64,
	2,
	palette_fingerprint_badge_thinned1BPP_UNCOMP,
	pixel_fingerprint_badge_thinned1BPP_UNCOMP,
};

tContext g_sContext;

int main(void)
{
    Grace_init(); // Activate Grace-generated configuration
    
    // Unlock pins after low power mode. (See #10420-D)
    PM5CTL0 &= ~LOCKLPM5;

    Template_DriverInit();

    GrContextInit(&g_sContext, &g_sTemplate_Driver);
    GrContextForegroundSet(&g_sContext, ClrBlack);
    GrContextBackgroundSet(&g_sContext, ClrWhite);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    GrClearDisplay(&g_sContext);
    GrFlush(&g_sContext);

    GrImageDraw(&g_sContext, &fingerprint_badge_thinned1BPP_UNCOMP, 17, 0);
    GrFlush(&g_sContext);

    while (1);
}

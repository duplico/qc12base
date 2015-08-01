/*
 * leds.c
 * (c) 2015 George Louthan
 * 3-clause BSD license; see license.md.
 */

/*
 *   LED controller (TLC5948A)
 *        (write on rise, change on fall,
 *         clock inactive low, MSB first)
 *        eUSCI_A0 - LEDs  (shared)
 *        somi, miso, clk
 *        GSCLK     P1.2 (timer TA1.1)
 *        LAT       P1.4
 */

#include "qc12.h"
#include "leds.h"
#include <radio.h>

#define TLC_THISISGS    0x00
#define TLC_THISISFUN   0x01

// Current TLC sending state:
uint8_t tlc_send_type = TLC_SEND_IDLE;

uint8_t tlc_tx_index = 0;   // Index of currently sending buffer

// GS entry transmission handling indices:
volatile uint8_t rgb_element_index = 14;
volatile uint8_t tlc_color_index = 0;
volatile uint8_t tlc_first_frame = 0;

uint8_t tlc_anim_mode = TLC_ANIM_MODE_IDLE;
uint8_t tlc_anim_index = 0;   // Number of channels to shift gs_data by
uint8_t tlc_anim_pad_len = 0;
uint8_t tlc_is_ambient = 0;

uint8_t tlc_loopback_data_out = 0x00;
volatile uint8_t tlc_loopback_data_in = 0x00;

rgbcolor_t *tlc_curr_anim;
uint8_t tlc_curr_anim_len;
uint8_t tlc_anim_looping;

// Flags:

const rgbcolor_t flag_rainbow_colors[] = {
        {0xe400, 0x0300, 0x0300}, // Red
        {0xff00, 0x8c00, 0x0000}, // Orange
        {0xff00, 0xed00, 0x0000}, // Yellow
        {0x0000, 0x8000, 0x2600}, // Green
        {0x0000, 0x4d00, 0xff00}, // Blue
        {0x7500, 0x0700, 0x8700}, // Purple
};

const tlc_animation_t flag_rainbow = {
        &flag_rainbow_colors[0],
        6,
        "Rainbow"
};

const rgbcolor_t flag_bi_colors[] = {
        {0xff00, 0x0000, 0xb000},
        {0,0,0},
//        {0x5300, 0x1f00, 0x6600}, // 115, 79, 150
        {0,0,0},
        {0x5000, 0x0000, 0xffff}, // 0, 56, 168
};

const tlc_animation_t flag_bi = {
        &flag_bi_colors[0],
        4,
        "Bisexual"
};

const rgbcolor_t flag_pan_colors[] = {
        {0xff00, 0x2100, 0x8c00}, // 255,33,140
        {0xff00, 0xd800, 0x0000}, //255,216,0
        {0xff00, 0xd800, 0x0000}, //255,216,0
        {0x2100, 0xb100, 0xff00}, //33,177,255
};

const tlc_animation_t flag_pan = {
        &flag_pan_colors[0],
        4,
        "Pansexual"
};

const rgbcolor_t flag_trans_colors[] = {
        {0x1B00, 0xCE00, 0xFA00}, //91,206,250
        {0x1B00, 0xCE00, 0xFA00}, //91,206,250
//        {0xFF00, 0x8900, 0x9800}, //245,169,184
        {0xff00, 0x0000, 0xb000},
        {0xFF00, 0xFF00, 0xFF00}, //255,255,255
        {0xFF00, 0xFF00, 0xFF00}, //255,255,255
//        {0xFF00, 0xA900, 0xB800}, //245,169,184
        {0xff00, 0x0000, 0xb000},
        {0x1B00, 0xCE00, 0xFA00}, //91,206,250
        {0x1B00, 0xCE00, 0xFA00}, //91,206,250
};

const tlc_animation_t flag_trans = {
        &flag_trans_colors[0],
        8,
        "Trans"
};

const rgbcolor_t flag_ace_colors[] = {
        {0x7000, 0x000, 0xFFFF}, //128,0,128
        {0x7000, 0x000, 0xFFFF}, //128,0,128
        {0x0000, 0x000, 0x0000}, //128,0,128
        {0x0000, 0x000, 0x0000}, //128,0,128
        {0xFF00, 0xFF00, 0xFF00}, //255,255,255
        {0x0300, 0x0300, 0x0300}, //163,163,163,
        {0x0300, 0x0300, 0x0300}, //163,163,163,
        {0x0000, 0x0000, 0x0000}, //163,163,163,
        {0x0300, 0x0300, 0x0300}, //163,163,163,
};

const tlc_animation_t flag_ace = {
        &flag_ace_colors[0],
        9,
        "Asexual"
};

const rgbcolor_t flag_ally_colors[] = {
        {0x000, 0x000, 0x000}, //0,0,0
        {0x5F00, 0x5F00, 0x5F00}, //255,255,255
        {0x000, 0x000, 0x000}, //0,0,0
        {0x000, 0x000, 0x000}, //0,0,0
        {0x5F00, 0x5F00, 0x5F00}, //255,255,255
        {0x000, 0x000, 0x000}, //0,0,0
        {0x000, 0x000, 0x000}, //0,0,0
        {0x5F00, 0x5F00, 0x5F00}, //255,255,255
        {0x000, 0x000, 0x000}, //0,0,0
};

const tlc_animation_t flag_ally = {
        &flag_ally_colors[0],
        9,
        "Ally"
};

//////////// unlockable: //////////

const rgbcolor_t flag_leather_colors[] = {
        {0x0800, 0x0800, 0x6B00}, //24,24,107
        {0x000, 0x000, 0x000}, //0,0,0
        {0x000, 0x000, 0x000}, //0,0,0
        {0x0800, 0x0800, 0x6B00}, //24,24,107
        {0x0800, 0x0800, 0x6B00}, //24,24,107
        {0xFF00, 0xFF00, 0xFF00}, //255,255,255
        {0x0800, 0x0800, 0x6B00}, //24,24,107
        {0x0800, 0x0800, 0x6B00}, //24,24,107
        {0x000, 0x000, 0x000}, //0,0,0
        {0x000, 0x000, 0x000}, //0,0,0
        {0x000, 0x000, 0x000}, //0,0,0
        {0xFF00, 0x000, 0x0000}, //231,0,57
        {0x000, 0x000, 0x000}, //0,0,0
        {0x000, 0x000, 0x000}, //0,0,0
};

const tlc_animation_t flag_leather = {
        &flag_leather_colors[0],
        14,
        "Leather"
};


const rgbcolor_t flag_bear_colors[] = {
        {0xA500, 0x3800, 0x0400},
        {0xD500, 0x6300, 0x0000},
        {0xD500, 0x6300, 0x0000},
        {0xD500, 0x6300, 0x0000},
        {0xFE00, 0xDD00, 0x6300},
        {0xFE00, 0xDD00, 0x6300},
        {0xbE00, 0xa600, 0x3800},
        {0x0000, 0x0000, 0x0000},
};

const tlc_animation_t flag_bear = {
        &flag_bear_colors[0],
        8,
        "Bear"
};

const rgbcolor_t flag_blue_colors[] = {
        //22, 13, 203
        {0x0000, 0x0000, 0xFFFF},
};

const tlc_animation_t flag_blue = {
        &flag_blue_colors[0],
        1,
        "Blue"
};

const rgbcolor_t flag_lblue_colors[] = {
        //153,235,255
        {0x0000, 0xC000, 0xFFFF},
};

const tlc_animation_t flag_lblue = {
        &flag_lblue_colors[0],
        1,
        "Light blue"
};

const rgbcolor_t flag_green_colors[] = {
        //76,187,23
        {0x0F00, 0xFF00, 0x0700},
};

const tlc_animation_t flag_green = {
        &flag_green_colors[0],
        1,
        "Green"
};

const rgbcolor_t flag_red_colors[] = {
        //255,0,0
        {0xFF00, 0x000, 0x000},
};

const tlc_animation_t flag_red = {
        &flag_red_colors[0],
        1,
        "Red"
};

const rgbcolor_t flag_yellow_colors[] = {
        //255,255,0
        {0xE000, 0xA000, 0x0000},
};

const tlc_animation_t flag_yellow = {
        &flag_yellow_colors[0],
        1,
        "Yellow"
};

const rgbcolor_t flag_pink_colors[] = {
        //249,162,241
        {0xd500, 0x0000, 0x6900},
};

const tlc_animation_t flag_pink = {
        &flag_pink_colors[0],
        1,
        "Pink"
};

/// end of flags //////

const tlc_animation_t *flags[FLAG_COUNT] = {
        &flag_rainbow,
        &flag_bi,
        &flag_pan,
        &flag_trans,
        &flag_ace,
        &flag_ally,
        &flag_leather,
        &flag_bear,
        &flag_blue,
        &flag_lblue,
        &flag_green,
        &flag_red,
        &flag_yellow,
        &flag_pink,
};

///////////////////////

rgbcolor_t rainbow1[10] = {
        // rainbow colors
        { 0x0f00, 0x00, 0x00},
        { 0xff00, 0x00, 0x00 },
        { 0x0f00, 0x0f00, 0x00},
        { 0x00, 0xff00, 0x00 },
        { 0x00, 0x0f00, 0x0f00 },
        { 0x00, 0x00, 0xff00 },
        { 0x100, 0x100, 0x2000 },
        { 0xe00, 0xe00, 0xe00 },
        { 0x100, 0x100, 0x100 },
        { 0x00, 0x00, 0x000 },
};

const rgbcolor_t test_colors[3] = {
        // rainbow colors
        { 0xff00, 0x00, 0x00},
        { 0x00, 0xff00, 0x00 },
        { 0x00, 0x00, 0xff00 },
};

uint8_t ring_fade_index = 0;
uint8_t ring_fade_steps = 0;

const rgbcolor_t color_off = {0, 0, 0};
const rgbcolor_t mood_green = { 0x0000, 0x3000, 0x0000};
const rgbcolor_t mood_red   = { 0x3000, 0x0000, 0x0000};
const rgbdelta_t mood_step = {
        (-0x3000) / 75,
        (0x3000) / 75,
        (0x0000)
};

rgbcolor_t tlc_ambient_colors = { 0x0000, 0x0000, 0x0000};

const tlc_animation_t flag_ambient = {
        &tlc_ambient_colors,
        1,
        "Mood"
};

rgbcolor_t tlc_colors_curr[5] = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
};

rgbcolor_t tlc_colors_next[5] = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
};

rgbdelta_t tlc_colors_step[5] = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0},
};

// Buffers containing actual data to send to the TLC:

uint8_t fun_base[] = {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00, // ...reserved...
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        // B135 / PSM(D1)       0
        // B134 / PSM(D0)       0
        // B133 / OLDENA        0
        // B132 / IDMCUR(D1)    0
        // B131 / IDMCUR(D0)    0
        // B130 / IDMRPT(D0)    0
        // B129 / IDMENA        0
        // B128 / LATTMG(D1)    1:
        0x01,
        // B127 / LATTMG(D0)    1
        // B126 / LSDVLT(D1)    0
        // B125 / LSDVLT(D0)    0
        // B124 / LODVLT(D1)    0
        // B123 / LODVLT(D0)    0
        // B122 / ESPWM         1
        // B121 / TMGRST        1
        // B120 / DSPRPT        1:
        0x87,
        // B119 / BLANK
        // and 7 bits of global brightness correction:
        0x08,
        // HERE WE SWITCH TO 7-BIT SPI.
        // The following index is 18:
        0x4F, // rgb0
        0x7F,
        0x7F,
        0x4F, // rgb1
        0x7F,
        0x7F,
        0x4F, // rgb2
        0x7F,
        0x7F,
        0x4F, // rgb3
        0x7F,
        0x7F,
        0x4F, // rgb4
        0x7F,
        0x7F,
        0x7F, // tx
};

inline void tlc_set_gs() {
    if (tlc_send_type != TLC_SEND_IDLE)
        return;
    tlc_send_type = TLC_SEND_TYPE_GS;
    tlc_tx_index = 0;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, TLC_THISISGS);
}

void tlc_set_fun() {
    while (tlc_send_type != TLC_SEND_IDLE)
        __no_operation(); // shouldn't ever actually have to block on this.
    tlc_send_type = TLC_SEND_TYPE_FUN;
    tlc_tx_index = 0;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, TLC_THISISFUN);
}

// Stage the blank bit:
inline void tlc_stage_blank(uint8_t blank) {
    if (blank) {
        fun_base[17] |= BIT7;
        fun_base[16] &= ~BIT1;
    } else {
        fun_base[17] &= ~BIT7;
        fun_base[16] |= BIT1;
    }
}

// Test the TLC chip with a shift-register loopback.
// Returns 0 for success and 1 for failure.
uint8_t tlc_test_loopback(uint8_t test_pattern) {
    // Send the test pattern 34 times, and expect to receive it shifted
    // a bit.
    tlc_loopback_data_out = test_pattern;
    while (tlc_send_type != TLC_SEND_IDLE); // I don't see this happening...

    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);

    tlc_send_type = TLC_SEND_TYPE_LB;
    tlc_tx_index = 0;
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, test_pattern);
    // Spin while we send and receive:
    while (tlc_send_type != TLC_SEND_IDLE);

    EUSCI_A_SPI_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);

    return tlc_loopback_data_in != (uint8_t) ((test_pattern << 7) | (test_pattern >> 1));
}

// Stage global brightness if different from default:
void tlc_stage_bc(uint8_t bc) {
    bc = bc & 0b01111111; // Mask out BLANK just in case.
    fun_base[17] &= 0b10000000;
    fun_base[17] |= bc;
}

void init_tlc() {
    // This is just out of an abundance of caution:
    UCA0CTLW0 |= UCSWRST;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 &= ~UCSWRST;

    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    tlc_set_gs();

    tlc_stage_blank(1);
    tlc_set_fun();
}

void stage_color(rgbcolor_t *dest_color_frame, uint8_t anim_index) {
    if (anim_index < tlc_anim_pad_len || anim_index >= tlc_curr_anim_len - tlc_anim_pad_len) {
        // If the current index is in the pad, i.e. before or after the anim:
        memcpy(dest_color_frame, &color_off, sizeof(rgbcolor_t));
    } else {
        // Current index is in the animation, not the pad:
        memcpy(dest_color_frame, &tlc_curr_anim[anim_index-tlc_anim_pad_len], sizeof(rgbcolor_t));
    }
}

inline void tlc_load_colors() {
    if (tlc_anim_mode == TLC_ANIM_MODE_SAME) {
        // Stage in current color:
        stage_color(&tlc_colors_curr[0], tlc_anim_index);
        // Stage in next color:
        stage_color(&tlc_colors_next[0], (tlc_anim_index+1) % tlc_curr_anim_len);

        tlc_colors_step[0].red = ((int_fast32_t) tlc_colors_next[0].red - tlc_colors_curr[0].red) / ring_fade_steps;
        tlc_colors_step[0].green = ((int_fast32_t) tlc_colors_next[0].green - tlc_colors_curr[0].green) / ring_fade_steps;
        tlc_colors_step[0].blue = ((int_fast32_t) tlc_colors_next[0].blue - tlc_colors_curr[0].blue) / ring_fade_steps;
    } else if (tlc_anim_mode == TLC_ANIM_MODE_SHIFT) {
        for (uint8_t i=0; i<5; i++) {
            // Stage in current color:
            stage_color(&tlc_colors_curr[i], (tlc_anim_index+i) % tlc_curr_anim_len);
            // Stage in next color:
            stage_color(&tlc_colors_next[i], (tlc_anim_index+i+1) % tlc_curr_anim_len);

            tlc_colors_step[i].red = ((int_fast32_t) tlc_colors_next[i].red - tlc_colors_curr[i].red) / ring_fade_steps;
            tlc_colors_step[i].green = ((int_fast32_t) tlc_colors_next[i].green - tlc_colors_curr[i].green) / ring_fade_steps;
            tlc_colors_step[i].blue = ((int_fast32_t) tlc_colors_next[i].blue - tlc_colors_curr[i].blue) / ring_fade_steps;
        }
    }
}

inline void tlc_fade_colors() {
    if (ring_fade_steps && ring_fade_index == ring_fade_steps-1) {
        // hit the destination:
        memcpy(&tlc_colors_curr[0], tlc_colors_next, sizeof(rgbcolor_t));

        if (tlc_anim_mode == TLC_ANIM_MODE_SHIFT) {
            for (uint8_t i=1; i<5; i++) {
                memcpy(&tlc_colors_curr[0], tlc_colors_next, sizeof(rgbcolor_t));
            }
        }
    } else {
        // Load index 0 no matter what.
        tlc_colors_curr[0].red += tlc_colors_step[0].red;
        tlc_colors_curr[0].green += tlc_colors_step[0].green;
        tlc_colors_curr[0].blue += tlc_colors_step[0].blue;

        // If we're shifting (i.e. using the rest of the buffer), then do the rest
        if (tlc_anim_mode == TLC_ANIM_MODE_SHIFT) {
            for (uint8_t i=1; i<5; i++) {
                tlc_colors_curr[i].red += tlc_colors_step[i].red;
                tlc_colors_curr[i].green += tlc_colors_step[i].green;
                tlc_colors_curr[i].blue += tlc_colors_step[i].blue;
            }
        }
    }
}

void tlc_set_ambient(uint8_t mood) {
    if (mood >= 90) {
        // green
        memcpy(&tlc_ambient_colors, &mood_green, sizeof(rgbcolor_t));
    } else if (mood < MOOD_THRESH_SAD ) {
        // red
        memcpy(&tlc_ambient_colors, &mood_red, sizeof(rgbcolor_t));
    } else {
        // go from green
        tlc_ambient_colors.red = mood_green.red - mood_step.red * (100-mood);
        tlc_ambient_colors.green = mood_green.green - mood_step.green * (100-mood);
        tlc_ambient_colors.blue = 0x0000;
    }
}

void tlc_display_ambient() {
    if (tlc_anim_mode != TLC_ANIM_MODE_IDLE)
        return;

    uint8_t speed = 85;
    if (neighbor_count > 7) {
        speed = 12;
    } else {
        speed = speed - speed * neighbor_count / 7;
    }

    tlc_start_anim(&flag_ambient, 0, speed, 0, 0);
    tlc_is_ambient = 1;
}

void tlc_start_anim(const tlc_animation_t *anim, uint8_t anim_len, uint8_t fade_steps, uint8_t all_lights_same, uint8_t loop) {
    f_tlc_anim_done = 0;
    tlc_first_frame = 1;
    tlc_anim_index = 0; // This is our index in the animation.
    tlc_is_ambient = 0;
    s_flag_wave = 0;

    if (all_lights_same) {
        tlc_anim_mode = TLC_ANIM_MODE_SAME;
        tlc_anim_pad_len = 1;
    }
    else {
        tlc_anim_mode = TLC_ANIM_MODE_SHIFT;
        tlc_anim_pad_len = 5;
    }

    if (!anim_len) {
        anim_len = anim->len;
    }

    tlc_curr_anim_len = anim_len + 2*tlc_anim_pad_len;
    tlc_curr_anim = (rgbcolor_t *) anim->colors;

    ring_fade_steps = fade_steps;
    ring_fade_index = 0;
    if (all_lights_same) {
        tlc_anim_mode = TLC_ANIM_MODE_SAME;
    } else {
        tlc_anim_mode = TLC_ANIM_MODE_SHIFT;
    }
    tlc_anim_looping = loop;
    tlc_load_colors();
}

void tlc_stop_anim(uint8_t blank) {
    if (blank) {
        tlc_stage_blank(blank);
        tlc_set_fun();
    }
    tlc_is_ambient = 0;
    tlc_anim_mode = TLC_ANIM_MODE_IDLE;
}

inline void tlc_timestep() {
    if (tlc_anim_mode == TLC_ANIM_MODE_IDLE) {
        return;
    }

    // Now compute the next frame:
    // If we're shifting, we use all of tlc_curr_colors;
    //  if they're all the same, we only care about index 0.

    // Start by incrementing the between-frame index.
    ring_fade_index++;

    if (ring_fade_index >= ring_fade_steps) {
        // If we're done cycling between frames, then we can increment the shift.
        ring_fade_index = 0;
        tlc_anim_index++;

        // If the shift will overflow, we're finished.
        // unless we're looping.
        if (tlc_anim_index == tlc_curr_anim_len - tlc_anim_pad_len) {
            if (tlc_anim_looping) {
                if (tlc_anim_looping != 0xFF)
                    tlc_anim_looping--;
                tlc_anim_index = 0;
            } else {
                tlc_anim_mode = TLC_ANIM_MODE_IDLE;
                f_tlc_anim_done = 1;
                return;
            }
        }
        // load the next color sets.
        tlc_load_colors();
    } else { // Otherwise, we're still fading...
        // Compute the next step fade.
        tlc_fade_colors();
    }
    // Display currently loaded frame:
    tlc_set_gs();

}

#pragma vector=USCI_A0_VECTOR
__interrupt void EUSCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, 4)) {
    //Vector 2 - RXIFG
    case 2:
        // We received some garbage sent to us while we were sending.
        if (tlc_send_type == TLC_SEND_TYPE_LB) {
            // We're only interested in it if we're doing a loopback test.
            tlc_loopback_data_in = EUSCI_B_SPI_receiveData(EUSCI_A0_BASE);
        } else {
            EUSCI_B_SPI_receiveData(EUSCI_A0_BASE); // Throw it away.
        }
        break; // End of RXIFG ///////////////////////////////////////////////////////

    case 4: // Vector 4 - TXIFG : I just sent a byte.
        if (tlc_send_type == TLC_SEND_TYPE_GS) {
            if (tlc_tx_index == 0) { // txl, msb
                EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (rfm_state ? 0x88 : 0x00));
            } else if (tlc_tx_index == 1) { // txl, lsb
                EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (rfm_state ? 0x88 : 0x00));
                rgb_element_index = 0;
                tlc_color_index = 0;
            } else if (tlc_tx_index == 32) { // done
                GPIO_pulse(TLC_LATPORT, TLC_LATPIN); // LATCH.
                tlc_send_type = TLC_SEND_IDLE;
                if (tlc_first_frame) {
                    tlc_first_frame = 0;
                    tlc_stage_blank(0);
                    tlc_set_fun();
                }
                break;
            } else { // gs
                switch(rgb_element_index % 6) { // WATCH OUT! Weird order below:
                case 1:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_colors_curr[tlc_color_index].blue & 0x00ff));
                    break;
                case 0:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_colors_curr[tlc_color_index].blue >> 8));
                    break;
                case 3:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_colors_curr[tlc_color_index].green & 0x00ff));
                    break;
                case 2:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_colors_curr[tlc_color_index].green >> 8));
                    break;
                case 4:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_colors_curr[tlc_color_index].red >> 8));
                    break;
                case 5:
                    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, (uint8_t) (tlc_colors_curr[tlc_color_index].red & 0x00ff));

                    if (tlc_anim_mode == TLC_ANIM_MODE_SHIFT) {
                        tlc_color_index++;
                    } // otherwise we keep the same color for all the RGB LEDs.

                    break;
                }
                rgb_element_index++;
            }
            tlc_tx_index++;
        } else if (tlc_send_type == TLC_SEND_TYPE_FUN) {
            if (tlc_tx_index == 18) { // after 18 we have to switch to 7-bit mode.
                UCA0CTLW0 |= UCSWRST;
                UCA0CTLW0 |= UC7BIT;
                UCA0CTLW0 &= ~UCSWRST;
                EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
            } else if (tlc_tx_index == 34) {
                GPIO_pulse(TLC_LATPORT, TLC_LATPIN); // LATCH.
                UCA0CTLW0 |= UCSWRST;
                UCA0CTLW0 &= ~UC7BIT;
                UCA0CTLW0 &= ~UCSWRST;
                EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
                tlc_send_type = TLC_SEND_IDLE;
                break;
            }
            EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, fun_base[tlc_tx_index]);
            tlc_tx_index++;
        } else if (tlc_send_type == TLC_SEND_TYPE_LB) { // Loopback for POST
            if (tlc_tx_index == 33) {
                tlc_send_type = TLC_SEND_IDLE;
                break;
            }
            EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, tlc_loopback_data_out);
            tlc_tx_index++;
        } else {
            tlc_send_type = TLC_SEND_IDLE; // probably shouldn't reach.
        }
        break; // End of TXIFG /////////////////////////////////////////////////////

    default: break;
    } // End of ISR flag switch ////////////////////////////////////////////////////
}

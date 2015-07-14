// System includes:
#include <stdint.h>
#include <grlib.h>
#include <stdio.h>
#include <string.h>

// Grace includes:
#include <ti/mcu/msp430/Grace.h>

// Project includes:
#include <qc12_oled.h>
#include "img.h"
#include "qc12.h"
#include "radio.h"
#include "leds.h"
#include "oled.h"

// Interrupt flags:
volatile uint8_t f_bl = 0;
volatile uint8_t f_br = 0;
volatile uint8_t f_bs = 0;
volatile uint8_t f_time_loop = 0;
volatile uint8_t f_new_second = 0;

// Function declarations:
void poll_buttons();

#pragma DATA_SECTION (my_conf, ".infoA");
#pragma DATA_SECTION (default_conf, ".infoB");

qc12conf my_conf;
const qc12conf backup_conf;

const qc12conf default_conf = {
        0,
        50,
        0,
        0,
        "", // NB: this is required to be a 0
        0x0000
};

const char titles[][10] = {
        "n00b",
        "UBER",
        "Spastic",
        "Bored",
        "Socialite",
};

// The code:

void check_conf() {

    CRC_setSeed(CRC_BASE, 0x0C12);
    for (uint8_t i = 0; i < sizeof(qc12conf) - 2; i++) {
        CRC_set8BitData(CRC_BASE, ((uint8_t *) &default_conf)[i]);
    }

    if (my_conf.crc16 != CRC_getResult(CRC_BASE)) {
        memcpy(&my_conf, &default_conf, sizeof(qc12conf));

        CRC_setSeed(CRC_BASE, 0x0C12);
        for (uint8_t i = 0; i < sizeof(qc12conf) - 2; i++) {
            CRC_set8BitData(CRC_BASE, ((uint8_t *) &default_conf)[i]);
        }
        my_conf.crc16 = CRC_getResult(CRC_BASE);
    }
}

void init_rtc() {
    RTC_B_definePrescaleEvent(RTC_B_BASE, RTC_B_PRESCALE_1, RTC_B_PSEVENTDIVIDER_4); // 32 Hz
    RTC_B_clearInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
    RTC_B_enableInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
}

void init() {
    Grace_init(); // Activate Grace-generated configuration

    check_conf();

    init_tlc();
    init_radio();
    init_oled();
    init_rtc();
}

// Power-on self test
void post() {

}

// Play a cute animation when we first turn the badge on.
void intro() {
    GrImageDraw(&g_sContext, &fingerprint_1BPP_UNCOMP, 0, 0);
    GrStringDrawCentered(&g_sContext, "Queercon", -1, 31, 94 + SYS_FONT_HEIGHT/3, 0);
    GrStringDrawCentered(&g_sContext, "twelve", -1, 31, 94 + SYS_FONT_HEIGHT/3 + SYS_FONT_HEIGHT, 0);
    GrStringDrawCentered(&g_sContext, "- 2015 -", -1, 31, 94 + SYS_FONT_HEIGHT/3 + SYS_FONT_HEIGHT*2, 0);
    GrFlush(&g_sContext);
}

const tRectangle name_erase = {0, NAME_Y_OFFSET, 63, NAME_Y_OFFSET + NAME_FONT_HEIGHT + SYS_FONT_HEIGHT};

void get_name() {

    if (my_conf.handle[0])
        return; // Already have a name set.

    GrClearDisplay(&g_sContext);

    GrContextFontSet(&g_sContext, &SYS_FONT);
    GrStringDraw(&g_sContext, "Enter a", -1, 0, 5, 1);
    GrStringDraw(&g_sContext, "name.", -1, 0, 5+SYS_FONT_HEIGHT, 1);
    GrStringDraw(&g_sContext, "Hold", -1, 0, 5+SYS_FONT_HEIGHT*3, 1);
    GrStringDraw(&g_sContext, "middle", -1, 0, 5+SYS_FONT_HEIGHT*4, 1);
    GrStringDraw(&g_sContext, "button", -1, 0, 5+SYS_FONT_HEIGHT*5, 1);
    GrStringDraw(&g_sContext, "to finish.", -1, 0, 5+SYS_FONT_HEIGHT*6, 1);

    GrContextFontSet(&g_sContext, &NAME_FONT); // &g_sFontFixed6x8);

    GrFlush(&g_sContext);

    uint8_t char_entry_index = 0;
    uint8_t curr_char = ' ';
    char name[NAME_MAX_LEN+1] = {0};
    name[0] = ' ';
    char undername[2] = "X";
    undername[0] = NAME_SEL_CHAR;
    uint8_t underchar_x = 0;
    uint8_t text_width = 0;
    uint8_t last_char_index = 0;

    uint8_t bs_down_cycles = 0;

    while (1) {
        if (f_time_loop) {
            f_time_loop = 0;
            poll_buttons();
            // Check for left/right buttons to change character slot
            text_width = GrStringWidthGet(&g_sContext, name, last_char_index+1);
            if (f_bl == BUTTON_RELEASE) {
                if (char_entry_index > 0) {
                    // check for deletion:
                    if (char_entry_index == last_char_index && curr_char == ' ')
                        last_char_index--;

                    char_entry_index--;
                    curr_char = name[char_entry_index];
                }
                f_bl = 0;
            }
            if (f_br == BUTTON_RELEASE) {
                if (char_entry_index < NAME_MAX_LEN && curr_char != ' ' && text_width < 58) {
                    char_entry_index++;
                    if (!name[char_entry_index])
                        name[char_entry_index] = ' ';
                    curr_char = name[char_entry_index];
                    if (char_entry_index > last_char_index)
                        last_char_index = char_entry_index;
                }
                f_br = 0;
            }
            if (f_bs == BUTTON_RELEASE) {
                if (curr_char == 'Z')
                    curr_char = 'a';
                else if (curr_char == 'z')
                    curr_char = '0';
                else if (curr_char == '9')
                    curr_char = ' ';
                else if (curr_char == ' ')
                    curr_char = 'A';
                else
                    curr_char++;
                name[char_entry_index] = curr_char;
                f_bs = 0;
                bs_down_cycles = 0;
            } else if ((last_char_index || name[0] != ' ') && f_bs == BUTTON_PRESS) {
                bs_down_cycles = 1;
                f_bs = 0;
            }

            if (bs_down_cycles && bs_down_cycles < NAME_COMMIT_CYCLES) {
                bs_down_cycles++;
            } else if (bs_down_cycles) {
                break;
            }

            underchar_x = GrStringWidthGet(&g_sContext, name, char_entry_index);

            // Clear the area:
            GrContextForegroundSet(&g_sContext, ClrBlack);
            GrRectFill(&g_sContext, &name_erase);
            GrContextForegroundSet(&g_sContext, ClrWhite);

            // Rewrite it:
            GrStringDraw(&g_sContext, name, -1, 0, NAME_Y_OFFSET, 1);
            GrLineDrawH(&g_sContext, 0, text_width, NAME_Y_OFFSET+12);
            GrStringDraw(&g_sContext, undername, -1, underchar_x, NAME_Y_OFFSET+13, 1);
            GrFlush(&g_sContext);
        }
    }

    uint8_t name_len = 0;
    while (name[name_len] && name[name_len] != ' ')
        name_len++;
    name[name_len] = 0; // null terminate.
    strcpy(my_conf.handle, name);
} // get_name

// TODO: This should be temporary:
uint8_t rainbow_interval = 6;

int main(void)
{
    init();
    post();
    intro(); // Play a cute animation when we first turn the badge on.
    delay(8000);
    get_name(); // Learn the badge's name (if we don't have it already)

    // TODO: Reset all the flags

    GrClearDisplay(&g_sContext);
    oled_draw_pane();

    oled_play_animation(standing, 0);
    oled_anim_next_frame();

    tlc_stage_blank(0);
    tlc_set_fun();

    while (1) {
        if (f_time_loop) {
            f_time_loop = 0;

            // Debounce & generate flags from the buttons:
            poll_buttons();

            // New LED animation frame if needed:
            if (!--rainbow_interval) {
                oled_anim_next_frame();
                rainbow_interval = 3;
                tlc_timestep();
            }


            // Do stuff:

            if (f_bl) {
                oled_play_animation(standing, 0);
                f_bl = 0;
            }
            if (f_bs) {
                oled_play_animation(walking, 2);
                f_bs = 0;
            }
            if (f_br) {
                oled_play_animation(waving, 3);
                f_br = 0;
            }

        }
        if (f_new_second) {
            f_new_second = 0;
        }

        // TODO: check all flags?
//        __bis_SR_register(SLEEP_BITS);
    }
} // main

void poll_buttons() {

    static uint8_t bl_read_prev = 1;
    static uint8_t bl_read = 1;
    static uint8_t bl_state = 1;

    static uint8_t br_read_prev = 1;
    static uint8_t br_read = 1;
    static uint8_t br_state = 1;

    static uint8_t bs_read_prev = 1;
    static uint8_t bs_read = 1;
    static uint8_t bs_state = 1;


    // Poll the buttons two time loops in a row to debounce and
    // if there's a change, raise a flag.
    // Left button:
    bl_read = GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN6);
    if (bl_read == bl_read_prev && bl_read != bl_state) {
        f_bl = bl_read? BUTTON_RELEASE : BUTTON_PRESS; // active high
        bl_state = bl_read;
    }
    bl_read_prev = bl_read;

    // Softkey button:
    bs_read = GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5);
    if (bs_read == bs_read_prev && bs_read != bs_state) {
        f_bs = bs_read? BUTTON_RELEASE : BUTTON_PRESS; // active high
        bs_state = bs_read;
    }
    bs_read_prev = bs_read;

    // Right button:
    br_read = GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN4);
    if (br_read == br_read_prev && br_read != br_state) {
        f_br = br_read? BUTTON_RELEASE : BUTTON_PRESS; // active high
        br_state = br_read;
    }
    br_read_prev = br_read;
} // poll_buttons

#pragma vector=RTC_VECTOR
__interrupt
void RTC_A_ISR(void) {
    switch (__even_in_range(RTCIV, 16)) {
    case 0: break;  //No interrupts
    case 2:         //RTCRDYIFG
        f_new_second = 1;
        __bic_SR_register_on_exit(SLEEP_BITS);
        break;
    case 4:         //RTCEVIFG
        //Interrupts every minute // We don't use this.
        __bic_SR_register_on_exit(SLEEP_BITS);
        break;
    case 6:         //RTCAIFG
        // Alarm!
        break;
    case 8: break;  //RT0PSIFG
    case 10:		// Rollover of prescale counter
        f_time_loop = 1; // We know what it does! It's a TIME LOOP MACHINE.
        // ...who would build a device that loops time every 32 milliseconds?
        // WHO KNOWS. But that's what it does.
        __bic_SR_register_on_exit(SLEEP_BITS);
        break; //RT1PSIFG
    case 12: break; //Reserved
    case 14: break; //Reserved
    case 16: break; //Reserved
    default: break;
    }
} // RTC_A_ISR

#include <stdint.h>
#include <grlib.h>
#include <qc12_oled.h>
#include <stdio.h>

// Grace includes:
#include <ti/mcu/msp430/Grace.h>

// Project includes:
#include "img.h"
#include "qc12.h"
#include "radio.h"
#include "leds.h"
#include "oled.h"

uint8_t f_bl = 0;
uint8_t f_br = 0;
uint8_t f_bs = 0;

volatile uint8_t f_time_loop = 0;
volatile uint8_t f_new_second = 0;

void init_rtc() {
    RTC_B_definePrescaleEvent(RTC_B_BASE, RTC_B_PRESCALE_1, RTC_B_PSEVENTDIVIDER_4); // 32 Hz
    RTC_B_clearInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
    RTC_B_enableInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
}

void init() {
    Grace_init(); // Activate Grace-generated configuration

    /*
     * Peripherals:
     *
     *  Radio (RFM69CW)
     *        (MSB first, clock inactive low,
     *         write on rise, change on fall, MSB first)
     *        eUSCI_B0 - radio
     *        somi, miso, clk, ste
     *        DIO0      P3.1
     *        RESET     P3.2
     */
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
    GrStringDraw(&g_sContext, "QC12", -1, 0, 94, 1);
    GrFlush(&g_sContext);
}

void poll_buttons();

#define UNDERNAME_SEL_CHAR '*'
#define MAX_NAME_LEN 12
#define NAME_COMMIT_CYCLES 80

void get_name() {
    GrClearDisplay(&g_sContext);
    GrStringDraw(&g_sContext, "Enter a", -1, 0, 10, 1);
    GrStringDraw(&g_sContext, "name.", -1, 0, 20, 1);
    GrStringDraw(&g_sContext, "Hold", -1, 0, 40, 1);
    GrStringDraw(&g_sContext, "middle", -1, 0, 50, 1);
    GrStringDraw(&g_sContext, "button", -1, 0, 60, 1);
    GrStringDraw(&g_sContext, "to finish.", -1, 0, 70, 1);

    GrContextFontSet(&g_sContext, &g_sFontCmsc12); // &g_sFontFixed6x8);

    GrFlush(&g_sContext);

    uint8_t name_y_offset = 90;
    uint8_t char_entry_index = 0;
    uint8_t curr_char = ' ';
    char name[MAX_NAME_LEN+1] = "        ";
    char undername[2] = "X";
    undername[0] = UNDERNAME_SEL_CHAR;
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
                if (char_entry_index < MAX_NAME_LEN && curr_char != ' ' && text_width < 58) {
                    char_entry_index++;
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
                GrStringDraw(&g_sContext, name, -1, 0, name_y_offset, 1);
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
            GrContextFontSet(&g_sContext, &g_sFontCmtt12); // &g_sFontFixed6x8);
            GrStringDraw(&g_sContext, "        ", -1, 0, name_y_offset+13, 1);
            GrContextForegroundSet(&g_sContext, ClrBlack);
            GrLineDrawH(&g_sContext, 0, 64, name_y_offset+12);
            GrContextForegroundSet(&g_sContext, ClrWhite);
            GrLineDrawH(&g_sContext, 0, text_width, name_y_offset+12);
            GrStringDraw(&g_sContext, undername, -1, underchar_x, name_y_offset+13, 1);
            GrContextFontSet(&g_sContext, &g_sFontCmsc12); // &g_sFontFixed6x8);
            GrFlush(&g_sContext);
        }
    }
}

// TODO: This should be temporary:
uint8_t rainbow_interval = 4;

int main(void)
{
    init();
    post();
    intro(); // Play a cute animation when we first turn the badge on.
    delay(2000);
    // TODO: persistent.
    get_name(); // Learn the badge's name (if we don't have it already)

    // TODO: Reset all the flags

    GrClearDisplay(&g_sContext);
    oled_draw_pane();

    oled_play_animation(standing, 0);
    oled_anim_next_frame();

//    tlc_stage_blank(1);
//    tlc_set_fun();


    while (1) {
        if (f_time_loop) {
            f_time_loop = 0;

            // Debounce & generate flags from the buttons:
            poll_buttons();

            // New LED animation frame if needed:
            if (!--rainbow_interval) {
                oled_anim_next_frame();
                rainbow_interval = 12;
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

        __bis_SR_register(SLEEP_BITS);
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

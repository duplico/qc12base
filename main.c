#include <driverlib/MSP430FR5xx_6xx/wdt_a.h>

// System includes:
#include <stdint.h>
#include <grlib.h>
#include <stdio.h>
#include <leds.h>
#include <string.h>
#include <stdlib.h>

#include <driverlib/MSP430FR5xx_6xx/driverlib.h>
// Grace includes:
#include <ti/mcu/msp430/Grace.h>

// Project includes:
#include <qc12_oled.h>
#include <img.h>
#include <qc12.h>
#include <radio.h>
#include <oled.h>

// Interrupt flags:
volatile uint8_t f_bl = 0;
volatile uint8_t f_br = 0;
volatile uint8_t f_bs = 0;
volatile uint8_t f_time_loop = 0;
volatile uint8_t f_new_second = 0;
volatile uint8_t f_rfm_rx_done = 0;
volatile uint8_t f_rfm_tx_done = 0;
volatile uint8_t f_tlc_anim_done = 0;
volatile uint8_t s_new_minute = 0;
volatile uint8_t f_radio_fault = 0;

// Non-interrupt signal flags (no need to avoid optimization):
uint8_t s_default_conf_loaded = 0;
uint8_t s_need_rf_beacon = 0;
uint8_t s_flag_wave = 0;
uint8_t s_flag_send = 0;
uint8_t s_new_uber_seen = 0;
uint8_t s_new_badge_seen = 0;
uint8_t s_new_uber_friend = 0;
uint8_t s_new_friend = 0;
uint8_t s_oled_needs_redrawn_idle = 0;
uint8_t s_overhead_done;
uint8_t s_befriend_failed = 0;
uint8_t s_befriend_success = 0;
uint8_t s_new_checkin = 0;
uint8_t s_send_play = 0;
uint8_t s_send_puppy = 0;

uint8_t puppy_target = 0;

uint8_t disable_beacon_service = 0;
uint8_t idle_mode_softkey_sel = 0;
uint8_t idle_mode_softkey_dis = 0;

uint8_t s_need_idle_action = 0;
uint8_t idle_action_seconds = 10;

uint8_t minute_secs = 60;

uint8_t befriend_mode = 0;
uint8_t befriend_mode_loops_to_tick = 0;
uint8_t befriend_mode_ticks = 0;
uint8_t befriend_mode_secs = 0;
uint8_t befriend_candidate = 0;
char befriend_candidate_handle[NAME_MAX_LEN+1] = {0};
uint8_t befriend_candidate_age = 0;

#define PLAY_MODE_CAUSE 1
#define PLAY_MODE_OBSERVE 2
#define PLAY_MODE_RECV 3
#define PLAY_MODE_EFFECT 4
#define PLAY_MODE_CAUSE_ALONE 5

uint8_t play_mode = 0;
uint8_t play_id = 0;

#define BF_S_BEACON 1
#define BF_S_OPEN   3
#define BF_S_WAIT   5
#define BF_C_OPENING 2
#define BF_C_CLOSING 4
#define BF_C_WAIT   6

#define FLAG_COUNT 14

uint8_t flag_id = 0;

uint8_t am_puppy = 0;

void poll_buttons();

#pragma DATA_SECTION (my_conf, ".infoA");
#pragma DATA_SECTION (default_conf, ".infoB");

// This is PERSISTENT:
//  That means that it will be CLOBBERED by the DEBUG TOOLCHAIN ONLY.
//  But it WON'T be on power cycle.
qc12conf my_conf = {0};

const qc12conf default_conf = {
    DEDICATED_BASE_ID,  // id
    0,  // mood
};

#pragma PERSISTENT(badge_seen_ticks)
uint16_t badge_seen_ticks[BADGES_IN_SYSTEM] = {0};
#pragma PERSISTENT(badges_seen)
uint8_t badges_seen[BADGES_IN_SYSTEM] = {0};
#pragma PERSISTENT(fav_badges_ids)
uint8_t fav_badges_ids[FAVORITE_COUNT] = {0};
#pragma PERSISTENT(fav_badges_handles)
char fav_badges_handles[FAVORITE_COUNT][NAME_MAX_LEN+1] = {0};

// Gaydar:
uint8_t window_position = 0; // only used for skipping windows.
uint8_t skip_window = 1;
uint8_t neighbor_count = 0;
uint8_t window_seconds = RECEIVE_WINDOW_LENGTH_SECONDS;
uint8_t neighbor_badges[BADGES_IN_SYSTEM] = {0};
uint8_t at_base = 0;
uint8_t at_suite_base = 0;

// In the "modal" sense:
uint8_t op_mode = OP_MODE_IDLE;

uint8_t suppress_softkey = 0;
uint16_t softkey_en = 0;

const char sk_labels[SK_SEL_MAX+1][12] = {
        "Unlock",
        "Lock",
        "B: Off",
        "B: Suite", // base ID 1, so we send sk_label index - 2
        "B: Pool",
        "B: Kickoff",
        "B: Mixer",
        "B: Talk",
        "Flag",
        "Set flag",
};

const char base_labels[][12] = { // so we send label index + 1
        "qcsuite",
        "pool",
        "kickoff",
        "mixer",
        "badgetalk"
};

void my_conf_write_crc() {
    if (my_conf.locked) {
        softkey_en = BIT0;
    } else {
        softkey_en = 0x3FE;
    }

    CRC_setSeed(CRC_BASE, 0x0C12);
    for (uint8_t i = 0; i < sizeof(qc12conf) - 2; i++) {
        CRC_set8BitData(CRC_BASE, ((uint8_t *) &default_conf)[i]);
    }
    my_conf.crc16 = CRC_getResult(CRC_BASE);
}

void check_conf() {
    CRC_setSeed(CRC_BASE, 0x0C12);
    for (uint8_t i = 0; i < sizeof(qc12conf) - 2; i++) {
        CRC_set8BitData(CRC_BASE, ((uint8_t *) &default_conf)[i]);
    }

    // Load default config:
    if (my_conf.crc16 != CRC_getResult(CRC_BASE)) {
        memcpy(&my_conf, &default_conf, sizeof(qc12conf));
        memset(badges_seen, 0, sizeof(uint16_t) * BADGES_IN_SYSTEM);
        memset(fav_badges_ids, 0xff, sizeof(uint8_t) * FAVORITE_COUNT);
        memset(fav_badges_handles, 0, sizeof(char) * FAVORITE_COUNT * (NAME_MAX_LEN+1));
        s_default_conf_loaded = 1;
        out_payload.handle[0] = 0;
        my_conf.locked = 1;
        my_conf.base_id = NOT_A_BASE;
        my_conf.flag_unlocks = START_WITH_FLAGS? 0xFF: 0;
        my_conf_write_crc();
        // Suppress any flags set from these so we don't do the animation:
        s_new_uber_seen = s_new_badge_seen = s_new_uber_friend = s_new_friend = 0;
    }

    am_puppy = 0;
}

void init_rtc() {
    RTC_B_definePrescaleEvent(RTC_B_BASE, RTC_B_PRESCALE_1, RTC_B_PSEVENTDIVIDER_2); // 4 => 32 Hz; 2 => 64 Hz
    RTC_B_clearInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
    RTC_B_enableInterrupt(RTC_B_BASE, RTC_B_CLOCK_READ_READY_INTERRUPT + RTC_B_TIME_EVENT_INTERRUPT + RTC_B_CLOCK_ALARM_INTERRUPT + RTC_B_PRESCALE_TIMER1_INTERRUPT);
}

void init_payload() {
    out_payload.base_id = NOT_A_BASE;
    out_payload.beacon = 0;
    out_payload.flag_id = 0;
    out_payload.play_id = 0;
    out_payload.friendship = 0;
    out_payload.from_addr = my_conf.badge_id;
    out_payload.to_addr = RFM_BROADCAST;
    memset(out_payload.handle, 0, NAME_MAX_LEN+1);
}

void init() {
    Grace_init(); // Activate Grace-generated configuration

    check_conf();

    my_conf.locked = 1;
    my_conf_write_crc();

    init_payload();
    init_tlc();
    init_radio();
    init_oled();
    init_rtc();
    srand(my_conf.badge_id);
}

// Power-on self test
void post() {
    // If the radio is super-broken, we won't even get here.
    // Check the crystal.
    uint8_t crystal_error = CSCTL5 & LFXTOFFG;

    // Check the shift register of the TLC.
    uint8_t led_error = tlc_test_loopback(0b10101010);
    led_error = led_error || tlc_test_loopback(0b01010101);

    // If we detected no errors, we're done here.
    if (!crystal_error)
        return;

    // Otherwise, show those errors and then delay for a bit so we can
    // see them.
    GrClearDisplay(&g_sContext);
    GrContextFontSet(&g_sContext, &SYS_FONT);
    uint8_t print_loc = 32;

    GrStringDraw(&g_sContext, "- POST -", -1, 0, 5, 0);

    if (crystal_error) {
        GrStringDraw(&g_sContext, "Err: XTAL!", -1, 0, print_loc, 0);
        print_loc += 12;
    }

    GrFlush(&g_sContext);

    delay(5000);
}

// Play a cute animation when we first turn the badge on.
void intro() {
    GrStringDrawCentered(&g_sContext, "qc12base", -1, 31, 10, 0);
    GrImageDraw(&g_sContext, &fingerprint_1BPP_UNCOMP, 0, 18);
    GrStringDrawCentered(&g_sContext, "- 2015 -", -1, 31, 125 - SYS_FONT_HEIGHT/2, 0);
    GrFlush(&g_sContext);
}

void radio_send_beacon(uint8_t address, uint8_t beacons) {
    out_payload.beacon = beacons;
    out_payload.flag_id = 0;
    out_payload.friendship = 0;
    out_payload.from_addr = my_conf.badge_id;
    out_payload.to_addr = address;
    out_payload.play_id = 0;
    out_payload.base_id = my_conf.base_id;
    radio_send_sync();
}

void radio_send_flag(uint8_t flag) {
    out_payload.beacon = 0;
    out_payload.flag_id = flag;
    out_payload.friendship = 0;
    out_payload.from_addr = 0;
    out_payload.to_addr = RFM_BROADCAST;
    out_payload.play_id = 0;
    out_payload.base_id = NOT_A_BASE;
    radio_send_sync();
    out_payload.from_addr = my_conf.badge_id;
}

void handle_infrastructure_services() {
    static uint8_t flag_in_cooldown = 0;

    // Handle inbound and outbound background radio functionality, and buttons.
    if (f_rfm_tx_done) {
        f_rfm_tx_done = 0;
        // And return to our normal receive automode:
        // RX->SB->RX on receive.
        mode_sync(RFM_MODE_RX);
        write_single_register(0x3b, RFM_AUTOMODE_RX);
    }

    // Radio RX tasks:

    if (f_rfm_rx_done) {
        radio_recv(); // If the CRC is bad, this will unset the flag.
    }

    // Got a probably valid message:
    if (f_rfm_rx_done && in_payload.from_addr != my_conf.badge_id) {
        f_rfm_rx_done = 0;
        in_payload.handle[NAME_MAX_LEN] = 0; // Make sure it's definitely null-terminated.

    } else if (f_rfm_rx_done) {
        // Ignore messages from our own ID, because what even is that anyway?
        f_rfm_rx_done = 0;
    }

    // Poll buttons and check whether we need to resend a befriend message:
    if (f_time_loop) {
        WDT_A_resetTimer(WDT_A_BASE); // pat pat pat
    }

    if (f_new_second) {
        if (f_radio_fault) {
            f_radio_fault = 0;
            init_radio();
        }

        if (flag_in_cooldown) {
            flag_in_cooldown--;
        }

        minute_secs--;
        if (!minute_secs) {
            minute_secs = 60;
            s_new_minute = 1;
        }

        window_seconds--;
        if (!window_seconds) {
            window_seconds = RECEIVE_WINDOW_LENGTH_SECONDS;
            if (skip_window != window_position) {
                s_need_rf_beacon = 1;
            }

            neighbor_count = 0;
            for (uint8_t i=0; i<BADGES_IN_SYSTEM; i++) {
                if (neighbor_badges[i]) {
                    neighbor_count++;
                    neighbor_badges[i]--;
                }
            }

            window_position = (window_position + 1) % RECEIVE_WINDOW;
            if (!window_position) {
                skip_window = rand() % RECEIVE_WINDOW;
                // If there's no one around, reset the radio just in case.
                if (!neighbor_count && rfm_state == RFM_IDLE) {
                    mode_sync(RFM_MODE_SL); // Going to sleep... mode...
                    write_single_register(0x3b, RFM_AUTOMODE_OFF);
                    init_radio();
                }
            }
        }
    }

    if (s_new_minute) {
        s_new_minute = 0;

        // NB: overflows every 8166.13 years:
        my_conf.uptime++;

        if (am_puppy && !(my_conf.uptime % 60)) {
            // Send it a few times:
            s_send_puppy = 4;
            puppy_target = 1 + rand() % neighbor_count;
        }

        my_conf_write_crc();
    }

    if (s_flag_send && rfm_state == RFM_IDLE) {
        s_flag_send--;
        flag_in_cooldown = FLAG_IN_COOLDOWN_SECONDS;
        radio_send_flag(flag_id | BIT7);
    }

    if (!disable_beacon_service && s_need_rf_beacon && rfm_state == RFM_IDLE) {
        // If we need to beacon, and we're not talking to the RFM module.
        // Note: Last year we also had a check for
        //  "!(read_single_register_sync(0x27) & (BIT1+BIT0))".
        // That is SyncAddressMatch and AutoMode (i.e. check whether we're
        // receiving something or are in our receive intermediate state.)
        // I'm not sure it added any robustness.
        s_need_rf_beacon = 0;
        radio_send_beacon(RFM_BROADCAST, 1);
    }
}

void handle_oled_actions() {
    if (f_time_loop) {
        oled_timestep();
    }

    if (f_new_second) {
    }

}

void try_to_sleep() {
    // If there are no more flags left to service, go to sleep.
    if (!(f_bl || f_br || f_bs || f_new_second || f_rfm_rx_done || f_rfm_tx_done || f_time_loop)) {
        __bis_SR_register(SLEEP_BITS);
    }
}

// Read the badgeholder's name if appropriate:
uint8_t unlock() {
    // Clear the screen and display the instructions.
    static tRectangle name_erase_rect = {0, NAME_Y_OFFSET, 63, NAME_Y_OFFSET + NAME_FONT_HEIGHT + SYS_FONT_HEIGHT};
    static uint8_t update_disp;
    update_disp = 1;

    GrClearDisplay(&g_sContext);
    GrContextFontSet(&g_sContext, &SYS_FONT);
    oled_print(0, 5, "Password please.", 1, 0);
    GrFlush(&g_sContext);

    // Switch to the NAME font so it's the expected width.
    GrContextFontSet(&g_sContext, &NAME_FONT);

    // Temporary buffer to hold the selected name.
    // (In the event of a power cycle we don't wand to be messing around
    //  with the actual config's handle)
    char name[NAME_MAX_LEN+1] = {' ', 0};
    uint8_t char_entry_index = 0;
    uint8_t curr_char = ' ';

    // String to display under the name; it's just the selection character,
    // configured in qc12.h
    const char undername[2] = {NAME_SEL_CHAR, 0};

    // For figuring out where to put the underline & selection character:
    uint8_t underchar_x = 0;
    uint8_t text_width = 0;
    uint8_t last_char_index = 0;

    // For determining whether name entry is complete:
    uint8_t bs_down_loops = 0;
    text_width = GrStringWidthGet(&g_sContext, name, last_char_index+1);

    while (1) {
        handle_infrastructure_services();

        if (f_new_second) {
            f_new_second = 0;
        }

        if (f_time_loop) {
            f_time_loop = 0;
            // Check for left/right buttons to change character slot
            if (f_bl == BUTTON_RELEASE) {
                if (char_entry_index > 0) {
                    // check for deletion:
                    if (char_entry_index == last_char_index) { // was: && curr_char == ' ')
                        name[char_entry_index] = ' ';
                        last_char_index--;
                    }
                    char_entry_index--;
                    curr_char = name[char_entry_index];
                    update_disp = 1;
                    text_width = GrStringWidthGet(&g_sContext, name, last_char_index+1);
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
                    update_disp = 1;
                    text_width = GrStringWidthGet(&g_sContext, name, last_char_index+1);
                }
                f_br = 0;
            }
            if (f_bs == BUTTON_RELEASE) {
                // Softkey button cycles the current character.
                // This is a massive PITA for the person entering their name,
                // but they only have to do it once so whatever.
                if (curr_char == 'Z') // First comes capital letters
                    curr_char = 'a';
                else if (curr_char == 'z') // Then lower case
                    curr_char = '0';
                else if (curr_char == '9') // Then numbers
                    curr_char = ' ';
                else if (curr_char == ' ') // Then a space, and then we cycle.
                    curr_char = 'A';
                else
                    curr_char++;
                name[char_entry_index] = curr_char;
                update_disp = 1;
                f_bs = 0;
                // Since it's released, clear the depressed loop count.
                bs_down_loops = 0;
            } else if ((last_char_index || name[0] != ' ') && f_bs == BUTTON_PRESS) {
                // If we're in a valid state to complete the name entry, and
                // the softkey button is depressed, then it's time to start
                // counting the number of time loops for which it is depressed.
                bs_down_loops = 1;
                f_bs = 0;
            }

            // If we're counting the number of loops for which the softkey is
            // depressed, go ahead and increment it. This is going to do one
            // double-count at the beginning, but I don't care.
            if (bs_down_loops && bs_down_loops < NAME_COMMIT_LOOPS) {
                bs_down_loops++;
            } else if (bs_down_loops) {
                break;
            }

            if (update_disp) {
                update_disp = 0;
                underchar_x = GrStringWidthGet(&g_sContext, name, char_entry_index);

                // Clear the area:
                GrContextForegroundSet(&g_sContext, ClrBlack);
                GrRectFill(&g_sContext, &name_erase_rect);
                GrContextForegroundSet(&g_sContext, ClrWhite);

                // Rewrite it:
                GrStringDraw(&g_sContext, name, -1, 0, NAME_Y_OFFSET, 1);
                GrLineDrawH(&g_sContext, 0, text_width, NAME_Y_OFFSET+12);
                GrStringDraw(&g_sContext, undername, -1, underchar_x, NAME_Y_OFFSET+13, 1);
                GrFlush(&g_sContext);
            }
        } // end if (f_time_loop)

        try_to_sleep();

    } // end while (1)

    // Commit the name with a correctly placed null termination character..
    uint8_t name_len = 0;
    while (name[name_len] && name[name_len] != ' ')
        name_len++;
    name[name_len] = 0; // null terminate.

    GrClearDisplay(&g_sContext);
    GrFlush(&g_sContext);

    suppress_softkey = 1;

    if (!strcmp(name, "OKHOMOr")) {
        // unlock with rainbows
        my_conf.locked = 0;
        my_conf.flag_unlocks = 0xFF;
        my_conf_write_crc();
        return 1;
    } else if (!strcmp(name, "OKHOMOn")) {
        // unlock
        my_conf.locked = 0;
        my_conf.flag_unlocks = 0;
        my_conf_write_crc();
        return 1;
    } else if (!strcmp(name, "OKHOMO")) {
        // unlock
        my_conf.locked = 0;
        my_conf_write_crc();
        return 1;
    } else {
        return 0; // fail
    }
} // handle_mode_name

uint8_t softkey_enabled(uint8_t index) {
    if (index == SK_SEL_SETFLAG && !my_conf.flag_unlocks) {
        return 0;
    }
    return ((1<<index) & softkey_en)? 1 : 0;
}

void handle_mode_idle() {
    // Clear any outstanding stray flags asking the character to do stuff
    //    so we know we're in a consistent state when we enter this mode.
    if (!softkey_enabled(idle_mode_softkey_sel))
        idle_mode_softkey_sel = 0;
    uint8_t s_new_pane = 1;

    GrClearDisplay(&g_sContext);
    // Pick our current appearance...
    if (oled_anim_state == OLED_ANIM_DONE) {
        s_oled_needs_redrawn_idle = 1;
    }
    if (oled_overhead_type == OLED_OVERHEAD_OFF) {
        s_overhead_done = 1;
    }

    if (tlc_anim_mode == TLC_ANIM_MODE_IDLE) {
        tlc_display_ambient();
    }

    while (1) {
        handle_infrastructure_services();
        handle_oled_actions();

        if (f_new_second) {
            f_new_second = 0;
        }
        if (f_time_loop) {
            f_time_loop = 0;
            if (idle_mode_softkey_dis) {
                f_br = f_bl = f_bs = 0;
            }

            if (f_br == BUTTON_PRESS) {
                // Left button
                do {
                    idle_mode_softkey_sel = (idle_mode_softkey_sel+1) % (SK_SEL_MAX+1);
                } while (!softkey_enabled(idle_mode_softkey_sel));
                s_new_pane = 1;
            }
            f_br = 0;

            if (f_bl == BUTTON_PRESS) {
                do {
                    idle_mode_softkey_sel = (idle_mode_softkey_sel+SK_SEL_MAX) % (SK_SEL_MAX+1);
                } while (!softkey_enabled(idle_mode_softkey_sel));
                s_new_pane = 1;
            }
            f_bl = 0;

            if (f_bs == BUTTON_RELEASE) {
                f_bs = 0;
                // Select button
                switch (idle_mode_softkey_sel) {
                case SK_SEL_UNLOCK:
                    unlock();
                    s_new_pane = 1;
                    idle_mode_softkey_sel = SK_SEL_LOCK;
                    oled_draw_pane_and_flush(idle_mode_softkey_sel);
                    break;
                case SK_SEL_LOCK:
                    my_conf.locked = 1;
                    my_conf_write_crc();
                    idle_mode_softkey_sel = SK_SEL_UNLOCK;
                    s_new_pane = 1;
                    break;
                case SK_SEL_SETFLAG:
                    op_mode = OP_MODE_SETFLAG;
                    break;
                case SK_SEL_BOFF:
                    my_conf.base_id = NOT_A_BASE;
                    my_conf_write_crc();
                    s_new_pane = 1;
                    oled_draw_pane_and_flush(idle_mode_softkey_sel);
                    break;
                default:
                    if (idle_mode_softkey_sel > SK_SEL_MAX) {
                        break;
                    }
                    // I need to SEND:  3 for pool
                    //                  4 for kickoff
                    //                  5 for mixer
                    //                  6 for talk
                    my_conf.base_id = idle_mode_softkey_sel - 1;
                    my_conf_write_crc();
                    op_mode = OP_MODE_IDLE;
                }
            }
        }

        if (s_new_pane) {
            // Title or softkey or something changed:
            s_new_pane = 0;
            oled_draw_pane_and_flush(idle_mode_softkey_sel);
        }

        if (op_mode != OP_MODE_IDLE) {
            break; // Escape the loop!
        }

        try_to_sleep();

    }

    // Cleanup:
    f_bs = 0;
}

void handle_mode_setflag() {
    static uint8_t softkey_sel;
    softkey_sel = my_conf.flag_id;
    uint8_t s_redraw = 1;

    GrClearDisplay(&g_sContext);
    GrImageDraw(&g_sContext, &flag1, 7, 32);
    // Write some instructions or a picture...

    while (1) {
        handle_infrastructure_services();

        if (f_new_second) {
            f_new_second = 0;
        }

        if (f_time_loop) {
            f_time_loop = 0;
            if (f_br == BUTTON_PRESS) {
                softkey_sel = (softkey_sel+1) % (FLAG_COUNT);
                s_redraw = 1;
            }
            f_br = 0;

            if (f_bl == BUTTON_PRESS) {
                softkey_sel = (softkey_sel+(FLAG_COUNT-1)) % (FLAG_COUNT);
                s_redraw = 1;
            }
            f_bl = 0;

            if (f_bs == BUTTON_RELEASE) {
                f_bs = 0;
                // Select button
                my_conf.flag_id = softkey_sel;
                my_conf_write_crc();
                flag_id = my_conf.flag_id;
                s_flag_send = FLAG_SEND_TRIES;
                op_mode = OP_MODE_IDLE;
                break;
            }
            f_bs = 0;
        }

        if (s_redraw) {
            // softkey or something changed:
            s_redraw = 0;
            GrContextFontSet(&g_sContext, &SOFTKEY_LABEL_FONT);
            GrStringDrawCentered(&g_sContext, "                ", -1, 31, 127 - SOFTKEY_FONT_HEIGHT/2, 1);

            if (softkey_sel == demo_anim_count) {
                GrStringDrawCentered(&g_sContext, "Done", -1, 32, 127 - SOFTKEY_FONT_HEIGHT/2, 1);
            } else {
                GrStringDrawCentered(&g_sContext, flags[softkey_sel]->anim_name, -1, 32, 127 - SOFTKEY_FONT_HEIGHT/2, 1);
            }

            GrLineDrawH(&g_sContext, 0, 64, 116);
            GrFlush(&g_sContext);
        }

        try_to_sleep();

    }
    f_bs = 0;

    // Cleanup:
    // (clear all the character stuff)
}

int main(void)
{
    init();
    intro();
    delay(3500);
    post();

    GrClearDisplay(&g_sContext);

    WDT_A_initWatchdogTimer(WDT_A_BASE, WDT_A_CLOCKSOURCE_ACLK, WDT_A_CLOCKDIVIDER_32K);
    WDT_A_start(WDT_A_BASE);

    while (1) {
        switch(op_mode) {
        case OP_MODE_IDLE:
            handle_mode_idle();
            break;
        case OP_MODE_NAME:
            break;
        case OP_MODE_SETFLAG:
            handle_mode_setflag();
            break;
        case OP_MODE_SLEEP:
            break;
        }

        // Reset user-interactive flags after switching modes:
        f_bl = f_br = f_bs = 0;

    } // loop forever. Sleeping is done inside the mode handlers.
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
        f_bl = bl_read? BUTTON_RELEASE : BUTTON_PRESS; // active low
        bl_state = bl_read;
    }
    bl_read_prev = bl_read;

    // Softkey button:
    bs_read = GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5);
    if (bs_read == bs_read_prev && bs_read != bs_state) {
        if (suppress_softkey) {
            // suppress_softkey means we don't generate a flag for the next
            // release (or press, I guess, but we mostly care about releases.)
            suppress_softkey = 0;
        } else {
            f_bs = bs_read? BUTTON_RELEASE : BUTTON_PRESS; // active low
        }
        bs_state = bs_read;
    }
    bs_read_prev = bs_read;

    // Right button:
    br_read = GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN4);
    if (br_read == br_read_prev && br_read != br_state) {
        f_br = br_read? BUTTON_RELEASE : BUTTON_PRESS; // active low
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
        //Interrupts every minute - ignored.
        break;
    case 6:         //RTCAIFG
        // Alarm!
        break;
    case 8: break;  //RT0PSIFG
    case 10:		// Rollover of prescale counter
        f_time_loop = 1; // We know what it does! It's a TIME LOOP MACHINE.
        // ...who would build a device that loops time every 16 milliseconds?
        // WHO KNOWS. But that's what it does.
        poll_buttons();
        tlc_timestep();
        __bic_SR_register_on_exit(SLEEP_BITS);
        break; //RT1PSIFG
    case 12: break; //Reserved
    case 14: break; //Reserved
    case 16: break; //Reserved
    default: break;
    }
} // RTC_A_ISR

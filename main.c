// System includes:
#include <stdint.h>
#include <grlib.h>
#include <stdio.h>
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
#include <leds.h>
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

#define START_BEFRIEND_TLC_ANIM tlc_start_anim(&flag_blue, 0, 30*GLOBAL_TLC_SPEED_SCALE, 1, 0)
#define LED_PULSE(colorflag) tlc_start_anim(&colorflag, 0, 3*GLOBAL_TLC_SPEED_SCALE, 1, 2);

#define BF_S_BEACON 1
#define BF_S_OPEN   3
#define BF_S_WAIT   5
#define BF_C_OPENING 2
#define BF_C_CLOSING 4
#define BF_C_WAIT   6

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
    BADGE_ID,  // id
    50,  // mood
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

// Mood:
uint8_t mood_tick_minutes = MOOD_TICK_MINUTES;

#define ACH_BABY    0
#define ACH_NEWBIE  1
#define ACH_NICE    2
#define ACH_SOCIAL  3
#define ACH_HIP     4
#define ACH_SEXY    5
#define ACH_FRIENDLY 6
#define ACH_COOL    7
#define ACH_STAR    8
#define ACH_BADGER  9
#define ACH_FISH    10
#define ACH_TWIN    11
#define ACH_TWINSY  12
#define ACH_CHILL   13
#define ACH_TEASE   14
#define ACH_PUNKD   15
#define ACH_TIRED   16
#define ACH_SLEEPY  17
#define ACH_SQUIRE  18
#define ACH_ELITE   19
#define ACH_MOODY   20
#define ACH_MIXER   21
#define ACH_KICKOFF 22
#define ACH_CHIEF   23
#define ACH_PUPPY   24
#define ACH_HANDLER 25
#define ACH_TOWEL   26
#define ACH_CHEATER 27

const char titles[NUM_ACHIEVEMENTS][8] = {
        "baby",
        "newbie",
        "nice",
        "social",
        "hip",
        "sexy",
        "friend",
        "cool",
        "star",
        "badger",
        "fish",
        "twin",
        "twinsy!",
        "chill",
        "tease",
        "punk'd",
        "tired",
        "sleepy",
        "squire",
        "elite",
        "moody",
        "queer",
        "early",
        "Chief",
        "Puppy",
        "Handler",
        "Towel",
        "Cheat",
};

const char title_descs[NUM_ACHIEVEMENTS][36] = {
        "Turn it on.",
        "You're all growed up",
        "Meet 20 qcbadges",
        "Meet 40 qcbadges",
        "Meet 60 qcbadges",
        "Sleep with your fave",
        "Make 5 friends",
        "Make 15 friends",
        "Make 50 friends",
        "qcbadge talk: 12p Fri, qcsuite",
        "Go to QC pool party: 9p Fri Bally's",
        "Find your twin",
        "Befriend your twin",
        "Hang around the suite a while",
        "Play a lot",
        "Be played with lot",
        "Be awake 24 hours",
        "Sleep over 8 hours",
        "Meet 10 qc ubers",
        "Befriend 10 qc ubers",
        "Be sad for 8 hours",
        "Go to a QC mixer: 4p daily, qcsuite",
        "Attend QC12 Thurs kickoff",
        "\"Popular!\"",
        "\"Where is he?\"",
        "\"You found him!\"",
        "\"No, you're a towel.\"",
        "\"?????\"",
};


// In the "modal" sense:
uint8_t op_mode = OP_MODE_IDLE;

uint8_t suppress_softkey = 0;
uint16_t softkey_en = 0;

const char sk_labels[SK_SEL_MAX+1][10] = {
        "Play",
        "ASL?",
        "Befriend",
        "Use flag",
        "Set flag",
        "Grow up!",
        "Set name",
        "Sleep"
};

void my_conf_write_crc() {
    if (!my_conf.adult) { // Base child softkeys:
        softkey_en = SK_BIT_ASL | SK_BIT_NAME | SK_BIT_PLAY;

        if (my_conf.time_to_hatch) {
            softkey_en |= SK_BIT_HATCH;
        }

    } else { // Base adult softkeys:
        softkey_en = SK_BIT_ASL | SK_BIT_NAME | SK_BIT_PLAY | SK_BIT_SLEEP | SK_BIT_FRIEND;
        if (my_conf.flag_unlocks) {
            softkey_en |= SK_BIT_SETFLAG | SK_BIT_FLAG;
        }
    }

    CRC_setSeed(CRC_BASE, 0x0C12);
    for (uint8_t i = 0; i < sizeof(qc12conf) - 2; i++) {
        CRC_set8BitData(CRC_BASE, ((uint8_t *) &default_conf)[i]);
    }
    my_conf.crc16 = CRC_getResult(CRC_BASE);
}

void mood_adjust_and_write_crc(int8_t change) {
    if (change < 0) {
        if (my_conf.mood < (-change)) {
            my_conf.mood = 0;
        } else {
            my_conf.mood += change;
        }
    } else if (change > 0) {
        if (my_conf.mood > (100 - change)) {
            my_conf.mood = 100;
        } else {
            my_conf.mood += change;
        }
    }

    if (my_conf.mood >= MOOD_THRESH_SAD) {
        my_conf.sadtime = 0;
    }

    my_conf_write_crc();

    tlc_set_ambient(my_conf.mood);
}

uint8_t achievement_have(uint8_t achievement_id) {
    if (achievement_id >= NUM_ACHIEVEMENTS) {
        return 0;
    }
    uint8_t frame = achievement_id / 8;
    uint8_t bit = 0x01 << (achievement_id % 8);
    if (my_conf.achievements[frame] & bit) {
        return 1;
    } else {
        return 0;
    }
}

void achievement_get(uint8_t achievement_id, uint8_t animate) {
    if (achievement_id >= NUM_ACHIEVEMENTS) {
        return;
    }

    uint8_t frame = achievement_id / 8;
    uint8_t bit = 0x01 << (achievement_id % 8);
    if (!(my_conf.achievements[frame] & bit) || achievement_id == ACH_SEXY) {
        // New achievement. woot.
        my_conf.achievements[frame] |= bit;
        if (achievement_id == ACH_FISH || achievement_id == ACH_KICKOFF || achievement_id == ACH_MIXER) {

        } else {
            my_conf.title_index = achievement_id;
        }
        if (animate && TLC_IS_A_GO) { // If we've nothing better to do with the lights,
            tlc_start_anim(&flag_rainbow, 0, 2*GLOBAL_TLC_SPEED_SCALE, 0, 1);
        }
        s_oled_needs_redrawn_idle = 1;
        mood_adjust_and_write_crc(MOOD_NEW_TITLE);
    } else {
        // Already got it. Boring.
    }
}

inline void set_badge_seen(uint8_t id) {
    if (id >= BADGES_IN_SYSTEM) {
        __no_operation();
        return;
    }

    if (!(BADGE_SEEN_BIT & badges_seen[id])) {
        badges_seen[id] |= BADGE_SEEN_BIT;
        my_conf.seen_count++;

        if (my_conf.seen_count >= 120) {
            achievement_get(ACH_CHIEF, 1);
        } else if (my_conf.seen_count >= 60) {
            achievement_get(ACH_HIP, 1);
        } else if (my_conf.seen_count >= 40) {
            achievement_get(ACH_SOCIAL, 1);
        } else if (my_conf.seen_count >=20) {
           achievement_get(ACH_NICE, 1);
        }

        if (id < UBERS_IN_SYSTEM) {
            my_conf.uber_seen_count++;
            // flag an animation
            if (id != my_conf.badge_id) {
                s_new_uber_seen = SIGNAL_BIT_OLED | SIGNAL_BIT_TLC;
                mood_adjust_and_write_crc(MOOD_NEW_UBER_SEEN);

                // SQUIRE: seen 10 ubers, but not uber:
                if (my_conf.uber_seen_count >= 10 && my_conf.badge_id < UBERS_IN_SYSTEM) {
                    achievement_get(ACH_SQUIRE, 1);
                }
            }
        } else {
            // flag a lamer animation
            if (id != my_conf.badge_id) {
                s_new_badge_seen = SIGNAL_BIT_OLED | SIGNAL_BIT_TLC;
                mood_adjust_and_write_crc(MOOD_NEW_SEEN);

                if (id >= 15 && my_conf.badge_id >= 15) {
                    if ((id-15)%80 == (my_conf.badge_id-15)%80) {
                        achievement_get(ACH_TWIN, 1);
                    }
                }
            }
        }
        my_conf_write_crc();
        // No need to write a CRC here because adjust_mood takes care of that for us.
    }

    if (oled_overhead_type == OLED_OVERHEAD_OFF) {
        s_overhead_done = 1;
    }
}

void set_base_seen(uint8_t base) {
    if (base >= BASES_IN_SYSTEM) {
        return;
    }

    uint16_t base_mask = BIT0 << base;

    if (!(base_mask & my_conf.bases_seen)) {
        s_new_checkin = 1;

        if (base == BASE_KICKOFF) {
            achievement_get(ACH_KICKOFF, 1);
        } else if (base == BASE_POOL) {
            achievement_get(ACH_FISH, 1);
        } else if (base == BASE_TALK) {
            achievement_get(ACH_BADGER, 1);
        } else if (base == BASE_MIXER) {
            achievement_get(ACH_MIXER, 1);
        }

        my_conf.bases_seen |= base_mask;
        mood_adjust_and_write_crc(MOOD_EVENT_ARRIVE);
    }
}

void set_badge_friend(uint8_t id) {
    if (!(id < BADGES_IN_SYSTEM)) {
        return;
    }

    if (!(BADGE_FRIEND_BIT & badges_seen[id])) {
        badges_seen[id] |= BADGE_FRIEND_BIT;
        my_conf.friend_count++;

        if (my_conf.friend_count >= 50) {
            achievement_get(ACH_STAR, 1);
        } else if (my_conf.friend_count >= 15) {
            achievement_get(ACH_COOL, 1);
        } else if (my_conf.friend_count >=5) {
           achievement_get(ACH_FRIENDLY, 1);
        }

        if (id < UBERS_IN_SYSTEM) {
            my_conf.uber_friend_count++;
            // flag an animation
            if (id != my_conf.badge_id) {
                s_new_uber_friend = SIGNAL_BIT_OLED | SIGNAL_BIT_TLC;
                mood_adjust_and_write_crc(MOOD_NEW_UBER_FRIEND);
            }

            // ELITE: seen 10 ubers:
            if (my_conf.uber_friend_count >= 10) {
                achievement_get(ACH_ELITE, 1);
            }

        } else {
            // flag a lamer animation
            if (id != my_conf.badge_id) {
                s_new_friend = SIGNAL_BIT_OLED | SIGNAL_BIT_TLC;
                mood_adjust_and_write_crc(MOOD_NEW_FRIEND);

                if (id >= 15 && my_conf.badge_id >= 15) {
                    if ((id-15)%80 == (my_conf.badge_id-15)%80) {
                        achievement_get(ACH_TWINSY, 1);
                    }
                }
            }
        }
        my_conf_write_crc();
    } else {
        mood_adjust_and_write_crc(MOOD_OLD_FRIEND);
    }

    if (oled_overhead_type == OLED_OVERHEAD_OFF) {
        s_overhead_done = 1;
    }

    if (id != my_conf.badge_id) {
        idle_mode_softkey_dis = 0;
        oled_draw_pane_and_flush(idle_mode_softkey_sel);
    }
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
        set_badge_seen(my_conf.badge_id);
        set_badge_friend(my_conf.badge_id);
        my_conf_write_crc();
        // Suppress any flags set from these so we don't do the animation:
        s_new_uber_seen = s_new_badge_seen = s_new_uber_friend = s_new_friend = 0;
    }

    am_puppy = 0;

    // Set our mood lights.
    mood_adjust_and_write_crc(0);
}

void tick_badge_seen(uint8_t id, char* handle) {
    if (!(id < BADGES_IN_SYSTEM)) {
        return;
    }
    if (badge_seen_ticks[id] < UINT16_MAX) {
        badge_seen_ticks[id]++;
        // do top badges:
        for (uint8_t top_index=0; top_index<FAVORITE_COUNT; top_index++) {
            if (fav_badges_ids[top_index] == id) {
                // If we run into the ID we're currently handling before
                // we run into one that needs to be demoted in its favor,
                // then its rank is OK (because we start at index 0, which
                // is the #1 spot). Let's go ahead and make sure we've still
                // got the most up-to-date handle for them though.
                strcpy(fav_badges_handles[top_index], handle); // handle has already been sanitized.

                break;
            }
            if (fav_badges_ids[top_index] == 0xff || (badge_seen_ticks[id] > badge_seen_ticks[fav_badges_ids[top_index]])) {
                // This is where it goes, and all the rest need to be demoted.

                // So we can start at the lowest ranked favorite, which is fav_badges_ids[FAVORITE_COUNT-1]
                //  and clobber it with its superior until one IS CLOBBERED BY top_index.
                //  Then we replace top_id's original spot with id.

                for (uint8_t index_to_clobber = FAVORITE_COUNT-1; index_to_clobber>top_index; index_to_clobber--) {
                    // index_to_clobber starts at the max value for fav_badges_ids
                    // and goes through top_index+1. (index_to_clobber will never be 0).
                    if (fav_badges_ids[index_to_clobber-1] == id) {
                        // If we would be clobbering something by DEMOTING the old, stale entry for the
                        // badge we're logging into this list now, just skip it.
                    } else {
                        fav_badges_ids[index_to_clobber] = fav_badges_ids[index_to_clobber-1];
                        strcpy(fav_badges_handles[index_to_clobber], fav_badges_handles[index_to_clobber-1]);
                    }
                }

                // now clobber top_index with out new one.
                fav_badges_ids[top_index] = id;
                strcpy(fav_badges_handles[top_index], handle); // handle has already been sanitized.

                break; // We only care about the highest ranking that we qualify for.
            }
        }
    }
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
    strcpy(out_payload.handle, my_conf.handle);
}

void init() {
    Grace_init(); // Activate Grace-generated configuration

    check_conf();

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

    tlc_set_fun();

    // If we detected no errors, we're done here.
    if (!(led_error || crystal_error))
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

    if (led_error) {
        GrStringDraw(&g_sContext, "Err: LED!", -1, 0, print_loc, 0);
        print_loc += 12;
    }

    GrFlush(&g_sContext);

    delay(5000);
}

// Play a cute animation when we first turn the badge on.
void intro() {
    GrImageDraw(&g_sContext, &fingerprint_1BPP_UNCOMP, 0, 0);
    GrStringDrawCentered(&g_sContext, "Queercon", -1, 31, 94 + SYS_FONT_HEIGHT/3, 0);
    GrStringDrawCentered(&g_sContext, "twelve", -1, 31, 94 + SYS_FONT_HEIGHT/3 + SYS_FONT_HEIGHT, 0);
    GrStringDrawCentered(&g_sContext, "- 2015 -", -1, 31, 94 + SYS_FONT_HEIGHT/3 + SYS_FONT_HEIGHT*2, 0);
    GrFlush(&g_sContext);
}

void radio_send_beacon(uint8_t address, uint8_t beacons) {
    out_payload.beacon = beacons;
    out_payload.flag_id = 0;
    out_payload.friendship = 0;
    out_payload.from_addr = my_conf.badge_id;
    out_payload.to_addr = address;
    out_payload.play_id = 0;
    radio_send_sync();
}

void radio_send_befriend(uint8_t mode) {
    out_payload.beacon = 0;
    out_payload.flag_id = 0;
    out_payload.friendship = mode;
    out_payload.from_addr = my_conf.badge_id;
    out_payload.play_id = 0;
    if (befriend_mode > 1) {
        out_payload.to_addr = befriend_candidate;
    } else {
        out_payload.to_addr = RFM_BROADCAST;
    }
    radio_send_sync();
}

void radio_send_flag(uint8_t flag) {
    out_payload.beacon = 0;
    out_payload.flag_id = flag;
    out_payload.friendship = 0;
    out_payload.from_addr = my_conf.badge_id;
    out_payload.to_addr = RFM_BROADCAST;
    out_payload.play_id = 0;
    radio_send_sync();
}

void radio_send_play(uint8_t index) {
    out_payload.beacon = 0;
    out_payload.flag_id = 0;
    out_payload.friendship = 0;
    out_payload.play_id = BIT7 | index;
    out_payload.from_addr = my_conf.badge_id;
    out_payload.to_addr = RFM_BROADCAST;
    radio_send_sync();
}

void radio_send_puppy(uint8_t neighbor_index) {
    static uint8_t neighbor_addr;
    for (neighbor_addr=0; neighbor_index && (neighbor_addr<BADGES_IN_SYSTEM); neighbor_addr++) {
        if (neighbor_badges[neighbor_addr])
            neighbor_index--;
    }
    neighbor_addr--;

    out_payload.beacon = 0;
    out_payload.flag_id = 0;
    out_payload.friendship = 0;
    out_payload.base_id = 1; // puppy base.
    out_payload.play_id = 0;
    out_payload.from_addr = my_conf.badge_id;
    out_payload.to_addr = neighbor_addr;
    radio_send_sync();
    out_payload.base_id = NOT_A_BASE;
}

void set_befriend_failed() {
    oled_set_overhead_text("Okbai :(", 20);
    s_befriend_failed = 1;
    befriend_mode = 0;
    idle_mode_softkey_dis = 0;
    oled_draw_pane_and_flush(idle_mode_softkey_sel);
}

// Received_flag is ignored if from_radio is 0.
void befriend_proto_step(uint8_t from_radio, uint8_t received_flag, uint8_t from_id) {
    if (!befriend_mode || rfm_state != RFM_IDLE) {
        return;
    }

    if (from_radio) {
        // Flag 5 is the highest that ever gets sent. If we see
        //  something higher, someone's ruined something.
        //  Therefore we fail.
        if (received_flag > 5 || received_flag == 0) {
            set_befriend_failed();
            return;
        }

        // Here, we've received a radio message.
        // We expect it to be either:
        //  befriend_mode+1 (time to move to next state)
        //      or
        //  befriend_mode-1 (need to re-send this state)

        // First we'll want to validate the sender's ID.
        if (befriend_mode == 1) {
            // If we are a beaconing server, we use the incoming
            //  sender to set our befriend_candidate.
            befriend_candidate = from_id;
            if (received_flag == 1 && from_id > my_conf.badge_id) {
                befriend_mode = 2;
            }
        } else if (from_id != befriend_candidate) {
            // Otherwise, we need to ignore anything that's not
            //  from our befriend_candidate.
            return;
        }

        // Now let's handle the protocol machine:
        if (received_flag == befriend_mode-1) {
            // Older message received: we need to re-transmit our current mode.
            radio_send_befriend(befriend_mode);
        } else if (received_flag == befriend_mode+1) {
            // Next message received: we're ready to move along in the protocol.
            befriend_mode_ticks = BEFRIEND_RESEND_TRIES;
            befriend_mode += 2;

            // Two valid protocol states are TIME ONLY:
            //  BF_S_WAIT (where we think we've transmitted the last packet
            //              in the conversation but want to make sure that
            //              the client has received it)
            //      and
            // BF_C_WAIT (where we've transmitted the client's last packet in
            //              the conversation, received the server's last
            //              packet, and we're waiting a bit in order to try
            //              to synchronize our next actions with the server.)

            // So if we're in one of those states, don't send anything.
            if (befriend_mode != BF_S_WAIT && befriend_mode != BF_C_WAIT) {
                oled_set_overhead_text("Hello!", 75);
                radio_send_befriend(befriend_mode);
            }
        } else {
            return;
        }
        // End of radio handling section

    } else {

        // (we've already ruled out 0)
        if (befriend_mode < 5) { // the "normal" modes:
            // BEACON, OPEN
            // OPENING, and CLOSING.

            // In these we retransmit if we have ticks left,
            //  otherwise we time out.

            if (befriend_mode == BF_S_BEACON || befriend_mode == BF_C_OPENING) {
                // Modes 1 and 2's timeout is controlled outside of this
                //  function, so make sure it never times out here.
                //  This is kind of a kludge.
                befriend_mode_ticks = BEFRIEND_RESEND_TRIES;
            }

            if (befriend_mode_ticks) {
                // still some retries left.
                befriend_mode_ticks--;
                // Do re-send our message.
                oled_set_overhead_text("Hello?", 75);
                radio_send_befriend(befriend_mode);
            } else {
                set_befriend_failed();
            }
        } else {
            // CLOSE_WAIT and JUST_WAIT. (The waiting modes)
            // Here we just tick down and call things a success if we reach
            //  zero.
            if (befriend_mode_ticks) {
                befriend_mode_ticks--;
            } else {
                befriend_mode = 0;
                s_befriend_success = SIGNAL_BIT_OLED | SIGNAL_BIT_TLC;
                oled_set_overhead_text(befriend_candidate_handle, 180);
                set_badge_friend(befriend_candidate);
            }
        }
    } // end of time-step handling section.
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

        // BASE SERVICES:
        // Base services are a sort of overlay - The incoming payload is going to tell us
        //  either that it's NOT A BASE (in which case we skip), OR it's going to give us
        //  a valid base ID and be either a badge (from_addr < BADGES_IN_SYSTEM) or be an
        //  actual base station (from_addr == DEDICATED_BASE_ID).
        if (in_payload.base_id != NOT_A_BASE && (in_payload.from_addr < BADGES_IN_SYSTEM || in_payload.from_addr == DEDICATED_BASE_ID)) {
            if (my_conf.adult && in_payload.base_id == 1) // puppy
            {
                oled_set_overhead_image(&puppy, 254);
                achievement_get(ACH_HANDLER, 1);
            }
            else if (in_payload.base_id == BASE_SUITE) { // suite
                at_base = RECEIVE_WINDOW;
                at_suite_base = 1;
            } else if (in_payload.base_id < BASES_IN_SYSTEM) { // other bases
                at_base = RECEIVE_WINDOW;
                set_base_seen(in_payload.base_id - 2);
            }
        }

        // BEACON SERVICES:
        // "Beacon services" refers to BADGE TO BADGE COMMUNICATION.
        if (!disable_beacon_service && in_payload.from_addr < BADGES_IN_SYSTEM) {

            // It's a standard beacon:
            // Increment the badge count if needed:
            if (in_payload.beacon) {
                // It's a beacon (one per cycle).
                // Update the TTL of the neighbor_badges record for the
                //  source badge.
                neighbor_badges[in_payload.from_addr] = RECEIVE_WINDOW * in_payload.beacon;
                // Mark it seen:
                set_badge_seen(in_payload.from_addr);
                tick_badge_seen(in_payload.from_addr, in_payload.handle);
            }

            // Play schedule. We only care about this if we're in IDLE mode,
            //  and if we're not busy. (We use the enabledness of the softkey as
            //  a surrogate for that ill-defined "business").
            if (in_payload.play_id && op_mode == OP_MODE_IDLE && !idle_mode_softkey_dis && (in_payload.play_id & 0b01111111) <= play_anim_count && !play_mode) {
                if (my_conf.mood > MOOD_THRESH_SAD) {
                    play_id = in_payload.play_id & 0b01111111;
                    // Just stand there for the length of time the causing badge
                    //  is doing its cause animation.
                    if (my_conf.adult && play_id < play_anim_count) {
                        play_mode = PLAY_MODE_RECV;
                        oled_play_animation(&standing, play_cause[play_id]->len);
                    } else if (!my_conf.adult && play_id == play_anim_count) {
                        play_mode = PLAY_MODE_RECV;
                        oled_play_animation(&infant_standing, infant_play.len*3);
                    }
                    if (play_mode) {
                        idle_mode_softkey_dis = 1;
                        oled_draw_pane_and_flush(idle_mode_softkey_sel);
                        my_conf.play_margin--;
                        my_conf_write_crc();
                        if (my_conf.play_margin < -20) {
                            achievement_get(ACH_PUNKD, 1);
                            my_conf.play_margin = 0;
                        }
                        if (TLC_IS_A_GO) {
                            tlc_start_anim(&flag_pink, 0, 5*GLOBAL_TLC_SPEED_SCALE, 1, 0); // POW PINK!
                        }
                    }
                }
            }

            // It's a flag schedule:
            if (in_payload.flag_id && (in_payload.flag_id & 0b01111111) < FLAG_COUNT) {
                // Wave a flag.
                if (my_conf.adult && !my_conf.flag_unlocks) {
                    my_conf.flag_unlocks = 1;
                    softkey_en |= SK_BIT_FLAG | SK_BIT_SETFLAG;
                    mood_adjust_and_write_crc(MOOD_GOT_FLAG);
                }
                if (!flag_in_cooldown) {
                    mood_adjust_and_write_crc(MOOD_FLAG);
                    flag_id = in_payload.flag_id & 0b01111111;
                    tlc_start_anim(flags[in_payload.flag_id & 0b01111111], 0, 3*GLOBAL_TLC_SPEED_SCALE, 0, 0);
                    s_flag_wave = 1;
                    s_flag_send = 2;
                } // Otherwise, ignore it.
            }

            // Resolve inbound or completed friendship requests:
            if (in_payload.friendship && my_conf.adult) {
                if (befriend_mode) {
                    // We're currently somewhere in the befriend process,
                    //  so the protocol function will handle managing
                    //  befriend_candidate for us.
                    if (in_payload.from_addr == befriend_candidate) {
                        strcpy(befriend_candidate_handle, in_payload.handle);
                    }
                    befriend_proto_step(1, in_payload.friendship, in_payload.from_addr);
                } else if (in_payload.friendship == BF_S_BEACON) {
                    // Only track beacons in this way.
                    befriend_candidate = in_payload.from_addr;
                    befriend_candidate_age = BEFRIEND_BCN_AGE_LOOPS;
                }
            }
        }

    } else if (f_rfm_rx_done) {
        // Ignore messages from our own ID, because what even is that anyway?
        f_rfm_rx_done = 0;
    }

    // Poll buttons and check whether we need to resend a befriend message:
    if (f_time_loop) {
//        poll_buttons();
        WDT_A_resetTimer(WDT_A_BASE); // pat pat pat
        if (my_conf.adult && befriend_mode && !befriend_mode_loops_to_tick &&
                rfm_state == RFM_IDLE) {
            befriend_proto_step(0, 0, BADGES_IN_SYSTEM);
            befriend_mode_loops_to_tick = BEFRIEND_LOOPS_TO_RESEND;
        } else if (my_conf.adult && befriend_mode && befriend_mode_loops_to_tick) {
            befriend_mode_loops_to_tick--;
        }

        if (befriend_candidate_age) {
            befriend_candidate_age--;
        }
    }

    if (f_new_second) {
        if (f_radio_fault) {
            f_radio_fault = 0;
            LED_PULSE(flag_red);
            init_radio();
        }

        if (befriend_mode == 1 || befriend_mode == 2) {
            if (befriend_mode_secs) {
                befriend_mode_secs--;
            } else {
                set_befriend_failed();
            }
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
            if (at_base) {
                at_base--;
                if (!at_base) {
                    at_suite_base = 0;
                }
            }
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
        if (my_conf.flag_cooldown) {
            my_conf.flag_cooldown--;
            my_conf_write_crc();
        }

        if (my_conf.mood <= MOOD_THRESH_SAD) {
            my_conf.sadtime++;
            my_conf_write_crc();
            if (my_conf.sadtime == 480) {
                achievement_get(ACH_MOODY, 1);
            }
        }

        mood_tick_minutes--;
        if (!mood_tick_minutes) {
            if (neighbor_count >= 5 || at_base) {
                // Happy time.
                mood_adjust_and_write_crc(MOOD_TICK_AMOUNT_UP);
            } else {
                // Boring time.
                mood_adjust_and_write_crc(MOOD_TICK_AMOUNT);
            }
            mood_tick_minutes = MOOD_TICK_MINUTES;

        }

        if (at_base && at_suite_base && my_conf.suite_minutes < 200) {
            my_conf.suite_minutes++;
        }
        if (my_conf.suite_minutes >= 200) {
            achievement_get(ACH_CHILL, 1);
        }

        // General timing stuff:
        my_conf.waketime++;
        if (my_conf.waketime > 1440) { // 24 hours
            achievement_get(ACH_TIRED, 1);
        }
        if (my_conf.uptime) { // Might be OK to let this roll over, but we won't.
            my_conf.uptime++;
        }
        if (am_puppy && !(my_conf.uptime % 60)) {
            // Send it a few times:
            s_send_puppy = 4;
            puppy_target = 1 + rand() % neighbor_count;
        }

        // Figure out if it's time to grow up.
        if (!my_conf.adult && (my_conf.uptime + (my_conf.badge_id % 15) > 80)) {
            // Time to grow up!
            my_conf.time_to_hatch = 1;
            my_conf_write_crc();
            softkey_en |= SK_BIT_HATCH;
            idle_mode_softkey_sel = SK_SEL_HATCH;
            tlc_start_anim(&flag_rainbow, 0, 50, 1, 1);
        }


        my_conf_write_crc();
    }

    if (s_flag_send && rfm_state == RFM_IDLE) {
        s_flag_send--;
        flag_in_cooldown = FLAG_IN_COOLDOWN_SECONDS;
        radio_send_flag(flag_id | BIT7);
    }

    if (s_send_play && rfm_state == RFM_IDLE) {
        s_send_play--;
        radio_send_play(play_id);
    }

    if (s_send_puppy && rfm_state == RFM_IDLE) {
        s_send_puppy--;
        radio_send_puppy(puppy_target);
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

void handle_led_actions() {
    if (f_time_loop) {

    }

    if (s_new_checkin && TLC_IS_A_GO) {
        s_new_checkin = 0;
        tlc_start_anim(&flag_rainbow, 0, 5*GLOBAL_TLC_SPEED_SCALE, 1, 4);
    }

    if (s_befriend_failed) {
        s_befriend_failed = 0;
        LED_PULSE(flag_red);
    }

    if (s_befriend_success & SIGNAL_BIT_TLC) {
        s_befriend_success &= ~SIGNAL_BIT_TLC;
        if (s_new_uber_friend | SIGNAL_BIT_TLC ||( s_new_friend | SIGNAL_BIT_TLC)) {
            tlc_start_anim(&flag_rainbow, 0, 3, 1, 4);
            s_new_uber_friend &= ~SIGNAL_BIT_TLC;
            s_new_friend &= ~SIGNAL_BIT_TLC;
        } else {
            LED_PULSE(flag_green);
        }
    }

    if ((s_new_uber_seen & SIGNAL_BIT_TLC || s_new_badge_seen & SIGNAL_BIT_TLC) && TLC_IS_A_GO) {
        // Big rainbow animation.
        tlc_start_anim(&flag_rainbow, 0, 4*GLOBAL_TLC_SPEED_SCALE, 1, 4);
        s_new_badge_seen &= ~SIGNAL_BIT_TLC;
        s_new_uber_seen &= ~SIGNAL_BIT_TLC;
    }

    if (f_tlc_anim_done) {
        f_tlc_anim_done = 0;
        if (s_flag_wave) {
            tlc_start_anim(flags[my_conf.flag_id], 0, 3*GLOBAL_TLC_SPEED_SCALE, 0, 3);
            s_flag_wave = 0;
        } else if (befriend_mode) {
            START_BEFRIEND_TLC_ANIM;
        } else if (!my_conf.adult && my_conf.time_to_hatch){
            tlc_start_anim(&flag_rainbow, 0, 50, 1, 1);
        } else {
            tlc_display_ambient();
        }
    }
}

void handle_character_actions() {
    if (f_time_loop) {
        oled_timestep();

    }

    if (f_new_second) {
        // Determine whether we should do a random action.
        if (idle_action_seconds && oled_anim_state == OLED_ANIM_DONE) {
            if (my_conf.adult && my_conf.mood > MOOD_THRESH_SAD && (idle_action_seconds & BIT0)) {
                oled_play_animation(&standing, (idle_action_seconds & BIT1) >> 1);
            }
            idle_action_seconds--;
        }
        if (!idle_action_seconds) {
            s_need_idle_action = 1;
            if (my_conf.adult) {
                idle_action_seconds = 2 + rand() % 7;
            }
            else {
                idle_action_seconds = 1 + rand() % 3;
            }
        } else {
        }
    }

    if (s_befriend_success & SIGNAL_BIT_OLED) {
        s_befriend_success &= ~SIGNAL_BIT_OLED;
        if (s_new_friend & SIGNAL_BIT_OLED || s_new_uber_friend & SIGNAL_BIT_OLED) {
            s_new_friend &= ~SIGNAL_BIT_OLED;
            s_new_uber_friend &= ~SIGNAL_BIT_OLED;
            if (my_conf.adult)
                oled_play_animation(&flap_arms, 10);
        } else {
            if (my_conf.adult)
                oled_play_animation(&wave_right, 10);
        }
    }

    if (s_oled_needs_redrawn_idle) {
        s_oled_needs_redrawn_idle = 0;
        if (my_conf.adult && my_conf.time_to_hatch) {
            my_conf.time_to_hatch = 0;
            idle_mode_softkey_dis = 0;
            my_conf_write_crc();
        }

        if (play_mode == PLAY_MODE_RECV || play_mode == PLAY_MODE_CAUSE_ALONE) {
            // We just finished standing still for receiving a play.
            //  (or causing something to happen to ourself)
            // Time to act!
            play_mode = PLAY_MODE_EFFECT;
            if (my_conf.adult) {
                oled_play_animation(play_effect[play_id], 0);
            } else {
                oled_play_animation(&infant_play, 3);
            }
            // This displeases us:
            mood_adjust_and_write_crc(MOOD_PLAY_RECV);
        } else if (play_mode == PLAY_MODE_EFFECT || play_mode == PLAY_MODE_OBSERVE) {
            // We just finished receiving a play.
            //  (or watching one)
            // Time to go back to normal.
            play_mode = 0;
            idle_mode_softkey_dis = 0;
            oled_draw_pane_and_flush(idle_mode_softkey_sel);
        } else if (play_mode == PLAY_MODE_CAUSE) {
            // Just finished causing a play. Time to stand back
            //  and observe.
            play_mode = PLAY_MODE_OBSERVE;
            if (my_conf.adult) {
                oled_play_animation(play_observe[play_id], 0);
            } else {
                oled_play_animation(&infant_standing, infant_play.len * 3);
            }
            oled_draw_pane_and_flush(idle_mode_softkey_sel);
        }

        if (my_conf.adult && !play_mode && my_conf.mood < MOOD_THRESH_SAD) {
            oled_anim_disp_frame(&bored_standing, 0);
        } else if (my_conf.adult && !play_mode) {
            oled_anim_disp_frame(&standing, 0);
        } else if (!play_mode) { // child's play just ended (lol)
            oled_anim_disp_frame(&infant_standing, 0);
        }
    }

    // Bail if we're not going to be able to start an animation, because that's
    // all that the rest of this function does.
    if (oled_anim_state != OLED_ANIM_DONE) {
        return;
    }

    if (my_conf.adult && (s_new_uber_seen & SIGNAL_BIT_OLED || s_new_badge_seen & SIGNAL_BIT_OLED)) {
        // Wave!
        oled_play_animation(&wave_right, 4);
        s_new_badge_seen &= ~SIGNAL_BIT_OLED;
        s_new_uber_seen &= ~SIGNAL_BIT_OLED;
    }

    if (s_need_idle_action) {
        s_need_idle_action = 0;
        // Pick a random idle animation to do.
        static uint8_t anim;

        if (my_conf.adult) {
            if (my_conf.mood > MOOD_THRESH_SAD) {
                anim = rand() % idle_anim_count;
                oled_play_animation(idle_anims[anim], rand() % 5);
            } else {
                anim = rand() % moody_idle_anim_count;
                oled_play_animation(moody_idle_anims[anim], rand() % 5);
            }
        } else {
            anim = rand() % infant_idle_anim_count;
            oled_play_animation(infant_idle_anims[anim], 4+rand() % 5);
        }
    }

}

void try_to_sleep() {
    // If there are no more flags left to service, go to sleep.
    if (!(f_bl || f_br || f_bs || f_new_second || f_rfm_rx_done || f_rfm_tx_done || f_time_loop)) {
        __bis_SR_register(SLEEP_BITS);
    }
}

// Read the badgeholder's name if appropriate:
void handle_mode_name() {
    // Clear the screen and display the instructions.
    static tRectangle name_erase_rect = {0, NAME_Y_OFFSET, 63, NAME_Y_OFFSET + NAME_FONT_HEIGHT + SYS_FONT_HEIGHT};
    disable_beacon_service = 1;
    static uint8_t update_disp;
    update_disp = 1;
    static uint8_t cheat_success;
    static uint8_t cheat_fail_cnt=0;

    cheat_success = 0;
    GrClearDisplay(&g_sContext);
    GrContextFontSet(&g_sContext, &SYS_FONT);
    oled_print(0, 5, "Enter a name.", 1, 0);
    oled_print(0, 3+SYS_FONT_HEIGHT*3, "Hold middle button to finish.", 1, 0);
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
    // For cheat mode:
    uint8_t bl_down_loops = 0;
    uint8_t cheat_mode = 0;
    text_width = GrStringWidthGet(&g_sContext, name, last_char_index+1);

    while (1) {
        handle_infrastructure_services();
        handle_led_actions();

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
                bl_down_loops = 0;
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
            } else if (f_bl == BUTTON_PRESS) {
                f_bl = 0;
                bl_down_loops = 1;
            }

            // If we're counting the number of loops for which the softkey is
            // depressed, go ahead and increment it. This is going to do one
            // double-count at the beginning, but I don't care.
            if (bs_down_loops && bs_down_loops < NAME_COMMIT_LOOPS) {
                bs_down_loops++;
            } else if (bs_down_loops) {
                break;
            }

            // Check to see if we're ready to enter CHEAT MODE.
            if (bl_down_loops && bl_down_loops < NAME_COMMIT_LOOPS) {
                bl_down_loops++;
            } else if (my_conf.handle[0] && !cheat_mode && bl_down_loops) {
                bl_down_loops = 0;
                tlc_start_anim(&flag_ally, 2, 2*GLOBAL_TLC_SPEED_SCALE, 1, 4);
                GrClearDisplay(&g_sContext);
                GrContextFontSet(&g_sContext, &SYS_FONT);
                oled_print(0, 5, "Enter a cheat code, you wascally wabbit.", 1, 0);
                GrFlush(&g_sContext);
                cheat_mode = 1;
                update_disp = 1;
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

    static uint8_t need_puppy;
    need_puppy = 0;

    if (cheat_mode) {
        if (!strcmp(name, CHEAT_FLAG_NC)) {
            cheat_success = 1;
            my_conf.flag_unlocks = 0xFF;
            my_conf_write_crc();
            check_conf();
        } else if (!strcmp(name, CHEAT_FLAG)) {
            cheat_success = 1;
            my_conf.flag_unlocks = 0x01;
            my_conf_write_crc();
            check_conf();
        } else if (!strcmp(name, CHEAT_TITLE)) {
            cheat_success = 1;
            my_conf.titles_unlocked = 1;
            my_conf_write_crc();
        } else if (!strcmp(name, CHEAT_HAPPY)) {
            cheat_success = 1;
            mood_adjust_and_write_crc(100);
        } else if (!strcmp(name, CHEAT_SAD)) {
            cheat_success = 1;
            mood_adjust_and_write_crc(-100);
        } else if (!strcmp(name, CHEAT_INFANT)) {
            cheat_success = 1;
            my_conf.adult = 0;
            my_conf_write_crc();
            check_conf();
        } else if (!strcmp(name, CHEAT_INVERT)) {
            cheat_success = 1;
            qc12_oledInit(1);
            tlc_stop_anim(0);
        } else if (!strcmp(name, CHEAT_UNINVERT)) {
            cheat_success = 1;
            qc12_oledInit(0);
            tlc_stop_anim(0);
        } else if (!strcmp(name, CHEAT_PUPPY)) {
            cheat_success = 1;
            need_puppy = 1;
        } else if (!strcmp(name, CHEAT_PUPPYOFF)) {
            cheat_success = 1;
            am_puppy = 0;
        } else if (!strcmp(name, CHEAT_MIRROR)) {
            cheat_success = 1;
            qc12oled_WriteCommand(0xC0);
        } else if (!strcmp(name, CHEAT_UNMIRROR)) {
            cheat_success = 1;
            qc12oled_WriteCommand(0xC8);
        }

        if (cheat_success) {
            cheat_success = 0;
            cheat_fail_cnt = 0;
            LED_PULSE(flag_green);
            achievement_get(ACH_CHEATER, 1);
            if (need_puppy) {
                am_puppy = 1;
                need_puppy = 0;
                achievement_get(ACH_PUPPY, 1);
            }
        }
        else {
            LED_PULSE(flag_red);
            cheat_fail_cnt++;
            if (cheat_fail_cnt == 5) {
                achievement_get(ACH_TOWEL, 1);
            }
        }


    } else {
        strcpy(my_conf.handle, name);
        my_conf_write_crc();
        strcpy(out_payload.handle, name);
    }

    achievement_get(ACH_BABY, 1);

    op_mode = OP_MODE_IDLE;
    suppress_softkey = 1; // And don't register the button release

    GrClearDisplay(&g_sContext);
    GrFlush(&g_sContext);
    disable_beacon_service = 0;
} // handle_mode_name

uint8_t softkey_enabled(uint8_t index) {
    if ((index == SK_SEL_FLAG || index == SK_SEL_SETFLAG) && (!my_conf.flag_unlocks || !my_conf.adult)) {
        return 0;
    }

    if (index == SK_SEL_FLAG) {
        return !my_conf.flag_cooldown;
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
        handle_led_actions();
        handle_character_actions();

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
                // Select button
                switch (idle_mode_softkey_sel) {
                case SK_SEL_ASL:
                    op_mode = OP_MODE_ASL;
                    break;
                case SK_SEL_SETFLAG:
                    op_mode = OP_MODE_SETFLAG;
                    break;
                case SK_SEL_NAME:
                    op_mode = OP_MODE_NAME;
                    break;
                case SK_SEL_PLAY:
                    if (neighbor_count && rand() % 3) {
                        // If I see other people, 2-in-3 chance of group play.
                        s_send_play = 1;
                        play_mode = PLAY_MODE_CAUSE;
                        tlc_start_anim(&flag_pink, 0, 5*GLOBAL_TLC_SPEED_SCALE, 1, 0); // POW PINK!
                        my_conf.play_margin++;
                        my_conf_write_crc();
                        if (my_conf.play_margin > 20) {
                            achievement_get(ACH_TEASE, 1);
                            my_conf.play_margin = 0;
                        }
                    } else {
                        play_mode = PLAY_MODE_CAUSE_ALONE;
                        tlc_start_anim(&flag_yellow, 0, 5*GLOBAL_TLC_SPEED_SCALE, 1, 0); // BRRP YELLOW!
                    }
                    if (my_conf.adult) {
                        play_id = rand() % play_anim_count;
                        oled_play_animation(play_cause[play_id], 0);
                    } else {
                        play_id = play_anim_count;
                        oled_play_animation(&infant_play, 3);
                    }
                    idle_mode_softkey_dis = 1;
                    oled_draw_pane_and_flush(idle_mode_softkey_sel);
                    mood_adjust_and_write_crc(MOOD_PLAY_SEND);
                    break;
                case SK_SEL_FLAG:
                    if (!my_conf.seen_flags) {
                        my_conf.seen_flags = 1;
                        my_conf_write_crc();
                    }
                    tlc_start_anim(flags[my_conf.flag_id], 0, 3*GLOBAL_TLC_SPEED_SCALE, 0, 5);
                    if (my_conf.flag_unlocks != 0xFF) {
                        my_conf.flag_cooldown = FLAG_OUT_COOLDOWN_MINUTES;
                        my_conf_write_crc();
                    }
                    flag_id = my_conf.flag_id;
                    s_flag_send = FLAG_SEND_TRIES;
                    idle_mode_softkey_sel = 0;
                    s_new_pane = 1;
                    break;
                case SK_SEL_HATCH:
                    // Grow up!
                    tlc_start_anim(&flag_ally, 2, 10, 1, 15);
                    oled_set_overhead_off();
                    achievement_get(ACH_NEWBIE, 0);
                    oled_play_animation(&infant_grow, 15);
                    my_conf.adult = 1;
                    idle_mode_softkey_dis = 1;
                    softkey_en &= ~SK_BIT_HATCH;
                    softkey_en |= SK_BIT_SLEEP | SK_BIT_FRIEND;
                    idle_mode_softkey_sel = SK_SEL_PLAY;
                    my_conf_write_crc();
                    break;
                case SK_SEL_FRIEND:
                    if (!my_conf.seen_befriend) {
                        my_conf.seen_befriend = 1;
                        my_conf_write_crc();
                    }
                    idle_mode_softkey_dis = 1;
                    oled_draw_pane_and_flush(idle_mode_softkey_sel);
                    befriend_mode_loops_to_tick = BEFRIEND_LOOPS_TO_RESEND;
                    befriend_mode_secs = BEFRIEND_TIMEOUT_SECONDS;
                    befriend_mode_ticks = BEFRIEND_RESEND_TRIES;

                    START_BEFRIEND_TLC_ANIM;

                    if (befriend_candidate_age) { // We have a valid candidate:
                        // This will be from a beacon, so we'll be the client:
                        befriend_mode = 2;
                        befriend_proto_step(0, 0, BADGES_IN_SYSTEM);
                    } else { // We haven't seen any beacons,
                        // so we'll be the server.
                        befriend_mode = 1;
                        befriend_proto_step(0, 0, BADGES_IN_SYSTEM);
                    }
                    break;
                case SK_SEL_SLEEP:
                    if (!my_conf.seen_sleep) {
                        my_conf.seen_sleep = 1;
                        my_conf_write_crc();
                    }
                    op_mode = OP_MODE_SLEEP;
                    break;
                default:
                    __never_executed();
                }
            }
            f_bs = 0;
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

void asl_draw_page(uint8_t page) {
    static uint8_t achievement_id;
    char buf[16] = "";
    tRectangle friends_rect = {5, 50, 59, 60};
    tRectangle uber_rect = {5, 93, 59, 103};
    switch (page) {
    case 0:
        GrContextFontSet(&g_sContext, &SYS_FONT);
        GrStringDrawCentered(&g_sContext, "I've seen:", -1, 32, 8, 0);

        GrStringDraw(&g_sContext, "overall", -1, 10, 23, 0);

        // Badges seen:
        sprintf(buf, "%d / %d", my_conf.seen_count, BADGES_IN_SYSTEM);
        GrRectDraw(&g_sContext, &friends_rect);
        GrStringDraw(&g_sContext, buf, -1, 10, 35, 0);
        friends_rect.sXMax = 5 + 54 * my_conf.seen_count / BADGES_IN_SYSTEM;

        // Ubers seen:
        GrStringDraw(&g_sContext, "uber", -1, 10, 65, 0);
        sprintf(buf, "%d / %d", my_conf.uber_seen_count, UBERS_IN_SYSTEM);
        GrRectDraw(&g_sContext, &uber_rect);
        GrStringDraw(&g_sContext, buf, -1, 10, 77, 0);
        uber_rect.sXMax = 5 + 54 * my_conf.uber_seen_count / UBERS_IN_SYSTEM;

        // Fill both meters:
        friends_rect.sYMax--;
        uber_rect.sYMax--;
        GrRectFill(&g_sContext, &uber_rect);
        GrRectFill(&g_sContext, &friends_rect);
        uber_rect.sYMax++;
        friends_rect.sYMax++;

        break;
    case 1:
        GrContextFontSet(&g_sContext, &SYS_FONT);
        GrStringDrawCentered(&g_sContext, "Friends:", -1, 32, 8, 0);

        GrStringDraw(&g_sContext, "overall", -1, 10, 23, 0);

        // Badges friended:
        sprintf(buf, "%d / %d", my_conf.friend_count, BADGES_IN_SYSTEM);
        GrRectDraw(&g_sContext, &friends_rect);
        GrStringDraw(&g_sContext, buf, -1, 10, 35, 0);
        friends_rect.sXMax = 5 + 54 * my_conf.friend_count / BADGES_IN_SYSTEM;


        // Ubers seen:
        GrStringDraw(&g_sContext, "uber", -1, 10, 65, 0);
        sprintf(buf, "%d / %d", my_conf.uber_friend_count, UBERS_IN_SYSTEM);
        GrRectDraw(&g_sContext, &uber_rect);
        GrStringDraw(&g_sContext, buf, -1, 10, 77, 0);
        uber_rect.sXMax = 5 + 54 * my_conf.uber_friend_count / UBERS_IN_SYSTEM;

        // Fill both meters:
        friends_rect.sYMax--;
        uber_rect.sYMax--;
        GrRectFill(&g_sContext, &uber_rect);
        GrRectFill(&g_sContext, &friends_rect);
        friends_rect.sYMax++;
        uber_rect.sYMax++;
        break;
    case 2:
        // Top friends
        GrContextFontSet(&g_sContext, &SYS_FONT);

        GrStringDraw(&g_sContext, "#1", -1, 3, 20, 0);
        if (fav_badges_handles[0][0]) {
            if (badges_seen[fav_badges_ids[0]] & BADGE_SEX_BIT) {
                GrImageDraw(&g_sContext, &bed, 25, 11);
            } else if (badges_seen[fav_badges_ids[0]] & BADGE_FRIEND_BIT) {
                GrImageDraw(&g_sContext, &heart, 20, 15);
            }
        }

        GrStringDraw(&g_sContext, "#2", -1, 3, 51, 0);
        if (fav_badges_handles[1][0]) {
            if (badges_seen[fav_badges_ids[1]] & BADGE_SEX_BIT) {
                GrImageDraw(&g_sContext, &bed, 25, 42);
            } else if (badges_seen[fav_badges_ids[1]] & BADGE_FRIEND_BIT) {
                GrImageDraw(&g_sContext, &heart, 20, 46);
            }
        }

        GrStringDraw(&g_sContext, "#3", -1, 3, 82, 0);
        if (fav_badges_handles[2][0]) {
            if (badges_seen[fav_badges_ids[2]] & BADGE_SEX_BIT) {
                GrImageDraw(&g_sContext, &bed, 25, 73);
            } else if ((badges_seen[fav_badges_ids[2]] & BADGE_FRIEND_BIT)) {
                GrImageDraw(&g_sContext, &heart, 20, 77);
            }
        }
        GrStringDrawCentered(&g_sContext, "My faves:", -1, 32, 8, 1);
        GrStringDraw(&g_sContext, fav_badges_handles[0], -1, 3, 35, 1);
        GrStringDraw(&g_sContext, fav_badges_handles[1], -1, 3, 66, 1);
        GrStringDraw(&g_sContext, fav_badges_handles[2], -1, 3, 97, 1);

        break;
    case 3:
        // I see
        GrStringDrawCentered(&g_sContext, "I can see", -1, 32, 8, 0);

        // Badges visible:
        sprintf(buf, "%d", neighbor_count);
        GrStringDrawCentered(&g_sContext, buf, -1, 32, 20, 0);
        GrStringDrawCentered(&g_sContext, "qcbadges", neighbor_count == 1 ? 7 : 8, 32, 32, 0);

        // Mood:
        GrStringDrawCentered(&g_sContext, "My mood:", -1, 32, 68, 0);
        if (my_conf.mood > MOOD_THRESH_HAPPY) {
            GrStringDrawCentered(&g_sContext, "Happy!", -1, 32, 83, 0);
        } else if (my_conf.mood < MOOD_THRESH_SAD) {
            GrStringDrawCentered(&g_sContext, "Sad!", -1, 32, 83, 0);
        } else {
            GrStringDrawCentered(&g_sContext, "OK.", -1, 32, 83, 0);
        }
        GrRectDraw(&g_sContext, &uber_rect);
        uber_rect.sXMax = 5 + 54 * my_conf.mood / 100;

        // Fill both meters:
        uber_rect.sYMax--;
        GrRectFill(&g_sContext, &uber_rect);
        uber_rect.sYMax++;
        break;
    default:
        // Achievement listing.
        if (my_conf.adult && !my_conf.seen_titles) {
            my_conf.seen_titles = 1;
            my_conf_write_crc();
        }

        achievement_id = page-4;
        oled_print(0,0, "Title:", 1, 0);
        oled_print(0,11, titles[achievement_id], 1, 1);
        GrLineDrawH(&g_sContext, 5, 58, 24);
        if (achievement_have(achievement_id)) {
            oled_print(0,25, "Unlocked!", 0, 1);
            GrImageDraw(&g_sContext, &check, 16, 40);
        } else {
            oled_print(0,25, "LOCKED", 0, 1);
            GrImageDraw(&g_sContext, &nocheck, 16, 40);
        }
        oled_print(0,62, title_descs[page-4], 0, 1);

        break;

    }

}

void handle_mode_asl() {
    // Radio does background stuff but character actions ignored.
    // TLC continues to animate.

    static uint8_t page_num;
    static uint8_t timeout_seconds;
    timeout_seconds = 75;
    page_num = 0;
    uint8_t s_redraw = 1;
    uint8_t asl_pages = 4;
    if (my_conf.adult) {
        asl_pages += NUM_ACHIEVEMENTS;
    }

    GrClearDisplay(&g_sContext);
    GrFlush(&g_sContext);

    while (1) {
        handle_infrastructure_services();
        handle_led_actions();

        if (f_new_second) {
            f_new_second = 0;

            if (timeout_seconds) {
                timeout_seconds--;
            } else {
                op_mode = OP_MODE_IDLE;
                break;
            }

        }

        if (f_time_loop) {
            f_time_loop = 0;
            if (f_br == BUTTON_PRESS) {
                page_num = (page_num+1) % (asl_pages);
                s_redraw = 1;
            }
            f_br = 0;

            if (f_bl == BUTTON_PRESS) {
                page_num = (page_num+(asl_pages-1)) % (asl_pages);
                s_redraw = 1;
            }
            f_bl = 0;

            if (f_bs == BUTTON_RELEASE) {
                f_bs = 0;

                if (my_conf.titles_unlocked && page_num >= 4 && achievement_have(page_num-4)) {
                    // This is your new title.
                    my_conf.title_index = page_num-4;
                    my_conf_write_crc();
                }

                // Select button
                op_mode = OP_MODE_IDLE;
                break;
            }
            f_bs = 0;
        }

        if (s_redraw) {
            // softkey or something changed:
            s_redraw = 0;
            timeout_seconds = 75;

            GrClearDisplay(&g_sContext);
            GrContextFontSet(&g_sContext, &SOFTKEY_LABEL_FONT);

            if (my_conf.titles_unlocked && page_num >= 4 && achievement_have(page_num-4)) {
                // This is your new title.
                GrStringDrawCentered(&g_sContext, "Pick", -1, 32, 127 - SOFTKEY_FONT_HEIGHT/2, 1);
            } else {
                GrStringDrawCentered(&g_sContext, "Close", -1, 32, 127 - SOFTKEY_FONT_HEIGHT/2, 1);
            }

            if (my_conf.adult && !my_conf.seen_titles) {
                GrImageDraw(&g_sContext, &idea, 45, 109); // To mark a NEW IDEA!
            }

            // Arrows:
            GrLineDraw(&g_sContext, 4, 112, 0, 116);
            GrLineDraw(&g_sContext, 4, 120, 0, 116);
            GrLineDraw(&g_sContext, 59, 112, 63, 116);
            GrLineDraw(&g_sContext, 59, 120, 63, 116);
            GrLineDrawH(&g_sContext, 0, 64, 116);

            asl_draw_page(page_num);

            GrFlush(&g_sContext);
        }

        try_to_sleep();

    }
    f_bs = 0;

    // Cleanup:
    // (clear all the character stuff)

}

void handle_mode_sleep() {
    // Clear the screen.
    GrClearDisplay(&g_sContext);
    GrFlush(&g_sContext);

    // Let our faves know we're going to sleep (so they remember us).
    // A beacon normally has a staying power of about 60*10 seconds (1min)
    // Let's give nearby badges a 10-minute grace period to sleep with us
    // as well. Sex can be one-way in this system, which is, in a somewhat
    // poetic sense, quite realistic.
    for (uint8_t t=4; t; t--) { // Do this a few times to try to get through.
        for (uint8_t i=0; i<FAVORITE_COUNT; i++) {
            if (badge_seen_ticks[fav_badges_ids[i]]) {
                radio_send_beacon(fav_badges_ids[i], 10);
                while (rfm_state != RFM_IDLE);
                WDT_A_resetTimer(WDT_A_BASE);
                delay(50);
            }
        }
    }

    // Wait for the radio to quiesce.
    while (rfm_state != RFM_IDLE)
        __no_operation();

    // Sleep the radio.
    mode_sync(RFM_MODE_SL); // Going to sleep... mode...
    write_single_register(0x3b, RFM_AUTOMODE_OFF);
    f_rfm_rx_done = f_rfm_tx_done = 0;

    my_conf.waketime = 0;
    my_conf.sleeptime = 0;
    my_conf_write_crc();


    // Kill the LEDs.
    tlc_stop_anim(1);

    uint8_t sexy = 0;
    for (uint8_t id=0; id<FAVORITE_COUNT; id++) {
        if (neighbor_badges[fav_badges_ids[id]] && (badges_seen[fav_badges_ids[id]] & BADGE_FRIEND_BIT)) {
            sexy = 1;
            my_conf.seen_sleep = 2;
            my_conf_write_crc();
            break;
        }
    }

    // Set up our Zzz animation.
    static uint8_t zzz_index = 0;

    while (1) {
        if (f_time_loop) {
            f_time_loop = 0;
//            poll_buttons();
            WDT_A_resetTimer(WDT_A_BASE); // pat pat pat

            if (f_bs == BUTTON_RELEASE) {
                f_bs = 0;
                break;
            }
            f_bs = f_br = f_bl = 0;

        }
        if (f_new_second) {
            f_new_second = 0;
            GrClearDisplay(&g_sContext);
            GrStringDraw(&g_sContext, "Zzz...", zzz_index, 15, 72, 1);

            if (sexy && ((~zzz_index & BIT0) || my_conf.sleeptime > 30)) {
                GrImageDraw(&g_sContext, &bed, 20, 90);
            }

            GrFlush(&g_sContext);
            zzz_index = (zzz_index+1) % 7;

            minute_secs--;
            if (!minute_secs) {
                minute_secs = 60;
                s_new_minute = 1;
            }

            if (s_new_minute) {
                s_new_minute = 0;
                if (my_conf.flag_cooldown) {
                    my_conf.flag_cooldown--;
                }

                my_conf.sleeptime++;

                if (my_conf.uptime) {
                    my_conf.uptime++;
                }
                mood_adjust_and_write_crc(MOOD_TICK_AMOUNT_UP);
            }
        }

        try_to_sleep(); // This one needs to be different...
    }

    if (my_conf.sleeptime > 480) {
        achievement_get(ACH_SLEEPY, 0);
    }

    if (my_conf.sleeptime > 30) {
        // If we were asleep for more than 30 minutes,
        // check to see if any of our favorites were around us when we
        // fell asleep. If so, we "slept with them."
        for (uint8_t id=0; id<FAVORITE_COUNT; id++) {
            if (neighbor_badges[fav_badges_ids[id]] && (badges_seen[fav_badges_ids[id]] & BADGE_FRIEND_BIT)) {
                badges_seen[fav_badges_ids[id]] |= BADGE_SEX_BIT;
                achievement_get(ACH_SEXY, 1);
                mood_adjust_and_write_crc(100);
            }
        }
    }

    my_conf.sleeptime = 0;
    mood_adjust_and_write_crc(10);

    init_radio();
    op_mode = OP_MODE_IDLE;
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
        handle_led_actions();


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
                op_mode = OP_MODE_IDLE;
                tlc_start_anim(flags[softkey_sel], 0, 1*GLOBAL_TLC_SPEED_SCALE, 0, 0);
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
    delay(1000);
    post();

    GrClearDisplay(&g_sContext);

    if (!my_conf.handle[0] || s_default_conf_loaded) { // Name is not set:
        op_mode = OP_MODE_NAME;
        s_default_conf_loaded = 0;
    }

    WDT_A_initWatchdogTimer(WDT_A_BASE, WDT_A_CLOCKSOURCE_ACLK, WDT_A_CLOCKDIVIDER_32K);
//    WDTCTL = WDTPW + WDTCNTCL + WDTHOLD + WDT_A_CLOCKSOURCE_ACLK, WDT_A_CLOCKDIVIDER_32K;
    WDT_A_start(WDT_A_BASE);

    while (1) {
        switch(op_mode) {
        case OP_MODE_IDLE:
            handle_mode_idle();
            break;
        case OP_MODE_NAME:
            handle_mode_name(); // Learn the badge's name (if we don't have it already)
            break;
        case OP_MODE_ASL:
            handle_mode_asl();
            break;
        case OP_MODE_SETFLAG:
            handle_mode_setflag();
            break;
        case OP_MODE_SLEEP:
            handle_mode_sleep();
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

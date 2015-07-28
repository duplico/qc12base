/*
 * qcxi.h
 *
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

#ifndef QC12_H_
#define QC12_H_

#include <stdint.h>
#include <driverlib/MSP430FR5xx_6xx/driverlib.h>
#include <msp430fr5949.h>
#include <grlib.h>

// CONFIGURATION SETTINGS ///////////////////////////////////////////

// System configuration
#define BADGES_IN_SYSTEM 175
#define UBERS_IN_SYSTEM 12
#define SLEEP_BITS LPM1_bits // because we need SMCLK for the TLC.
#define NUM_ACHIEVEMENTS 32
#define FAVORITE_COUNT 3
#define RADIO_TX_POWER_DB 4 // [-18 .. 13]
#define FLAG_OUT_COOLDOWN_MINUTES 10
#define FLAG_IN_COOLDOWN_SECONDS 10
#define FLAG_SEND_TRIES 3
#define MOOD_TICK_MINUTES 10
#define MOOD_TICK_AMOUNT -3
#define BEFRIEND_TIMEOUT_SECONDS 3
#define BEFRIEND_RESEND_TRIES 10
#define BEFRIEND_LOOPS_TO_RESEND 10

#define RECEIVE_WINDOW 8
#define RECEIVE_WINDOW_LENGTH_SECONDS 10

// Name entry configuration parameters:
#define NAME_SEL_CHAR '*'
#define NAME_MAX_LEN 14
// this will break everything if it's 1 or possibly 2:
#define NAME_COMMIT_LOOPS 80

// Character name & title font:
#define NAME_FONT_HEIGHT 14
#define NAME_FONT g_sFontCmss12b

// Main system font:
#define SYS_FONT g_sFontCmss12b
#define SYS_FONT_HEIGHT 12

// Softkey label font:
#define SOFTKEY_LABEL_FONT SYS_FONT
#define SOFTKEY_FONT_HEIGHT 12 // not 12 because we're using sm. caps only.

//////////////////////////
// Derived definitions ///

#define NAME_Y_OFFSET 10+SYS_FONT_HEIGHT*7 // Top of the name entry field.
#define SPRITE_Y 128 - 64 - SOFTKEY_FONT_HEIGHT - 3 // Top of the char sprite.
#define RFM_TX_POWER (((uint8_t)(18 + RADIO_TX_POWER_DB)) & 0b00011111)

#define TLC_IS_A_GO (tlc_anim_mode == TLC_ANIM_MODE_IDLE || tlc_is_ambient)

/////////////////////////////////////////////////////////////////////
// Hardware related defines /////////////////////////////////////////

// LED controller:
#define TLC_LATPORT     GPIO_PORT_P1
#define TLC_LATPIN      GPIO_PIN4

// Radio:
#define RFM_NSS_PORT_OUT P1OUT
#define RFM_NSS_PIN      GPIO_PIN3

/////////////////////////////////////////////////////////////////////
// State constants //////////////////////////////////////////////////

// Button events:
#define BUTTON_PRESS 1
#define BUTTON_RELEASE 2

// LED sending types:
#define TLC_SEND_IDLE     0
#define TLC_SEND_TYPE_GS  1
#define TLC_SEND_TYPE_FUN 2
#define TLC_SEND_TYPE_LB  3
// LED animation states:
#define TLC_ANIM_MODE_IDLE  0
#define TLC_ANIM_MODE_SHIFT 1
#define TLC_ANIM_MODE_SAME  2

// Overall operating modes:
#define OP_MODE_IDLE 0
#define OP_MODE_NAME 2
#define OP_MODE_ASL 4
#define OP_MODE_SLEEP 6
#define OP_MODE_SETFLAG 8
#define OP_MODE_BEFRIEND 10
#define OP_MODE_MAX OP_MODE_BEFRIEND

// Softkey options:
#define SK_SEL_PLAY 0
#define SK_SEL_ASL 1
#define SK_SEL_FRIEND 2
#define SK_SEL_FLAG 3
#define SK_SEL_SETFLAG 4
#define SK_SEL_RPS 5
#define SK_SEL_NAME 6
#define SK_SEL_SLEEP 7
#define SK_SEL_MAX 7

extern const char sk_labels[][10];
extern uint16_t softkey_en;

// Badge count tracking:
#define BADGE_SEEN_BIT BIT0
#define BADGE_FRIEND_BIT BIT1

// Radio bidness:
#define NOT_A_BASE 0xFF

// Masks for managing flags/signals that are consumed by both the
// OLED and the TLC systems:
#define SIGNAL_BIT_TLC BIT0
#define SIGNAL_BIT_OLED BIT1

//////////////////////////////////////////////////////////////////////
// Functions etc. ////////////////////////////////////////////////////

#define GPIO_pulse(port, pin) do { GPIO_setOutputHighOnPin(port, pin); GPIO_setOutputLowOnPin(port, pin); } while (0)
#define CEILING_DIV(x,y) (((x) + (y) - 1) / (y))

// The delay function, which we don't really want to use much, please.
void delay(unsigned int);

// Interrupt flags:
extern volatile uint8_t f_time_loop;
extern volatile uint8_t f_rfm_rx_done;
extern volatile uint8_t f_rfm_tx_done;
extern volatile uint8_t f_tlc_anim_done;

extern uint8_t s_oled_anim_finished;

typedef struct {
    uint8_t to_addr, from_addr, base_id;
    uint8_t beacon;
    uint8_t friendship;
    uint8_t flag_from;
    uint8_t flag_id;
    uint8_t play_id;
    char handle[NAME_MAX_LEN];
} qc12payload;

extern qc12payload in_payload, out_payload;

extern const char titles[][10];

typedef struct {
    uint8_t badge_id;
    uint8_t mood;
    uint8_t title_index;
    uint8_t flag_id;
    uint8_t flag_cooldown;
    uint16_t exp;
    uint8_t seen_count;
    uint8_t uber_seen_count;
    uint8_t friend_count;
    uint8_t uber_friend_count;
    uint8_t achievements[CEILING_DIV(NUM_ACHIEVEMENTS, 8)];
    uint8_t top_seen[3];
    char top_seen_handles[3][NAME_MAX_LEN];
    char handle[NAME_MAX_LEN+1];
    uint16_t crc16;
} qc12conf;

extern qc12conf my_conf;
extern const qc12conf default_conf;

extern uint8_t idle_mode_softkey_sel;
extern uint8_t op_mode;

extern uint8_t neighbor_count;

#endif /* QC12_H_ */

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

void usci_a_send(uint16_t base, uint8_t data);







// Configuration flags
#define BADGE_TARGET 1
#define DEBUG_SERIAL 0
#define BADGES_IN_SYSTEM 160
#define RECEIVE_WINDOW 8
#define RECEIVE_WINDOW_LENGTH_SECONDS 10
#define BUS_BASE_ID 0xEE
#define TRICK_INTERVAL_SECONDS 6
#define TRICK_COUNT 12

// Configuration of pins for the badge and launchpad
#if BADGE_TARGET
	// Target is the actual badge:
	#include <msp430fr5949.h>

	// Radio:
	// TODO
	#define RFM_NSS_PORT GPIO_PORT_P1
	#define RFM_NSS_PORT_DIR P1DIR
	#define RFM_NSS_PORT_OUT P1OUT
	#define RFM_NSS_PIN GPIO_PIN3

#else
	// Target is the Launchpad+shield:
	#include <msp430f5329.h>

	// Radio:
	#define RFM_NSS_PORT GPIO_PORT_P3
	#define RFM_NSS_PORT_DIR P3DIR
	#define RFM_NSS_PORT_OUT P3OUT
	#define RFM_NSS_PIN GPIO_PIN7

#endif

// Useful defines:
#define BIT15 0x8000
#define DATA_IF(val) if (val) P1OUT |= BIT5; else P1OUT &= ~BIT5;
#define GPIO_pulse(port, pin) do { GPIO_setOutputHighOnPin(port, pin); GPIO_setOutputLowOnPin(port, pin); } while (0)

// The delay function, which we don't really want to use much, please.
void delay(unsigned int);

extern volatile uint8_t f_time_loop;
extern volatile uint8_t f_rfm_rx_done;
extern volatile uint8_t f_rfm_tx_done;

typedef struct {
	uint8_t to_addr, from_addr, base_id, clock_authority;
	uint8_t prop_from;
	uint8_t prop_id;
	uint16_t prop_time_loops_before_start;
	uint8_t beacon;
} qcxipayload;

extern qcxipayload in_payload, out_payload;

// Extra features for the Launchpad version - serial and LED chains:
#if !BADGE_TARGET
	// Debug serial
	extern volatile char ser_buffer_rx[255];
	extern volatile char ser_buffer_tx[255];
	extern volatile uint8_t f_ser_rx;
	void ser_print(char*);
	void ser_init();
	// WS2812 LEDs:
	extern uint8_t ws_frameBuffer[];
#endif

#endif /* QC12_H_ */

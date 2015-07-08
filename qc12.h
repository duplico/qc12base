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

// Configuration flags
#define BADGE_TARGET 1
#define BADGES_IN_SYSTEM 200
#define SLEEP_BITS LPM1_bits // because we need SMCLK for the TLC.

// Configuration of pins

// Radio:
#define RFM_NSS_PORT_OUT P1OUT
#define RFM_NSS_PIN GPIO_PIN3

// Button events:
#define BUTTON_PRESS 1
#define BUTTON_RELEASE 2

// Useful defines:
#define GPIO_pulse(port, pin) do { GPIO_setOutputHighOnPin(port, pin); GPIO_setOutputLowOnPin(port, pin); } while (0)

// The delay function, which we don't really want to use much, please.
void delay(unsigned int);

extern volatile uint8_t f_time_loop;
extern volatile uint8_t f_rfm_rx_done;
extern volatile uint8_t f_rfm_tx_done;

typedef struct {
	uint8_t to_addr, from_addr, base_id;
	uint8_t beacon;
} qc12payload;

extern qc12payload in_payload, out_payload;

#endif /* QC12_H_ */

/*
 * radio.h
 *
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <stdint.h>

#define RFM_FIFO 	0x00
#define RFM_OPMODE 	0x01

#define RFM_MODE_RX 0b00010000
#define RFM_MODE_SB 0b00000100
#define RFM_MODE_TX 0b00001100
#define RFM_MODE_SL 0b00000000

#define RFM_IRQ1 0x27
#define RFM_IRQ2 0x28

#define RFM_BROADCAST 0xff

extern volatile uint8_t rfm_state;

#define RFM_IDLE 0
#define RFM_BUSY 1

#define RFM_AUTOMODE_RX 0b01100101
#define RFM_AUTOMODE_TX 0b01011011
#define RFM_AUTOMODE_OFF 0b00000000

void init_radio();

void write_single_register(uint8_t, uint8_t);
uint8_t read_single_register_sync(uint8_t);
void mode_sync(uint8_t mode);

void radio_send_sync();
void radio_send_half_async();
void radio_send_async();

void radio_recv();

uint8_t rfm_crcok();

#endif /* RADIO_H_ */

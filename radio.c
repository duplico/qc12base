/*
 * radio.c
 *
 * (c) 2014 George Louthan
 * 3-clause BSD license; see license.md.
 */

/*
 *  Radio (RFM69CW)
 *        (MSB first, clock inactive low,
 *         write on rise, change on fall, MSB first)
 *        eUSCI_B0 - radio
 *        somi, miso, clk, ste
 *        DIO0      P3.1
 *        RESET     P3.2
 */

#include "radio.h"
#include "qc12.h"
#include "leds.h"

#include <stdint.h>
#include <driverlib/MSP430FR5xx_6xx/driverlib.h>
#include <string.h>

#define SPICLK 8000000

uint8_t returnValue = 0;

volatile uint8_t rfm_state = RFM_IDLE;

qc12payload in_payload;
qc12payload out_payload;

// temp buffer:
uint8_t in_bytes[sizeof(in_payload)];

#define RESET_PORT	GPIO_PORT_P3
#define RESET_PIN	GPIO_PIN2

#define DIO_PORT	GPIO_PORT_P3
#define DIO_PIN		GPIO_PIN1

void delay(unsigned int ms) {
    while (ms--)
        __delay_cycles(8000);
}

void init_radio() {

    // SPI for radio is done in Grace.

    volatile uint8_t rfm_reg_tx_index = 0;
    volatile uint8_t rfm_reg_rx_index = 0;
    volatile uint8_t rfm_begin = 0;
    volatile uint8_t rfm_rw_reading = 0; // 0- read, 1- write
    volatile uint8_t rfm_rw_single = 0; // 0- single, 1- fifo
    volatile uint8_t rfm_single_msg = 0;

    volatile uint8_t rfm_reg_ifgs = 0;
    volatile uint8_t rfm_reg_state = RFM_IDLE;

    // Radio reboot procedure:
    //  hold RESET high for > 100 us
    //  pull RESET low, wait 5 ms
    //  module is ready

    GPIO_setOutputHighOnPin(RESET_PORT, RESET_PIN);
    delay(10);
    GPIO_setOutputLowOnPin(RESET_PORT, RESET_PIN);
    delay(10);

    EUSCI_B_SPI_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    EUSCI_B_SPI_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT);

    rfm_reg_state = RFM_IDLE;
    mode_sync(RFM_MODE_SB);

    // One weird thing about this radio module. The datasheet lists
    // "default (recommended)" values for the config registers. For all but
    // 8 registers, these are truly default; however, for those 8, the
    // "default" values and "reset (built-in)" values (the latter of which I
    // would describe as "default," and the former, "recommended," but what
    // do I know?) are different. Those are:
    //   Addr Name           Desc
    //   0x18 RegLna         LNA settings
    //   0x19 RegRxBw        Channel filter bandwidth control
    //   0x8b RegAfcBw       Channel filter bandwidth control during AFC
    //   0x26 RegDioMapping2 Mapping DIO5, ClkOut frequency
    //   0x29 RegRssiThresh  RSSI Threshold control
    // 0x2f-
    //  -0x36 RegSyncValue   Sync Word bytes 1-8
    //   0x3c RegFifoThresh  Fifo threshold, Tx start condition
    //   0x6f RegTestDagc    Fading Margin Improvement

    // init radio to recommended "defaults" (seriously, wtf are they
    //  calling them defaults for if they're not set BY DEFAULT?????
    //  Sheesh.), per datasheet:
    write_single_register(0x18, 0b00001000); // Low-noise amplifier (Def:0x08, rec:0x88)
    write_single_register(0x19, 0b01010101); // Bandwidth control "default"
    write_single_register(0x1a, 0x8b); // Auto Frequency Correction BW "default"
    write_single_register(0x26, 0x07); // Disable ClkOut
    write_single_register(0x29, 190); // RSSI Threshold (228 recommended; 210 from qcxi)

    // Other configuration:

    /// Output configuration:
    write_single_register(0x11, 0b10000000 | RFM_TX_POWER); // PA0On and Output power
//	write_single_register(0x12, 0b00001111); // PA0 ramp time

    write_single_register(0x25, 0b00000000); // GPIO map to default

    for (uint8_t sync_addr = 0x2f; sync_addr <= 0x36; sync_addr++) {
        write_single_register(sync_addr, "QuCo12XII"[(uint8_t) (sync_addr%9)]);
    }

    // Preamble size:
    write_single_register(0x2d, 0x0F);

    // Setup addresses and length:
    write_single_register(0x37, 0b00110100); // Packet configuration (see DS)
    write_single_register(0x38, sizeof(qc12payload)); // PayloadLength
    write_single_register(0x39, my_conf.badge_id); // NodeAddress
    write_single_register(0x3A, RFM_BROADCAST); // BroadcastAddress

    write_single_register(0x3c, 0x80 | (sizeof(qc12payload) - 1)); // TxStartCondition - FifoNotEmpty

    // Now that we're done with this setup business, we can enable the
    // DIO interrupts. We have to wait until now because otherwise if
    // the is radio activity during setup it will enter our protocol
    // state machine way too early, which can cause the system to hang
    // indefinitely.

    // Auto packet mode: RX->SB->RX on receive.
    mode_sync(RFM_MODE_RX);
    write_single_register(0x3b, RFM_AUTOMODE_RX);
    volatile uint8_t ret = 0;

    GPIO_selectInterruptEdge(DIO_PORT, DIO_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_clearInterrupt(DIO_PORT, DIO_PIN);
    GPIO_enableInterrupt(DIO_PORT, DIO_PIN);

    ret = read_single_register_sync(0x01);

}

uint8_t radio_barrier_with_timeout() {
    uint32_t spin = 800000;
    while (rfm_state != RFM_IDLE) {
        spin--;
        if (!spin) {
            f_radio_fault = 1;
            return 1;
        }
    }
    return 0;
}

void usci_b0_send_sync(uint8_t data) {
    EUSCI_B_SPI_transmitData(EUSCI_B0_BASE, data);
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE,
              EUSCI_B_SPI_TRANSMIT_INTERRUPT));
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE,
            EUSCI_B_SPI_RECEIVE_INTERRUPT));
    EUSCI_A_SPI_receiveData(EUSCI_B0_BASE); // Throw away the stale garbage we got while sending.
}

uint8_t usci_b0_recv_sync(uint8_t data) {
    EUSCI_B_SPI_transmitData(EUSCI_B0_BASE, data);
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE,
              EUSCI_B_SPI_TRANSMIT_INTERRUPT));
    while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE,
            EUSCI_B_SPI_RECEIVE_INTERRUPT));
    return EUSCI_B_SPI_receiveData(EUSCI_B0_BASE);
}

void write_single_register(uint8_t addr, uint8_t data) {
    /*
     * This blocks.
     */
    if (radio_barrier_with_timeout()) return; // Block until ready to write.

    RFM_NSS_PORT_OUT &= ~RFM_NSS_PIN;
    usci_b0_send_sync(addr | BIT7);
    usci_b0_send_sync(data);
    RFM_NSS_PORT_OUT |= RFM_NSS_PIN;
}

uint8_t read_single_register_sync(uint8_t addr) {
    uint8_t rfm_single_msg = 0;
    if (radio_barrier_with_timeout()) return 0; // Block until ready to write.

    addr = 0b01111111 & addr; // MSB=0 => write command
    // Hold NSS low to begin frame.
    RFM_NSS_PORT_OUT &= ~RFM_NSS_PIN;
    usci_b0_send_sync(addr);
    rfm_single_msg = usci_b0_recv_sync(0xff);
    // Hold NSS high to end frame.
    RFM_NSS_PORT_OUT |= RFM_NSS_PIN;

    return rfm_single_msg;
}

void mode_sync(uint8_t mode) {
    write_single_register(RFM_OPMODE, mode);
    uint8_t reg_read;
    uint16_t spin = 65535;
    do {
        spin--;
        write_single_register(RFM_OPMODE, mode);
        if (!spin) {
            init_radio();
            return; // This is potentially going to put us in a very weird state.
            // Happily this isn't a failure mode we really see.
        }
        reg_read = read_single_register_sync(RFM_IRQ1);
    } while (!(BIT7 & reg_read));
}

uint8_t expected_dio_interrupt = 0;

void radio_send_sync() {
    // Wait for, e.g., completion of receiving something.
    if (radio_barrier_with_timeout()) return;
    mode_sync(RFM_MODE_SB);

    // Compute our CRC.
    CRC_setSeed(CRC_BASE, 0x0C12);
    for (uint8_t i = 0; i < sizeof(qc12payload) - 2; i++) {
        CRC_set8BitData(CRC_BASE, ((uint8_t *) &out_payload)[i]);
    }
    out_payload.crc = CRC_getResult(CRC_BASE);

    // Intermediate mode is TX
    // Enter condition is FIFO level
    // Exit condition is PacketSent.
    // Then we'll go back to SB. Once the transmission completes, we'll go
    // to the normal automode.
    write_single_register(0x3b, RFM_AUTOMODE_TX);

    expected_dio_interrupt = 1; // will be xmit finished.
    rfm_state = RFM_BUSY;

    RFM_NSS_PORT_OUT &= ~RFM_NSS_PIN;
    usci_b0_send_sync(RFM_FIFO | BIT7); // FIFO write command

    for (uint8_t rfm_reg_tx_index=0; rfm_reg_tx_index<sizeof(qc12payload); rfm_reg_tx_index++) {
        usci_b0_send_sync(((uint8_t *) &out_payload)[rfm_reg_tx_index]);
    }
    RFM_NSS_PORT_OUT |= RFM_NSS_PIN;

}

void radio_recv() {
    // Grab the data out of the FIFO. Side effects city here!
    if (radio_barrier_with_timeout()) return;
    rfm_state = RFM_BUSY;

    // Hold NSS low to begin frame.
    RFM_NSS_PORT_OUT &= ~RFM_NSS_PIN;
    usci_b0_send_sync(RFM_FIFO); // FIFO read command
    for (uint8_t rfm_reg_rx_index=0; rfm_reg_rx_index<sizeof(qc12payload); rfm_reg_rx_index++) {
        in_bytes[rfm_reg_rx_index] = usci_b0_recv_sync(0xFF);
    }
    RFM_NSS_PORT_OUT |= RFM_NSS_PIN;

    // Ideally FifoNotEmpty would be asserted here...
    // Ain't nobody got time for that.
    memcpy(&in_payload, in_bytes, sizeof(qc12payload));
    rfm_state = RFM_IDLE;

    // Compute our CRC.
    CRC_setSeed(CRC_BASE, 0x0C12);
    for (uint8_t i = 0; i < sizeof(qc12payload) - 2; i++) {
        CRC_set8BitData(CRC_BASE, ((uint8_t *) &in_payload)[i]);
    }
    if (in_payload.crc != CRC_getResult(CRC_BASE)) {
        // CRC fail.
        in_payload.from_addr = BADGES_IN_SYSTEM;
        in_payload.base_id = NOT_A_BASE;
        f_rfm_rx_done = 0;
    }
}

/*
 * ISR for the SPI interface to the radio.
 *
 * We either just sent or just received something.
 * Here's how this goes down.
 *
 * (NB: all bets are off in the debugger: the order of RXIFG and TXIFG tend
 *      to reverse when stepping through line by line. Doh.)
 *
 * First RXIFG is always ignored
 * First TXIFG is always the command
 *
 * We can either be reading/writing a single register, in which case:
 *
 *    If READ:
 *    	RXIFG: Second byte goes into rfm_single_msg
 *    	TXIFG: Second byte is 0
 *
 * 	  If WRITE:
 * 	  	RXIFG: Second byte is ignored
 * 	  	TXIFG: Second byte is rfm_single_msg
 *
 * Or we can be reading/writing the FIFO, in which case:
 *
 *    If READ:
 *    	Until index==len:
 *    		RXIFG: put the read message into rfm_fifo
 *    		TXIFG: send 0
 *    If WRITE:
 *    	Until index==len:
 *    		RXIFG: ignore
 *    		TXIFG: send the message from rfm_fifo
 *
 *
 */
#pragma vector=USCI_B0_VECTOR
__interrupt void EUSCI_B0_ISR(void) {
    switch (__even_in_range(UCB0IV, 4)) {
    //Vector 2 - RXIFG
    case 2:
        break;
    case 4: // Vector 4 - TXIFG : I just sent a byte.
        break;
    default:
        break;
    } // End of ISR flag switch ////////////////////////////////////////////////////
}

/*
 * ISR for DIO0 from the RFM module. It's asserted when a job (TX or RX) is finished.
 */
#pragma vector=PORT3_VECTOR
__interrupt void radio_interrupt_0(void) {
    switch (P3IV) {
    case BIT2:
        if (expected_dio_interrupt) { // tx finished.
            // The automode logic will have put us back into SB mode at this
            // point. So raise our interrupt flag for tx_done...
            f_rfm_tx_done = 1;
            expected_dio_interrupt = 0;
            rfm_state = RFM_IDLE;
            // NB: We will still need to return to our normal receive automode:
            // RX->SB->RX on receive.
            // OUTSIDE AN ISR we need: write_single_register(0x3b, RFM_AUTOMODE_RX);
            // and then: write_single_register_async(RFM_OPMODE, RFM_MODE_RX);
        } else { // rx
            f_rfm_rx_done = 1;
        }
        __bic_SR_register_on_exit(LPM3_bits);
        break;

    }
}

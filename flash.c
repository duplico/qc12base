/*
 * flash.c
 *
 *  Created on: Jun 25, 2015
 *      Author: George
 */

void init_flash() {

	// TESTING THE FLASH:

	// Flash:
	// CS#          P1.1
	// HOLD#   P1.0
	// WP#          P3.0

	// CS# high normally
	// HOLD# high normally
	// WP# high normally (write protect)

	// Read status register:

	//    P1OUT &= ~BIT1; // CS low, select.
	//    usci_a_send(EUSCI_A0_BASE, 0x05); // Read status.
	//    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
	//    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
	//    EUSCI_A_SPI_receiveData(EUSCI_A0_BASE); // Throw away the stale garbage we got while sending.
	//
	//    usci_a_send(EUSCI_A0_BASE, 0xFF); // Meaningless crap.
	//    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
	//    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
	//    in = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);
	//
	//    P1OUT |= BIT1; // CS high, deselect.
	//
	//    sprintf(str, "status = %d", in);
	//    GrStringDraw(&g_sContext, str, -1, 0, 0, 1);
	//    GrFlush(&g_sContext);
	//
	//    // WRITE ENABLE:
	//    P1OUT &= ~BIT1; // CS low, select.
	//    usci_a_send(EUSCI_A0_BASE, 0x06); // WREN
	//    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
	//    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
	//    EUSCI_A_SPI_receiveData(EUSCI_A0_BASE); // Throw away the stale garbage we got while sending.
	//    P1OUT |= BIT1; // CS high, deselect.
	//
	//    // Read status register:
	//
	//    P1OUT &= ~BIT1; // CS low, select.
	//    usci_a_send(EUSCI_A0_BASE, 0x05); // Read status.
	//    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
	//    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
	//    EUSCI_A_SPI_receiveData(EUSCI_A0_BASE); // Throw away the stale garbage we got while sending.
	//
	//    usci_a_send(EUSCI_A0_BASE, 0xFF); // Meaningless crap.
	//    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE,
	//    		EUSCI_A_SPI_RECEIVE_INTERRUPT));
	//    in = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);
	//
	//    P1OUT |= BIT1; // CS high, deselect.
	//
	//    sprintf(str, "status = %d", in);
	//    GrStringDraw(&g_sContext, str, -1, 0, 10, 1);
	//    GrFlush(&g_sContext);

}

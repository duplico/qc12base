/*
 *  This file is automatically generated and does not require a license
 *
 *  ==== WARNING: CHANGES TO THIS GENERATED FILE WILL BE OVERWRITTEN ====
 *
 *  To make changes to the generated code, use the space between existing
 *      "USER CODE START (section: <name>)"
 *  and
 *      "USER CODE END (section: <name>)"
 *  comments, where <name> is a single word identifying the section.
 *  Only these sections will be preserved.
 *
 *  Do not move these sections within this file or change the START and
 *  END comments in any way.
 *  ==== ALL OTHER CHANGES WILL BE OVERWRITTEN WHEN IT IS REGENERATED ====
 *
 *  This file was generated from
 *      C:/ti/grace_3_10_00_82/packages/ti/mcu/msp430/csl2/communication/EUSCI_init.xdt
 */
#include <stdint.h>
#include "_Grace.h"
#include <driverlib/MSP430FR5xx_6xx/inc/hw_memmap.h>
#include <driverlib/MSP430FR5xx_6xx/eusci_a_spi.h>

/* USER CODE START (section: EUSCI_A0_init_c_prologue) */
/* User defined includes, defines, global variables and functions */
/* USER CODE END (section: EUSCI_A0_init_c_prologue) */

/*
 *  ======== EUSCI_A0_graceInit ========
 *  Initialize Config for the MSP430 eUSCI_A0
 */
void EUSCI_A0_graceInit(void)
{
    /* Struct to pass to EUSCI_A_SPI_initMaster */
    EUSCI_A_SPI_initMasterParam initSPIMasterParam = {0};

    /* USER CODE START (section: EUSCI_A0_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: EUSCI_A0_graceInit_prologue) */

    /* initialize eUSCI SPI master mode */
    initSPIMasterParam.selectClockSource = EUSCI_A_SPI_CLOCKSOURCE_SMCLK;
    initSPIMasterParam.clockSourceFrequency = 4000000;
    initSPIMasterParam.desiredSpiClock = 500000;
    initSPIMasterParam.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    initSPIMasterParam.clockPhase = EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    initSPIMasterParam.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    initSPIMasterParam.spiMode = EUSCI_A_SPI_3PIN;
    EUSCI_A_SPI_initMaster(EUSCI_A0_BASE, &initSPIMasterParam);

    /* enable eUSCI SPI */
    EUSCI_A_SPI_enable(EUSCI_A0_BASE);

    /* disable eUSCI SPI transmit interrupt */
    EUSCI_A_SPI_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);

    /* disable eUSCI SPI receive interrupt */
    EUSCI_A_SPI_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);

    /* USER CODE START (section: EUSCI_A0_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: EUSCI_A0_graceInit_epilogue) */

}

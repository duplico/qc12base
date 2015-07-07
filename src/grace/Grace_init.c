/*
 *  ======== Grace_init.c ========
 *  DO NOT MODIFY THIS FILE - CHANGES WILL BE OVERWRITTEN
 */

#include <msp430.h>
#include "_Grace.h"

/* verify that the MSP430 headers included support the code that's generated */
#if defined(__TI_COMPILER_VERSION__)
  /* pragma required to suppress TI warning that #warning is unrecognized */
  #pragma diag_suppress 11
#endif
#if __MSP430_HEADER_VERSION__ < 1121
  #if defined(__TI_COMPILER_VERSION__)
    #warn The MSP430 headers included may be incompatible with the generated source files.  If the value of __MSP430_HEADER_VERSION__, declared by msp430.h, is less than 1121, please update your version of the msp430 headers.
  #elif defined(__GNUC__) || defined(__IAR_SYSTEMS_ICC__)
    #warning The MSP430 headers included may be incompatible with the generated source files.  If the value of __MSP430_HEADER_VERSION__, declared by msp430.h, is less than 1121, please update your version of the msp430 headers.
  #else
    /* if we can't just warn, resort to ANSI C's #error */
    #error The MSP430 headers included may be incompatible with the generated source files.  If the value of __MSP430_HEADER_VERSION__, declared by msp430.h, is less than 1121, please update your version of the msp430 headers.
  #endif
#endif
#if defined(__TI_COMPILER_VERSION__)
  /* pragma required to restore TI warnings about unrecognized directives */
  #pragma diag_default 11
#endif

/*
 *  ======== Grace_init =========
 *  Initialize all configured Grace peripherals
 */
void Grace_init(void)
{
    /* Stop watchdog timer from timing out during initial start-up. */
    WDTCTL = WDTPW | WDTHOLD;

    /* initialize Config for the MSP430 A3 Timer 1 */
    Timer1_A3_graceInit();

    /* initialize Config for the MSP430 pin mux */
    PinMux_graceInit();

    /* initialize Config for the MSP430 FR57xx family clock systems (CS_A) */
    CS_A_graceInit();

    /* initialize Config for the MSP430 eUSCI_A0 */
    EUSCI_A0_graceInit();

    /* initialize Config for the MSP430 EUSCI_A1 */
    EUSCI_A1_graceInit();

    /* initialize Config for the MSP430 EUSCI_B0 */
    EUSCI_B0_graceInit();

    /* initialize Config for the MSP430 PMM (Power Management Module) */
    PMM_graceInit();

    /* initialize Interrupt vector support */
    InterruptVectors_graceInit();

    /* initialize Config for the MSP430 System Registers */
    System_graceInit();

    /* initialize Config for the MSP430 RTC_B */
    RTC_B_graceInit();

    /* initialize Config for the MSP430 WDT_A */
    WDT_A_graceInit();
}

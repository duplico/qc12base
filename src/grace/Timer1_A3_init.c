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
 *      C:/ti/grace_3_10_00_82/packages/ti/mcu/msp430/csl2/timer/ITimerx_A_init.xdt
 */
#include <driverlib/MSP430FR5xx_6xx/inc/hw_memmap.h>
#include <driverlib/MSP430FR5xx_6xx/timer_a.h>
#include "_Grace.h"

/* USER CODE START (section: Timer1_A3_init_c_prologue) */
/* User defined includes, defines, global variables and functions */
/* USER CODE END (section: Timer1_A3_init_c_prologue) */

/*
 *  ======== Timer1_A3_graceInit ========
 *  Initialize Config for the MSP430 A3 Timer 1
 */
void Timer1_A3_graceInit(void)
{
    /* Struct to pass to Timer_A_initUpMode */
    Timer_A_initUpModeParam initUpParam = {0};

    /* Struct to pass to Timer_A_initCompareMode */
    Timer_A_initCompareModeParam initCompParam = {0};

    /* USER CODE START (section: Timer1_A3_graceInit_prologue) */
    /* User initialization code */
    /* USER CODE END (section: Timer1_A3_graceInit_prologue) */

    /* Initialize TimerA in up mode */
    initUpParam.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    initUpParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpParam.timerPeriod = 2;
    initUpParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    initUpParam.timerClear = TIMER_A_SKIP_CLEAR;
    initUpParam.startTimer = false;
    Timer_A_initUpMode(TIMER_A1_BASE, &initUpParam);

    /* Initialize Compare Mode for Capture/Compare Register 1 */
    initCompParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    initCompParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initCompParam.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    initCompParam.compareValue = 1;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initCompParam);

    /* Start TimerA counter */
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    /* USER CODE START (section: Timer1_A3_graceInit_epilogue) */
    /* User code */
    /* USER CODE END (section: Timer1_A3_graceInit_epilogue) */
}

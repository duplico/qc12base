#include <msp430.h>
#include "_Grace.h"

/* USER CODE START (section: InterruptVectors_init_c_prologue) */
/* User defined includes, defines, global variables and functions */
/* USER CODE END (section: InterruptVectors_init_c_prologue) */

/*
 *  ======== InterruptVectors_graceInit ========
 */
void InterruptVectors_graceInit(void)
{
}


/*
 *  ======== Timer1_A3 Interrupt Service Routine ======== 
 */
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR_HOOK(void)
{
    /* USER CODE START (section: TIMER1_A0_ISR_HOOK) */
    /* replace this comment with your code */
    /* USER CODE END (section: TIMER1_A0_ISR_HOOK) */
}

/*
 *  ======== Timer1_A3 Interrupt Service Routine ======== 
 */
#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR_HOOK(void)
{
    /* USER CODE START (section: TIMER1_A1_ISR_HOOK) */
    /* replace this comment with your code */
    /* USER CODE END (section: TIMER1_A1_ISR_HOOK) */
}



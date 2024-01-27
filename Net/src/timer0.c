#include "timer0.h"
#include "CH57x_common.h"
#include "cc.h"

static uint32_t  time_10ms = 0;
 

/*************************************************************************
Timer0    10ms
*************************************************************************/
void InitTimer0(void)
{
  TMR0_TimerInit((10000*32));  //10ms 
  TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END); 
  NVIC_EnableIRQ(TMR0_IRQn);
}

uint32_t get_time_10ms(void)
{
    return time_10ms;
}

void TMR0_IRQHandler( void )        // TMR0
{
    TMR0_ClearITFlag( TMR0_3_IT_CYC_END );
    time_10ms++;
}

//Should return time in miliseconds
u32_t sys_now()
{
  	return time_10ms*10;
}

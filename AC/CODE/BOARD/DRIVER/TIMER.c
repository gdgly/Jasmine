#include 	<LPC177x_8x.H>
#include "Board.h"
#include <stdio.h>

static uint32_t TimerTick=0;


/****************************************************************************\
 Function:  TimerInit
 Descript:  定时器初始化  
 Input:	    定时器号，毫秒中断值
 Output:    无 
 Return:    0
 Author:    quanhouwei
 Datetime:  17-08-17
 *****************************************************************************/
void TimerInit(uint8_t no,uint16_t time)
{
	if (no == 0)
    {
    	LPC_SC->PCONP  |= (1 << 1);    //打开功率空客估值                                     

    	LPC_TIM0->TCR  = 0x02;         // 复位定时器 
    	LPC_TIM0->IR   = 1;
    	LPC_TIM0->CTCR = 0;
    	LPC_TIM0->TC   = 0;
    	LPC_TIM0->PR   = time-1;

    	LPC_TIM0->MR0  = PeripheralClock/1000;

    	LPC_TIM0->MCR  = 0x03;       // MRO与TC之匹配时将产生中断                                              

    	LPC_TIM0->TCR  = 0x01;       // 启动      

		NVIC_EnableIRQ(TIMER0_IRQn);                                        
    	NVIC_SetPriority(TIMER0_IRQn, 3);
	}

	if (no == 1)
	{
	}
 
}


void TIMER0_IRQHandler(void)
{
	LPC_TIM0->IR = 0x01;        // 清中断标志
	TimerTick++;
}

uint32_t TimerRead(void)
{
	return TimerTick;
}












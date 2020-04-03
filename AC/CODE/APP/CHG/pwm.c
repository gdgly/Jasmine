#include "pwm.h"
static uint8_t lasttim=0xff;
/************************************************************************************************************
** 函 数 名 : PwmInit
** 功能描述 : 输出PWM波
** 输    入 : tim =HIVCNT  输出高电平   tim =   PWMCNT   输出 PWM  波
** 输    出 : 无
** 返    回 : 返回 
*************************************************************************************************************
*/

void PwmInit(uint8_t tim)
{
	uint32_t	pwmrval;
	
	if(tim==lasttim) 
		return;
	lasttim=tim;
	LPC_SC->PCONP |= (0x01 << 6);                     /* 打开PWM模块功率控制位       */  
	LPC_IOCON->P1_24  &= ~0x07;										    /* 功能选择                    */
	LPC_IOCON->P1_24  |= 2;                           /* P1.24作PWM1[5]输出           */
	pwmrval = PeripheralClock / 10000;	
	
	LPC_PWM1->PR  = 0x09;                          /* FPCLK_FREQ分频，PWM计数频率  */																								 /* 为FPCLK_FREQ/10000           */
	LPC_PWM1->MCR = 0x02;                          /* PWMMR0与PWMTC匹配时复位PWMTC */
	LPC_PWM1->PCR = (1 << 13);                     /* 使能PWM5输出                 */
	LPC_PWM1->MR0 = pwmrval;                       /* PWM速率控制/计数初值         */
	LPC_PWM1->MR5 = (pwmrval / ( 100 - HIVCNT )) * tim;      /* 设置边沿脉冲的脉宽和位置     */
	LPC_PWM1->LER = 0x7F;                          /* 锁存所有PWM匹配值            */
	LPC_PWM1->TCR = 0x02;                          /* 复位PWMTC                    */
	LPC_PWM1->TCR = 0x09;                          /* 使能PWM                      */
	
}




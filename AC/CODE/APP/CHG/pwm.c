#include "pwm.h"
static uint8_t lasttim=0xff;
/************************************************************************************************************
** �� �� �� : PwmInit
** �������� : ���PWM��
** ��    �� : tim =HIVCNT  ����ߵ�ƽ   tim =   PWMCNT   ��� PWM  ��
** ��    �� : ��
** ��    �� : ���� 
*************************************************************************************************************
*/

void PwmInit(uint8_t tim)
{
	uint32_t	pwmrval;
	
	if(tim==lasttim) 
		return;
	lasttim=tim;
	LPC_SC->PCONP |= (0x01 << 6);                     /* ��PWMģ�鹦�ʿ���λ       */  
	LPC_IOCON->P1_24  &= ~0x07;										    /* ����ѡ��                    */
	LPC_IOCON->P1_24  |= 2;                           /* P1.24��PWM1[5]���           */
	pwmrval = PeripheralClock / 10000;	
	
	LPC_PWM1->PR  = 0x09;                          /* FPCLK_FREQ��Ƶ��PWM����Ƶ��  */																								 /* ΪFPCLK_FREQ/10000           */
	LPC_PWM1->MCR = 0x02;                          /* PWMMR0��PWMTCƥ��ʱ��λPWMTC */
	LPC_PWM1->PCR = (1 << 13);                     /* ʹ��PWM5���                 */
	LPC_PWM1->MR0 = pwmrval;                       /* PWM���ʿ���/������ֵ         */
	LPC_PWM1->MR5 = (pwmrval / ( 100 - HIVCNT )) * tim;      /* ���ñ�������������λ��     */
	LPC_PWM1->LER = 0x7F;                          /* ��������PWMƥ��ֵ            */
	LPC_PWM1->TCR = 0x02;                          /* ��λPWMTC                    */
	LPC_PWM1->TCR = 0x09;                          /* ʹ��PWM                      */
	
}




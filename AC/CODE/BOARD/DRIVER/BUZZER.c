#include"Board.h"
#include "GPIO.h"


/****************************************************************************\
 Function:  BuzzerInit
 Descript:  蜂鸣器初始化
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void BuzzerInit(void)
{
	GpioSetDir(GPIO_BUZZER,OUTPUT);

	GpioSetLev(GPIO_BUZZER,LOW);
}


/****************************************************************************\
 Function:  BuzzerOn
 Descript:  使能蜂鸣器
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void BuzzerOn(void)
{
	GpioSetLev(GPIO_BUZZER,HIGH);
}


/****************************************************************************\
 Function:  BuzzerOff
 Descript:  失能蜂鸣器
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void BuzzerOff(void)
{
	GpioSetLev(GPIO_BUZZER,LOW);
}






#include"Board.h"
#include "GPIO.h"


/****************************************************************************\
 Function:  LedInit
 Descript:  LED初始化
 Input:	    无	
 Output:    无 
 Return:    0
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedInit(void)
{
	GpioSetDir(GPIO_LEDRUN,OUTPUT);
	GpioSetLev(GPIO_LEDRUN,LOW);


	GpioSetDir(GPIO_LEDUSB,OUTPUT);
	GpioSetLev(GPIO_LEDUSB,LOW);

	GpioSetDir(GPIO_LEDCHARGE,OUTPUT);
	GpioSetLev(GPIO_LEDCHARGE,LOW);

	GpioSetDir(GPIO_LEDPWR,OUTPUT);
	GpioSetLev(GPIO_LEDPWR,LOW);

	GpioSetDir(GPIO_LEDWARNING,OUTPUT);
	GpioSetLev(GPIO_LEDWARNING,LOW);

	GpioSetDir(GPIO_LEDCS,OUTPUT);
	GpioSetLev(GPIO_LEDCS,LOW);
	
	//
	GpioSetLev(GPIO_LEDPWR,LOW);
	GpioSetLev(GPIO_LEDPWR,HIGH);
	GpioSetLev(GPIO_LEDPWR,LOW);
	GpioSetLev(GPIO_LEDPWR,HIGH);
	
	GpioSetLev(GPIO_LEDCHARGE,LOW);
	GpioSetLev(GPIO_LEDCHARGE,HIGH);
	GpioSetLev(GPIO_LEDCHARGE,LOW);
	GpioSetLev(GPIO_LEDCHARGE,HIGH);
	
	GpioSetLev(GPIO_LEDWARNING,LOW);
	GpioSetLev(GPIO_LEDWARNING,HIGH);
	GpioSetLev(GPIO_LEDWARNING,LOW);
	GpioSetLev(GPIO_LEDWARNING,HIGH);
	
	GpioSetLev(GPIO_LEDCS,LOW);
	GpioSetLev(GPIO_LEDCS,HIGH);
	GpioSetLev(GPIO_LEDCS,LOW);
	GpioSetLev(GPIO_LEDCS,HIGH);
	
	
	
//	GpioSetDir(GPIO_INPUT1, INPUT);
//	GpioSetDir(GPIO_INPUT2, INPUT);
//	GpioSetDir(GPIO_INPUT3, INPUT);
//	GpioSetDir(GPIO_INPUT4, INPUT);
	
//	while (1){
//		if(GpioGetLev(GPIO_INPUT1)==LOW){
//			__nop();
//		} else {
//			__nop();
//		}
//		
//		if(GpioGetLev(GPIO_INPUT2)==LOW){
//			__nop();
//		} else {
//			__nop();
//		}
//		
//		if(GpioGetLev(GPIO_INPUT3)==LOW){
//			__nop();
//		} else {
//			__nop();
//		}
//		
//		if(GpioGetLev(GPIO_INPUT4)==LOW){
//			__nop();
//		} else {
//			__nop();
//		}
//	}
	

}




/****************************************************************************\
 Function:  LenRunOn
 Descript:  运行灯亮
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedRunOn(void)
{
	GpioSetLev(GPIO_LEDRUN,HIGH);
}




/****************************************************************************\
 Function:  LedRunOff
 Descript:  运行灯灭
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedRunOff(void)
{
	GpioSetLev(GPIO_LEDRUN,LOW);
}


/****************************************************************************\
 Function:  LedUsbOn
 Descript:  USB灯亮
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedUsbOn(void)
{
	GpioSetLev(GPIO_LEDUSB,HIGH);
}



/****************************************************************************\
 Function:  LedUsbOff
 Descript:  USB灯灭
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedUsbOff(void)
{
	GpioSetLev(GPIO_LEDUSB,LOW);
}



/****************************************************************************\
 Function:  LedChargeOn
 Descript:  充电灯亮
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedChargeOn(void)
{
	GpioSetLev(GPIO_LEDCHARGE,HIGH);
}


/****************************************************************************\
 Function:  LedChargeOff
 Descript:  充电灯灭
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedChargeOff(void)
{
	GpioSetLev(GPIO_LEDCHARGE,LOW);
}






/****************************************************************************\
 Function:  LedPwrOn
 Descript:  电源灯亮
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedPwrOn(void)
{
	GpioSetLev(GPIO_LEDPWR,HIGH);
}



/****************************************************************************\
 Function:  LedPwrOff
 Descript:  电源灯灭
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedPwrOff(void)
{
	GpioSetLev(GPIO_LEDPWR,LOW);
}




/****************************************************************************\
 Function:  LedWarnOn
 Descript:  警告灯亮
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedWarnOn(void)
{
	GpioSetLev(GPIO_LEDWARNING,HIGH);
}

/****************************************************************************\
 Function:  LedWarnOff
 Descript:  警告灯灭
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedWarnOff(void)
{
	GpioSetLev(GPIO_LEDWARNING,LOW);
}


/****************************************************************************\
 Function:  LedCsOn
 Descript:  CS灯亮
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedCsOn(void)
{
	GpioSetLev(GPIO_LEDCS,HIGH);
}



/****************************************************************************\
 Function:  LedCsOff
 Descript:  CS灯灭
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void LedCsOff(void)
{
	GpioSetLev(GPIO_LEDCS,LOW);
}








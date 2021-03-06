#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <LPC177x_8x.H>
#include "stdint.h"
#include "io.h"
#include "includes.h"






/************************************************************************************************************
** 函 数 名 : Swich_ModeIn
** 功能描述 : 配置Mode管脚为输入
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void Swich_ModeIn(void)
{
	LPC_GPIO2->DIR &= (~SWITCH_);
}


/************************************************************************************************************
** 函 数 名 : GunMode
** 功能描述 : 检测模式
** 输    入 : 无
** 输    出 : 0  或 1 
** 返    回 : 无
*************************************************************************************************************
*/
uint8_t GunMode(void)
{
	static uint8_t LastMode;
	uint8_t mode;
	mode = (LPC_GPIO2->PIN & SWITCH_) >> 11;
	if(mode != LastMode){
		OSTimeDlyHMSM(0, 0, 0, 10);
		mode = (LPC_GPIO2->PIN & SWITCH_) >> 11;
		LastMode = mode;
		return mode;			
	}
	return mode;		

}











/************************************************************************************************************
** 函 数 名 : TEST_CC
** 功能描述 : 检查CC 线是否连接
** 输    入 : MODE = 1 带枪    MODE = 0 不带枪
** 输    出 : 无
** 返    回 : 插枪返回1，未插枪返回 0
*************************************************************************************************************
*/
uint8_t Test_CC(uint8_t MODE) 
{
	uint8_t CcStatus;
	static uint8_t LastCcStatus;
	
	 
	if(GunMode() != GUNSEAT){
		return 1;
	}
	CcStatus = (LPC_GPIO1->PIN & TEST_cc) >> 28;
	if(LastCcStatus != CcStatus){
		OSTimeDlyHMSM(0, 0, 0, 10);
		CcStatus = (LPC_GPIO1->PIN & TEST_cc) >> 28;	
		LastCcStatus = CcStatus;
	}	
	if(CcStatus){
		return 0;
	}else{
		return 1;
	}
	
}



/************************************************************************************************************
** 函 数 名 : Test_Lock
** 功能描述 : 检查是否锁止  lockstatus = 0 锁  
** 输    入 : 无
** 输    出 : 无
** 返    回 : 锁止返回 0   未锁止返回 1
*************************************************************************************************************
*/
uint8_t Test_Lock(void)		
{
	if(GunMode() != GUNSEAT){
		return 0;
	}
	if(LPC_GPIO1->PIN & _LOCKIN){
		return 1;
	}else{
		return 0;
	}
}





/************************************************************************************************************
** 函 数 名 : AC_OFF
** 功能描述 : 断开接触器
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void  AC_OFF(void)
{
	LPC_GPIO3->DIR |= AC;
	LPC_GPIO3->CLR |= AC;	 
}

/************************************************************************************************************
** 函 数 名 : AC_ON
** 功能描述 : 吸合接触器
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void AC_ON(void)
{
	LPC_GPIO3->DIR |= AC;       //继电器闭合
	LPC_GPIO3->SET |= AC;	 
}


/************************************************************************************************************
** 函 数 名 : E_LOCK
** 功能描述 : 电子锁上锁
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void E_Lock(void)
{
	if(GunMode() != GUNSEAT)
		return ;
	LPC_GPIO3->DIR |= _LOCK; 
	LPC_GPIO3->SET |= _LOCK;  
}	

/************************************************************************************************************
** 函 数 名 : E_Unlock
** 功能描述 : 电子锁解锁
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void E_Unlock(void)   
{	
	LPC_GPIO3->DIR |= _LOCK;
	LPC_GPIO3->CLR |= _LOCK;  
}







/************************************************************************************************************
** 函 数 名 : Test_Door
** 功能描述 : 门禁检测
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1 ==门禁打开   0 == 正常
*************************************************************************************************************
*/
uint8_t Test_Door(void)
{
  uint8_t DoorStatus;
	static uint8_t LastDoorStatus;

	if(GunMode() == GUNSEAT){  // 枪座型
		return 0;
	}
	DoorStatus = (LPC_GPIO1->PIN & _LOCKIN) >> 20;
	if(LastDoorStatus != DoorStatus){
		OSTimeDlyHMSM(0, 0, 0, 10);
		DoorStatus = (LPC_GPIO1->PIN & _LOCKIN) >> 20;
		LastDoorStatus = DoorStatus;
	}
	if(DoorStatus){
		return 0;
	}else{
		return 1;
	}
}





/************************************************************************************************************
** 函 数 名 : TestStop
** 功能描述 : 急停检测
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1 ==急停按下   0 == 正常
*************************************************************************************************************
*/
uint8_t TestStop(void)
{
	uint8_t Stop;
	static uint8_t LastStop;
	
	Stop = (LPC_GPIO3->PIN & STOP_) >> 25;   
	if(LastStop != Stop){
		OSTimeDlyHMSM(0, 0, 0, 10);
		Stop = (LPC_GPIO3->PIN & STOP_) >> 25;   
		LastStop = Stop;
	}
	if(Stop){
		return 1;
	}else{
		return 0;
	}
}




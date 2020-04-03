#include "Board.h"
#include <LPC177x_8x.H>
#include "UART.h"
#include "TIMER.h"
#include "CAN.h"
#include "LED.h"
#include "FRAM.h"
#include "MX25L.h"
#include "RTC.h"
#include "BUZZER.h"
#include "DWIN.h"
#include "EEPROM.h"
#include "RELAY.h"
#include "GPIO.h"
#include "SPI.h"
#include "includes.h"


/****************************************************************************\
 Function:  BoardInit
 Descript:  板级初始化
 Input:	    无	
 Output:    无 
 Return:    0
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
void BoardInit(void)
{	
	SystemInit();	      // 时钟   (CPU 12M p 60M)
	 
	UartInit(DEBUGUARTNO,115200);   // 串口

	TimerInit(0,SYSTIME); // 定时器
	
	CANInit(DEV_CAN1, 125000); //与充电控制板通信

	LedInit();			  // LED灯	
	
	RelayInit(); //继电器初始化
	
	FramInit();			  // 铁电

//	Mx25Init();			  // mx25 flash	

	RtcInit();			  // RTC

	BuzzerInit();		  // 蜂鸣器

	DwinInit(); //迪文屏初始化
	
	EepromInit(); //EEPROM初始化
	
	GpioSetDir(GPIO_NORCS,OUTPUT);
	SpiInit(MX25SPINO); //SPI初始化
	
//#if 1
//	if (test11 == 1){
//		buf[0] = 'h';
//		buf[1] = 'e';
//		buf[2] = 'l';
//		buf[3] = 'l';
//		buf[4] = 'o';
//		buf[5] = '\r';
//		buf[6] = '\n';
//		buf[7] = 0;
//		FramWrite(10, buf, 8);
//	}
//	memset(bufr, 0, 20);
//	FramRead(10, bufr, 20);
//	print("***%s***\r\n",bufr);
//#endif

//#if 1
//	if (test11 == 1){
//		buf[0] = '1';
//		buf[1] = '2';
//		buf[2] = '3';
//		buf[3] = '4';
//		buf[4] = '5';
//		buf[5] = '\r';
//		buf[6] = '\n';
//		buf[7] = 0;
//		Mx25Erase(0);
//		Mx25write(0,buf,8);
//	}
//	Mx25Read(0,bufr, 20);
//	print("***%s***\r\n",bufr);
//#endif	
}


void delay(uint16_t dely)
{
	uint16_t i=0, j=0;
	
	for(i=0; i <= dely/10; i++)
	{
		for(j = 0; j <= dely; j++)
		{
		}
	}
}


/****************************************************************************\
 Function:  DelayMs 
 Descript:  延时毫秒，利用操作系统的调度
 Input:	    延时的毫秒数	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-17
 *****************************************************************************/
void DelayMs(uint16_t data)
{
	  OSTimeDly(data);
}



























































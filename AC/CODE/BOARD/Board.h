#ifndef _BOARD_H
#define _BOARD_H

#include <stdint.h>

//#include	<stdio.h>
//#include    <stdlib.h>
//#include    <string.h>
//#include    <ctype.h>
//#include    <setjmp.h>
//#include    <rt_misc.h>
//#include    <stdarg.h>
//#include    <math.h>
//#include    <time.h>


//#include 	<LPC177x_8x.H>


//#include 	"GPIO.h"
//#include 	"UART.h"
//#include 	"TIMER.h"
//#include    "LED.h"
//#include    "SPI.h"
//#include    "FRAM.h"
//#include    "MX25L.h"
//#include    "RTC.h"
//#include    "RS485.h"
//#include    "Buzzer.h"
////#include    "W5500.h"
//#include    "MODEM.h"
//#include    "MT625.h"
//#include    "Dwin.h"
//#include    "ELM.h"

#define      SYSTIME         10        // 10ms的中断

#define      T10MS           1		// 10ms	
#define      T100MS          10		// 100ms
#define      T1S			 100	// 1s	
#define      T1M			 6000   // 1M
#define      T1H			 360000  // 1H











// 	串口0  P002  P003  
#define GPIO_U0TXD     		2
#define GPIO_U0RXD     		3

// 	串口1  P200  P201  
#define GPIO_U1TXD     		200
#define GPIO_U1RXD     		201

// 串口2  P010  P011
#define GPIO_U2TXD     		10
#define GPIO_U2RXD     		11

// 串口3  P025 P026
#define GPIO_U3TXD     		25
#define GPIO_U3RXD     		26

// CAN1  P021  P022
#define GPIO_CANRD1    		21
#define GPIO_CANTD1    		22

// CAN2  P207 P208
#define GPIO_CANRD2    		207
#define GPIO_CANTD2    		208


// LED灯
#define GPIO_LEDRUN    		12
#define GPIO_LEDUSB    		13
#define GPIO_LEDCHARGE 		415
#define GPIO_LEDPWR    		425
#define GPIO_LEDWARNING 	428
#define GPIO_LEDCS			429

//开关量检测
#define GPIO_INPUT1			325
#define GPIO_INPUT2			120
#define GPIO_INPUT3			123
#define GPIO_INPUT4			124

//继电器输出
#define GPIO_OUTPUT1			128
#define GPIO_OUTPUT2			129
#define GPIO_OUTPUT3			0
#define GPIO_OUTPUT4			1


// 铁电存储  SPI0
#define GPIO_FRAMSCK   		15
#define GPIO_FRAMCS    		16
#define GPIO_FRAMMISO  		17
#define GPIO_FRAMMOSI  		18


// NORFLASH存储 SPI1
#define GPIO_NORMISO   		118
#define GPIO_NORSCK    		119
#define GPIO_NORMOSI   		122
#define GPIO_NORCS     		126

// 网卡芯片  SPI2
#define GPIO_SNETCLK   		100
#define GPIO_SNETMOSI  		101
#define GPIO_SNETMISO  		104
#define GPIO_SNETCS    		108
#define GPIO_SNETINT  		206
#define GPIO_SNETRST  		410



// AD转换
#define GPIO_ADC1      		29
#define GPIO_ADC2      		30

// USB
#define GPIO_USBD2A    		31
#define GPIO_USBD2B    		32

// 看门狗
#define GPIO_WDI       		27

// 蜂鸣器
#define GPIO_BUZZER    		212

// 485
#define GPIO_485CON2   		403

// GPRRS 
#define GPIO_GPRSRST   		500
#define GPIO_GPRSPWR   		326

// 
#define GPIO_MODE8     		404




#define FRAMSPINO       	0    //铁电 SPI
#define MX25SPINO       	1    //MX25 SPI
#define ETHSPINO            2    //以太网 SPI 


#define DEBUGUARTNO		0		//调试串口
#define DISPUARTNO		1   // 串口屏 UART
#define RS485UARTNO		2   // 485通信 UART
#define GPRSUARTNO		3   // GPRS模块 UART
#define MTUARTNO			4   // 读卡器 UART








void BoardInit(void);
void delay(uint16_t dely);
void DelayMs(uint16_t data);





#endif


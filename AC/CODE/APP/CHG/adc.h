#ifndef __ADC_H_
#define __ADC_H_
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <LPC177x_8x.H>
#include "stdint.h"

#ifndef _BV
#define _BV(_x_) (1UL << (_x_))
#endif
#define ADC_CLK             400000   /* set to 400kHz        */
#define ADC_12BITS          1
#define ADC_INTERRUPT_FLAG  1        /* 1 ：使能中断 0 ：软件控制      */

#define ADC_12BITS          1

#define ADC_DONE            0x80000000
#define ADC_OVERRUN         0x40000000
#define ADC_ADINT           0x00010000
#define ADCVAL_NUM          50     // 平均值


#define ADTRANSITION			75 
#define MIN6V	 				5000		//对应5V
#define MAX6V					7000		//对应7V
#define MIN9V	 				8000		//对应8V
#define MAX9V					10000 		//对应10V
#define MIN12V					10260		//寄存器取值2717	  对应未插枪的最小取值范围
#define MAX12V					12092		//寄存器取值3211		对应未插枪的最大取值范围

#define		ADC12V		4
#define		ADC9V			3
#define		ADC6V			2

#define 	ADC0V			1

#define TEMP50       733            //50摄氏度
#define TEMP85       875            //85摄氏度


void ADCInit( uint32_t ADC_Clk );
uint8_t GetTemp(uint8_t port);
void ADCScan(void);
uint8_t Get_AdcStatus(uint8_t port);

#endif

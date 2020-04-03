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
#define ADC_INTERRUPT_FLAG  1        /* 1 ��ʹ���ж� 0 ���������      */

#define ADC_12BITS          1

#define ADC_DONE            0x80000000
#define ADC_OVERRUN         0x40000000
#define ADC_ADINT           0x00010000
#define ADCVAL_NUM          50     // ƽ��ֵ


#define ADTRANSITION			75 
#define MIN6V	 				5000		//��Ӧ5V
#define MAX6V					7000		//��Ӧ7V
#define MIN9V	 				8000		//��Ӧ8V
#define MAX9V					10000 		//��Ӧ10V
#define MIN12V					10260		//�Ĵ���ȡֵ2717	  ��Ӧδ��ǹ����Сȡֵ��Χ
#define MAX12V					12092		//�Ĵ���ȡֵ3211		��Ӧδ��ǹ�����ȡֵ��Χ

#define		ADC12V		4
#define		ADC9V			3
#define		ADC6V			2

#define 	ADC0V			1

#define TEMP50       733            //50���϶�
#define TEMP85       875            //85���϶�


void ADCInit( uint32_t ADC_Clk );
uint8_t GetTemp(uint8_t port);
void ADCScan(void);
uint8_t Get_AdcStatus(uint8_t port);

#endif

#ifndef __PWM_H
#define __PWM_H
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <LPC177x_8x.H>
#include "stdint.h"

#define		HIVCNT		  0			    //����ߵ�ƽ
#define		PWMCNT	   	(100-54)			//32A (ϵ��0.6)
#define		PWMLOW	   	100			//�����ѹ
void PwmInit(uint8_t tim);
#endif


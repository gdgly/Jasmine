#ifndef __PWM_H
#define __PWM_H
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <LPC177x_8x.H>
#include "stdint.h"

#define		HIVCNT		  0			    //输出高电平
#define		PWMCNT	   	(100-54)			//32A (系数0.6)
#define		PWMLOW	   	100			//输出负压
void PwmInit(uint8_t tim);
#endif


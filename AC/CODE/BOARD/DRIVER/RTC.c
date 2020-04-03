#include"Board.h"

#include 	<LPC177x_8x.H>
#include <stdio.h>
#include "RTC.h"

/****************************************************************************\
 Function:  RtcInit
 Descript:  RTC初始化
 Input:	    无	
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-23
 *****************************************************************************/
void RtcInit (void)
{	
  	LPC_SC->PCONP |= (1 << 9);                  
	
	LPC_RTC->CCR   = 0x00;                                                  
    LPC_RTC->ILR   = 0x03;                                                 
    LPC_RTC->CIIR  = 0x1;                                                
	LPC_RTC->AMR   = 0xff;                                 
		
//    LPC_RTC->YEAR  = 0;                                        
//    LPC_RTC->MONTH = 0;
//    LPC_RTC->DOM   = 0;
//    LPC_RTC->DOW   = 0;
//    LPC_RTC->HOUR  = 0;
//    LPC_RTC->MIN   = 0;
//    LPC_RTC->SEC   = 0;    
    LPC_RTC->CIIR  = 0x01;
	LPC_RTC->ILR = (0x01 | 0x02);    
	LPC_RTC->CCR   = 0x01;                                                 
}


/****************************************************************************\
 Function:  RtcWrite
 Descript:  写RTC时间
 Input:	    RTC时间
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-23
 *****************************************************************************/
void RtcWrite(Rtc *data)
{
	LPC_RTC->YEAR = data->year;                                        
    LPC_RTC->MONTH = data->month;
    LPC_RTC->DOM = data->day;
    LPC_RTC->HOUR = data->hour;
    LPC_RTC->MIN = data->min;
    LPC_RTC->SEC = data->sec;
}

/****************************************************************************\
 Function:  RtcRead
 Descript:  读RTC时间
 Input:	    无
 Output:    RTC时间 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-23
 *****************************************************************************/
void RtcRead(Rtc *data)
{
	data->year = LPC_RTC->YEAR;                                        
    data->month = LPC_RTC->MONTH;
   	data->day = LPC_RTC->DOM;
    data->hour = LPC_RTC->HOUR;
    data->min = LPC_RTC->MIN;
    data->sec = LPC_RTC->SEC;   
}




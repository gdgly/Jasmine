#ifndef _RTC_H
#define _RTC_H

typedef struct _RTC_
{
    uint16_t  year;
    uint8_t  	month;
    uint8_t  	day;
    uint8_t  	hour;
    uint8_t  	min;
    uint8_t  	sec;
}Rtc;


void RtcInit (void);
void RtcWrite(Rtc *data);
void RtcRead(Rtc *data);



#endif


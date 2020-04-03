#ifndef _CHGTASK_H
#define _CHGTASK_H

#include <stdint.h>
#include "App.h"
#include "Timer.h"
#include "RTC.h"
#include "UART.h"
#include "CAN.h"
#include "ChgInterface.h"
#include "ElmTask.h"
#include "board.h"
#include "para.h"


uint8_t GetEXT_StopCause(uint8_t port);
CHARGEINFO GetEXT_CCBInfo(uint8_t port);
uint8_t EXT_StartCharge(uint8_t port);
uint8_t EXT_StopCharge(uint8_t port);
void ChgHandle(uint8_t port);
void ChgTask(void *pdata);
uint16_t ReadPwm(uint8_t port);
#endif

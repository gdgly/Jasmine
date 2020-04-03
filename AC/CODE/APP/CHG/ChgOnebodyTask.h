#ifndef __CHGONEBODYTASH_H
#define __CHGONEBODYTASH_H
#include "io.h"
#include "pwm.h"
#include "adc.h"
uint8_t StartOnebodyChg(uint8_t port);
uint8_t StopOnebodyChg(uint8_t port);
uint8_t GetOnebodyStopCause(uint8_t port);
CHARGEINFO GetChgOnebodyInfo(uint8_t port);
void ChgOnebodyPro(uint8_t port);
void ChgOnebodyTask(void *pdata);
#endif


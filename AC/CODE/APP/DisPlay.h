#ifndef __DISPLAY_H
#define __DISPLAY_H
#include <stdint.h>

void DispMenuInfo(uint8_t* Time, uint8_t* Online);
void DispStartDeviceInfo(uint8_t MajorVer, uint8_t MinorVer, uint8_t PatchVer);
void DispNoPileUsed(void);
void DispInsertGunInfo(void);
void DispDeviceFault(uint8_t* tempbuf);
void DispErrIdGunInfo(uint8_t port, uint8_t* IdBuf);

//刷卡相关
void DispCardLockInfo(uint8_t* Pile );
void DispUnLockFailureInfo(uint32_t FailureId );

//显示当前余额
void DispRemainMoneyInfo(uint32_t Money);
//结算界面
void DispAccount(uint32_t SumEnergy, uint32_t SumMoney, uint32_t RemainMoney, uint32_t SumTime, uint8_t* Date);

//在线卡验证相关界面
void DispInVerify(void);
void DispVerifySuccesInfo(uint32_t Money);
void DispVerifyFailure(uint8_t* tempbuf);

//启动相关界面
void DispStartChgInfo(uint32_t Time);
void DispOperationInfo(void);
void DispStartSucessInfo(void);
void DispStartFailureInfo(void);

//升级中界面
void DispUpdataInfo( uint8_t Present, uint8_t* Tip);

//单枪处理相关界面
void DispAIdleInfo(uint8_t* ACode, uint8_t* APileNum, uint8_t GunStatus);
void DispAChargeInfo(uint32_t SumEnergy, uint32_t SumMoney, uint32_t CurrentA, uint32_t CurrentB, uint32_t CurrentC, uint32_t VoltageA,  uint32_t VoltageB,  uint32_t VoltageC, uint32_t SumTime);
void DispAStopInfo(uint8_t* Prompt, uint8_t* StpReason, uint32_t SumEnergy, uint32_t SumMoney );




//双枪处理相关界面
void DispAIdleBIdleInfo( uint8_t* ACode, uint8_t* APileNum, uint8_t AGunStatus, \
							           uint8_t* BCode, uint8_t* BPileNum, uint8_t BGunStatus);

void DispAIdleBChgInfo( uint8_t* ACode, uint8_t* APileNum, uint8_t AGunStatus,  \
												uint32_t BSumEnergy, uint32_t BSumMoney, uint32_t BCurrentA, uint32_t BCurrentB,uint32_t BCurrentC,uint32_t BVoltageA, uint32_t BVoltageB, uint32_t BVoltageC,  uint32_t BSumTime );

void DispAIdleBStopInfo( uint8_t* ACode, uint8_t* APileNum, uint8_t AGunStatus, \
												 uint8_t* BPrompt, uint8_t* BStpReason, uint32_t BSumEnergy, uint32_t BSumMoney );


void DispAChgBIdleInfo( uint32_t ASumEnergy, uint32_t ASumMoney, uint32_t ACurrentA, uint32_t ACurrentB, uint32_t ACurrentC, uint32_t AVoltageA, uint32_t AVoltageB,  uint32_t AVoltageC,   uint32_t ASumTime, \
											  uint8_t* BCode, uint8_t* BPileNum, uint8_t BGunStatus);

void DispAChgBChgInfo( uint32_t ASumEnergy, uint32_t ASumMoney, uint32_t ACurrentA, uint32_t ACurrentB, uint32_t ACurrentC, uint32_t AVoltageA, uint32_t AVoltageB, uint32_t AVoltageC,  uint32_t ASumTime, \
											 uint32_t BSumEnergy, uint32_t BSumMoney, uint32_t BCurrentA, uint32_t BCurrentB, uint32_t BCurrentC, uint32_t BVoltageA, uint32_t BVoltageB, uint32_t BVoltageC,  uint32_t BSumTime );


void DispAChgBStopInfo( uint32_t ASumEnergy, uint32_t ASumMoney, uint32_t ACurrentA, uint32_t ACurrentB, uint32_t ACurrentC, uint32_t AVoltageA, uint32_t AVoltageB, uint32_t AVoltageC,  uint32_t ASumTime, \
												uint8_t* BPrompt, uint8_t* BStpReason, uint32_t BSumEnergy, uint32_t BSumMoney );


void DispAStopBChgInfo( uint8_t* APrompt, uint8_t* AStpReason, uint32_t ASumEnergy, uint32_t ASumMoney, \
												uint32_t BSumEnergy, uint32_t BSumMoney, uint32_t BCurrentA, uint32_t BCurrentB, uint32_t BCurrentC, uint32_t BVoltageA, uint32_t BVoltageB, uint32_t BVoltageC, uint32_t BSumTime );

void DispAStopBStopInfo( uint8_t* APrompt, uint8_t* AStpReason, uint32_t ASumEnergy, uint32_t ASumMoney, \
												 uint8_t* BPrompt, uint8_t* BStpReason, uint32_t BSumEnergy, uint32_t BSumMoney );

void DispAStopBIdleInfo( uint8_t* APrompt, uint8_t* AStpReason, uint32_t ASumEnergy, uint32_t ASumMoney, \
										     uint8_t* BCode, uint8_t* BPileNum, uint8_t BGunStatus);


void DispRecordInfo(uint8_t StartType,uint32_t StartCardID,uint8_t* StartDateTime,uint32_t SumTime, uint32_t SumEnergy,uint32_t SumMoney,uint8_t StopCause, uint8_t Line );

void DispRecordNullInfo(uint8_t i );

void DispRecordMoreInfo(uint8_t ChgPort,uint8_t StartType,uint32_t StartCardID,uint8_t* StartDateTime, uint8_t* StopDateTime,uint32_t SumTime, uint32_t SumEnergy,uint32_t StartMoney, uint32_t SumMoney,uint8_t StopCause,uint8_t StartSoc, uint8_t StopSoc );












#endif





#ifndef _RECORD_H
#define _RECORD_H
#include "Cfg.h"
//铁电存储数据分布
//锁卡信息 预留1024个字节空间
#define FRAMADDR_LOCKCARDINFO	0
//充电记录信息 预留1024个字节空间
#define FRAMADDR_CHARGERECORDINFO	0x400
//充电过程中记录信息 预留2*1024字节空间
#define FRAMADDR_CHARGINGRECORDINFO	0x800

//充电记录索引信息
typedef struct{
	uint16_t index;//索引号
	uint8_t recordstate[RECORD_TOTAL_NUM];//记录状态 0无记录 1有记录
}CHARGERECORDINFO;

//锁卡记录信息
typedef struct{
	uint32_t StorePoint;//存储指针 0-199
	uint16_t SerialPoint;//序列号指针
	uint16_t SerialNum[100];//序列号
	uint32_t CardID[100];//卡号
}LOCKCARDINFO;

//充电过程中记录信息
typedef struct{
	uint16_t Head;//固定为0x5a5a
	uint16_t Len;//数据区有效字节数 不超过1000字节
	uint8_t data[1000];//数据区
	uint16_t Tail;//从头标志（包含头标志）到尾标志（不包含尾标志）的数据累加和
}CHARGINGINFO;

uint32_t GetChargeRecordIndex(uint8_t port);
uint32_t GetChargeRecordNumber(uint8_t port);

uint8_t InsertChargeRecord(uint8_t *recordbuf, uint16_t recordlen);
void DeleteChargeRecord(uint16_t index);
uint8_t FindChargeRecord(uint16_t index, uint8_t *recordbuf, uint16_t recordlen);

uint32_t InsertLockCardRecord(uint32_t CardID);
void DeleteLockCardRecord(uint32_t CardID);
uint8_t FindLockCardRecord(uint32_t CardID, uint32_t *SerialNum);

uint8_t InsertChargingRecord(uint8_t port, uint8_t *recordbuf, uint16_t recordlen);
void DeleteChargingRecord(uint8_t port);

void RecordInit(void);
void DeleteAllRecord(void);

#endif



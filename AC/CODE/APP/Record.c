#include "App.h"
#include "FRAM.h"
#include "ff.h"
#include "diskio.h"
#include "Record.h"
#include "UART.h"
#include <string.h>
#include <stdio.h>
#include "includes.h"
#include "Cfg.h"
static CHARGERECORDINFO ChargeRecordInfo;

static LOCKCARDINFO LockCardInfo;

static CHARGINGINFO ChargingInfo[2];

static FATFS FecordFS[2];

/************************************************************************************************************
** 函 数 名 : GetChargeRecordIndex
** 功能描述 : 获取充电记录索引号
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	索引号
*************************************************************************************************************
*/
uint32_t GetChargeRecordIndex(uint8_t port)
{
	return ChargeRecordInfo.index;
}

/************************************************************************************************************
** 函 数 名 : GetChargeRecordNumber
** 功能描述 : 获取充电记录数量
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	数量
*************************************************************************************************************
*/
uint32_t GetChargeRecordNumber(uint8_t port)
{
	uint32_t i, num = 0;
	for (i = 0; i < RECORD_TOTAL_NUM; i++){
		if (ChargeRecordInfo.recordstate[i] != 0){
			num++;
		}
	}
	return num;
}

/************************************************************************************************************
** 函 数 名 : ChargeRecordInit
** 功能描述 : 充电记录初始化
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void ChargeRecordInit(void)
{
	uint8_t os_err;
	uint16_t i;
	
	memset(&ChargeRecordInfo, 0, sizeof(CHARGERECORDINFO));
	OSMutexPend (FramMutex, 0, &os_err);
	FramRead(FRAMADDR_CHARGERECORDINFO, (uint8_t *)&ChargeRecordInfo, sizeof(CHARGERECORDINFO));
	OSMutexPost (FramMutex);
	if (ChargeRecordInfo.index >= RECORD_TOTAL_NUM){
		memset(&ChargeRecordInfo, 0, sizeof(CHARGERECORDINFO));
	} else {
		for (i = 0; i < RECORD_TOTAL_NUM; i++){
			if (ChargeRecordInfo.recordstate[i] > 1){
				memset(&ChargeRecordInfo, 0, sizeof(CHARGERECORDINFO));
				break;
			}
		}
	}
}

/************************************************************************************************************
** 函 数 名 : InsertChargeRecord
** 功能描述 : 插入充电记录
** 输    入 : recordbuf 记录缓冲区 recordlen 记录长度 不超过1000字节
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t InsertChargeRecord(uint8_t *recordbuf, uint16_t recordlen)
{
	uint8_t os_err;
	char FileName[16];
	FIL file;
	FRESULT ret;
	uint32_t ByteNumToWrite;
	
	if (recordlen > 1000)
		return 0;
	if ((ChargeRecordInfo.index + 1) >= RECORD_TOTAL_NUM)
		ChargeRecordInfo.index = 0;
	else
		ChargeRecordInfo.index++;
	ChargeRecordInfo.recordstate[ChargeRecordInfo.index] = 1;
	
	OSMutexPend (SFlashMutex, 0, &os_err);
	sprintf(FileName,"1:/rec%d.txt", ChargeRecordInfo.index);
	ret = f_open(&file, FileName, FA_CREATE_ALWAYS | FA_WRITE);
	if(ret != FR_OK){
		OSMutexPost (SFlashMutex);
		return 0;
	}
	ret = f_write(&file, recordbuf, recordlen, &ByteNumToWrite);
	if((ret != FR_OK) || (ByteNumToWrite != recordlen)){
		f_close(&file);
		ret = f_mkfs(SERIALFLASH, 0, 4096);
		if (ret != FR_OK){
			print("error:record filesystem mount fail!\r\n");
		}
		ret = f_open(&file, FileName, FA_CREATE_ALWAYS | FA_WRITE);
		if(ret != FR_OK){
			OSMutexPost (SFlashMutex);
			return 0;
		}
		ret = f_write(&file, recordbuf, recordlen, &ByteNumToWrite);
		if((ret != FR_OK) || (ByteNumToWrite != recordlen)){
			f_close(&file);
			OSMutexPost (SFlashMutex);
			return 0;
		}
	}
	f_close(&file);
	OSMutexPost (SFlashMutex);
	
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_CHARGERECORDINFO, (uint8_t *)&ChargeRecordInfo, sizeof(CHARGERECORDINFO));
	OSMutexPost (FramMutex);
	
	return 1;
}

/************************************************************************************************************
** 函 数 名 : DeleteChargeRecord
** 功能描述 : 删除充电记录
** 输    入 : index 索引号
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
void DeleteChargeRecord(uint16_t index)
{
	uint8_t os_err;
	char FileName[16];
	
	if (index > RECORD_TOTAL_NUM)
		return;
	
	OSMutexPend (SFlashMutex, 0, &os_err);
	sprintf(FileName,"1:/rec%d.txt", index);
	f_unlink(FileName);
	OSMutexPost (SFlashMutex);
	
	ChargeRecordInfo.recordstate[index] = 0;
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_CHARGERECORDINFO, (uint8_t *)&ChargeRecordInfo, sizeof(CHARGERECORDINFO));
	OSMutexPost (FramMutex);
	
	return;
}

/************************************************************************************************************
** 函 数 名 : FindChargeRecord
** 功能描述 : 查找充电记录
** 输    入 : index 索引号 recordbuf 记录缓冲区 recordlen 记录长度 不超过1000字节
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t FindChargeRecord(uint16_t index, uint8_t *recordbuf, uint16_t recordlen)
{
	uint8_t os_err;
	char FileName[16];
	FIL file;
	FRESULT ret;
	uint32_t ByteNumToRead;
	
	if (index > RECORD_TOTAL_NUM)
		return 0;
	if (ChargeRecordInfo.recordstate[index] == 0){
		return 0;
	}
	
	OSMutexPend (SFlashMutex, 0, &os_err);
	sprintf(FileName,"1:/rec%d.txt", index);
	ret = f_open(&file, FileName, FA_OPEN_EXISTING | FA_READ);
	if(ret != FR_OK){
		OSMutexPost (SFlashMutex);
		return 0;
	}
	ret = f_read(&file, recordbuf, recordlen, &ByteNumToRead);
	if(ret != FR_OK){
		f_close(&file);
		OSMutexPost (SFlashMutex);
		return 0;
	}
	f_close(&file);
	OSMutexPost (SFlashMutex);
	
	return 1;
}

/************************************************************************************************************
** 函 数 名 : LockCardRecordInit
** 功能描述 : 锁卡记录初始化
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void LockCardRecordInit(void)
{
	uint8_t os_err;
	memset(&LockCardInfo, 0, sizeof(LOCKCARDINFO));
	OSMutexPend (FramMutex, 0, &os_err);
	FramRead(FRAMADDR_LOCKCARDINFO, (uint8_t *)&LockCardInfo, sizeof(LOCKCARDINFO));
	OSMutexPost (FramMutex);
}

/************************************************************************************************************
** 函 数 名 : InsertLockCardRecord
** 功能描述 : 插入锁卡记录
** 输    入 : CardID 卡号
** 输    出 : 无
** 返    回 :	锁卡记录号
*************************************************************************************************************
*/
uint32_t InsertLockCardRecord(uint32_t CardID)
{
	uint8_t i;
	uint8_t os_err;
	
	for (i = 0; i < 100; i++){
		if (LockCardInfo.CardID[i] == CardID){
			LockCardInfo.SerialNum[i] = 0;
			LockCardInfo.CardID[i] = 0;
		}
	}
	
	if ((LockCardInfo.StorePoint + 1) >= 100)
		LockCardInfo.StorePoint = 0;
	else
		LockCardInfo.StorePoint++;
	LockCardInfo.SerialPoint = LockCardInfo.SerialPoint + 1;
	LockCardInfo.SerialNum[LockCardInfo.StorePoint] = LockCardInfo.SerialPoint;
	LockCardInfo.CardID[LockCardInfo.StorePoint] = CardID;
	
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_LOCKCARDINFO, (uint8_t *)&LockCardInfo, sizeof(LOCKCARDINFO));
	OSMutexPost (FramMutex);
	
	return LockCardInfo.SerialNum[LockCardInfo.StorePoint];
}

/************************************************************************************************************
** 函 数 名 : DeleteLockCardRecord
** 功能描述 : 删除锁卡记录
** 输    入 : CardID 卡号
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void DeleteLockCardRecord(uint32_t CardID)
{
	uint8_t i;
	uint8_t os_err;
	
	for (i = 0; i < 100; i++){
		if (LockCardInfo.CardID[i] == CardID){
			LockCardInfo.SerialNum[i] = 0;
			LockCardInfo.CardID[i] = 0;
		}
	}
	
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_LOCKCARDINFO, (uint8_t *)&LockCardInfo, sizeof(LOCKCARDINFO));
	OSMutexPost (FramMutex);
}

/************************************************************************************************************
** 函 数 名 : FindLockCardRecord
** 功能描述 : 搜索锁卡记录
** 输    入 : CardID 卡号
** 输    出 : SerialNum 序列号
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t FindLockCardRecord(uint32_t CardID, uint32_t *SerialNum)
{
	uint8_t i;
	
	for (i = 0; i < 100; i++){
		if (LockCardInfo.CardID[i] == CardID){
			*SerialNum = LockCardInfo.SerialNum[i];
			return 1;
		}
	}
	
	return 0;
}

/************************************************************************************************************
** 函 数 名 : ChargingRecordInit
** 功能描述 : 充电过程中记录初始化
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void ChargingRecordInit(void)
{
	uint8_t os_err;
	uint8_t i = 0,*p1,*p2;
	uint16_t sum,j,len;
	
	for (i = 0; i < 2; i++){
		memset(&ChargingInfo[i], 0, sizeof(CHARGINGINFO));
		OSMutexPend (FramMutex, 0, &os_err);
		FramRead(FRAMADDR_CHARGINGRECORDINFO + i * 1024, (uint8_t *)&ChargingInfo[i], sizeof(CHARGINGINFO));
		OSMutexPost (FramMutex);
		
		p1 = (uint8_t *)&ChargingInfo[i].Head;
		p2 = (uint8_t *)&ChargingInfo[i].Tail;
		len = p2 - p1;
		sum = 0;
		for (j = 0; j < len; j++)
			sum += *p1++;
		if ((ChargingInfo[i].Head == 0x5a5a) && (sum == ChargingInfo[i].Tail) && (ChargingInfo[i].Len <= 1000)){
			InsertChargeRecord(ChargingInfo[i].data, ChargingInfo[i].Len);
		}
		memset(&ChargingInfo[i], 0, sizeof(CHARGINGINFO));
		OSMutexPend (FramMutex, 0, &os_err);
		FramWrite(FRAMADDR_CHARGINGRECORDINFO + i * 1024, (uint8_t *)&ChargingInfo[i], sizeof(CHARGINGINFO));
		OSMutexPost (FramMutex);
	}
}

/************************************************************************************************************
** 函 数 名 : InsertChargingRecord
** 功能描述 : 插入充电过程记录
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... recordbuf 缓冲区 recordlen 长度
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t InsertChargingRecord(uint8_t port, uint8_t *recordbuf, uint16_t recordlen)
{
	uint8_t os_err;
	uint8_t *p1,*p2;
	uint16_t sum,j,len;
	
	if (recordlen > 1000)
		return 0;
	memset(&ChargingInfo[port], 0, sizeof(CHARGINGINFO));
	ChargingInfo[port].Head = 0x5a5a;
	ChargingInfo[port].Len = recordlen;
	memcpy(ChargingInfo[port].data, recordbuf, recordlen);
	p1 = (uint8_t *)&ChargingInfo[port].Head;
	p2 = (uint8_t *)&ChargingInfo[port].Tail;
	len = p2 - p1;
	sum = 0;
	for (j = 0; j < len; j++)
		sum += *p1++;
	ChargingInfo[port].Tail = sum;
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_CHARGINGRECORDINFO + port * 1024, (uint8_t *)&ChargingInfo[port], sizeof(CHARGINGINFO));
	OSMutexPost (FramMutex);
	return 1;
}

/************************************************************************************************************
** 函 数 名 : DeleteChargingRecord
** 功能描述 : 删除充电过程中记录
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
void DeleteChargingRecord(uint8_t port)
{
	uint8_t os_err;
	
	memset(&ChargingInfo[port], 0, sizeof(CHARGINGINFO));
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_CHARGINGRECORDINFO + port * 1024, (uint8_t *)&ChargingInfo[port], sizeof(CHARGINGINFO));
	OSMutexPost (FramMutex);
}


/************************************************************************************************************
** 函 数 名 : RecordInit
** 功能描述 : 记录初始化函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void RecordInit(void)
{
	FRESULT ret;
	FIL file;
	
	//挂载
	ret = f_mount(SERIALFLASH, &FecordFS[SERIALFLASH]);
	if (ret != FR_OK){
		ret = f_mkfs(SERIALFLASH, 0, 4096);
		if (ret != FR_OK){
			print("record filesystem mount fail!\r\n");
			while(1);
		}
	}
	//检查文件
	ret = f_open(&file,"1:/rec.txt",FA_OPEN_EXISTING | FA_READ);
	if(ret != FR_OK){
		ret = f_open(&file,"1:/rec.txt",FA_OPEN_EXISTING | FA_READ);
		if(ret != FR_OK){
			//格式化文件
			print("create filesystem for record\r\n");
			ret = f_mkfs(SERIALFLASH, 0, 4096);
			if(ret != FR_OK){
				print("create filesystem fail!\r\n");
				while(1);
			}
			f_open(&file,"1:/rec.txt",FA_CREATE_ALWAYS | FA_WRITE);
			f_close(&file);
		}
	}
	f_close(&file);
	
	//充电记录初始化
	ChargeRecordInit();
	
	//锁卡记录初始化
	LockCardRecordInit();
	
	//充电过程中记录初始化
	ChargingRecordInit();
}

/************************************************************************************************************
** 函 数 名 : DeleteAllRecord
** 功能描述 : 删除所有记录函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void DeleteAllRecord(void)
{
	uint8_t os_err;
	
	//挂载并格式化文件
	f_mount(SERIALFLASH, &FecordFS[SERIALFLASH]);
	f_mkfs(SERIALFLASH, 0, 4096);
	
	//删除充电记录信息
	memset(&ChargeRecordInfo, 0, sizeof(CHARGERECORDINFO));
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_CHARGERECORDINFO, (uint8_t *)&ChargeRecordInfo, sizeof(CHARGERECORDINFO));
	OSMutexPost (FramMutex);
	
	//删除锁卡记录信息
	memset(&LockCardInfo, 0, sizeof(LOCKCARDINFO));
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_LOCKCARDINFO, (uint8_t *)&LockCardInfo, sizeof(LOCKCARDINFO));
	OSMutexPost (FramMutex);
	
	//删除充电过程中记录信息
	memset(&ChargingInfo[0], 0, sizeof(CHARGINGINFO));
	memset(&ChargingInfo[1], 0, sizeof(CHARGINGINFO));
	OSMutexPend (FramMutex, 0, &os_err);
	FramWrite(FRAMADDR_CHARGINGRECORDINFO + 0 * 1024, (uint8_t *)&ChargingInfo[0], sizeof(CHARGINGINFO));
	FramWrite(FRAMADDR_CHARGINGRECORDINFO + 0 * 1024, (uint8_t *)&ChargingInfo[1], sizeof(CHARGINGINFO));
	OSMutexPost (FramMutex);
}

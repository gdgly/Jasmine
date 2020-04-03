#include "includes.h"
#include "stdint.h"
#include "encrypt.h"
#include "MT625.h"
#include "CardTask.h"
#include "Para.h"
#include "UART.h"
static CARDTASKINFO CardTaskInfo={0};
static uint8_t SetCardFlag;
static uint8_t SetCardType;
static uint8_t SetCardPara[16];
static uint8_t SetCardRet;
 
/************************************************************************************************************
** 函 数 名 : CardGetState
** 功能描述 : 获取卡状态
** 输    入 : 无
** 输    出 : 无
** 返    回 :	1 有刷卡  0 未刷卡
*************************************************************************************************************
*/
uint8_t CardGetState(void)
{
	if (CardTaskInfo.swmark == 1){
		return 1;
	} else {
		return 0;
	}
}

/************************************************************************************************************
** 函 数 名 : CardGetType
** 功能描述 : 获取卡类型 (在线卡 或离线卡)
** 输    入 : 无
** 输    出 : 1 在线卡  2 离线卡
** 返    回 :	无
*************************************************************************************************************
*/
uint8_t CardGetType(void)
{
	return CardTaskInfo.cardinfo.type;
}

/************************************************************************************************************
** 函 数 名 : CardGetId
** 功能描述 : 获取卡ID
** 输    入 : 无
** 输    出 : 无
** 返    回 :	卡号
*************************************************************************************************************
*/
uint32_t CardGetId(void)
{
	return CardTaskInfo.cardinfo.cardID;
}

/************************************************************************************************************
** 函 数 名 : CardSetLockState
** 功能描述 : 设置卡锁状态
** 输    入 : state 1解除锁定 2加锁
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t CardSetLockState(uint32_t id, uint8_t state)
{
	SetCardRet = 0;
	SetCardType = 1;
	SetCardPara[0] = state;
	SetCardFlag = 1;
	while(SetCardFlag == 1)
		OSTimeDlyHMSM (0, 0, 0, 10);	
	return SetCardRet;
}
 


/************************************************************************************************************
** 函 数 名 : CardGetLockState
** 功能描述 : 获取卡锁状态
** 输    入 : 无
** 输    出 : 无
** 返    回 :	1 正常   2 锁定
*************************************************************************************************************
*/
uint8_t CardGetLockState(void)
{
	return CardTaskInfo.cardinfo.lockst;
}

/************************************************************************************************************
** 函 数 名 : CardSetMoney
** 功能描述 : 设置卡余额
** 输    入 : money 精度0.01元
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t CardSetMoney(uint32_t id, uint32_t money)
{
	SetCardRet = 0;
	SetCardType = 2;
	memcpy(SetCardPara, &money, 4);
	SetCardFlag = 1;
	while(SetCardFlag == 1)
		OSTimeDlyHMSM (0, 0, 0, 10);	
	return SetCardRet;
}


/************************************************************************************************************
** 函 数 名 : CardSetMoneyAndUnlock
** 功能描述 : 设置卡余额并解锁
** 输    入 : money 精度0.01元
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t CardSetMoneyAndUnlock(uint32_t id, uint32_t money)
{
	if(id != CardGetId())
		return 0;
	SetCardRet = 0;
	SetCardType = 4;
	memcpy(SetCardPara, &money, 4);
	SetCardFlag = 1;
	while(SetCardFlag == 1)
		OSTimeDlyHMSM (0, 0, 0, 10);	
	return SetCardRet;
}




/************************************************************************************************************
** 函 数 名 : CardGetMoney
** 功能描述 : 获取卡余额
** 输    入 : 无
** 输    出 : 无
** 返    回 :	余额 精度0.01元
*************************************************************************************************************
*/
uint32_t CardGetMoney(void)
{
	return CardTaskInfo.cardinfo.Money;
}

/************************************************************************************************************
** 函 数 名 : CardSetPileNumber
** 功能描述 : 设置卡桩编号
** 输    入 : data 编号 4字节
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t CardSetPileNumber(uint32_t id, uint8_t *data)
{
	SetCardRet = 0;
	SetCardType = 3;
	memcpy(SetCardPara, data, 4);
	SetCardFlag = 1;
	while(SetCardFlag == 1)
		OSTimeDlyHMSM (0, 0, 0, 10);
	return SetCardRet;
}




/************************************************************************************************************
** 函 数 名 : CardSetPileNumberAndLock
** 功能描述 : 设置卡桩编号并加锁
** 输    入 : data 编号 4字节
** 输    出 : 无
** 返    回 :	0失败 1成功
*************************************************************************************************************
*/
uint8_t CardSetPileNumberAndLock(uint32_t id, uint8_t *data)
{
	if(id != CardGetId())
		return 0;
	SetCardRet = 0;
	SetCardType = 5;
	memcpy(SetCardPara, data, 4);
	SetCardFlag = 1;
	while(SetCardFlag == 1)
		OSTimeDlyHMSM (0, 0, 0, 10);
	return SetCardRet;
}



/************************************************************************************************************
** 函 数 名 : CardGetPileNumber
** 功能描述 : 获取卡桩编号
** 输    入 : 无
** 输    出 : data 编号 4字节
** 返    回 :	无
*************************************************************************************************************
*/
void CardGetPileNumber(uint8_t *data)
{
	if (data != 0){
		memcpy(data, CardTaskInfo.cardinfo.PileNumber, 4);
	}
}



/************************************************************************************************************
** 函 数 名 : CardProc
** 功能描述 : 卡处理函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void CardProc(void)
{
	uint32_t UID;

	static uint8_t state;
	uint64_t data64;
	uint8_t CustomNum[4]; 
	
	if (MTSearch(&UID) == 1){
		if (state != 1){
			ParaGetCustomerNumber(CustomNum);
			data64 = CalcKey(*(uint32_t*)CustomNum);
			memcpy(CardTaskInfo.cardinfo.KeyA, &data64, 6);
			if (MTRead(&CardTaskInfo.cardinfo) == 1){
				state = 1;
				CardTaskInfo.swmark = 1;
			}
		}
	} else {
		state = 0;
		CardTaskInfo.cardinfo.cardID = 0;
		CardTaskInfo.swmark = 0;
	}
	
	if (SetCardFlag == 1){
		ParaGetCustomerNumber(CustomNum);
		data64 = CalcKey(*(uint32_t*)CustomNum);
		memcpy(CardTaskInfo.cardinfo.KeyA, &data64, 6);
		if (SetCardType == 1){//解锁
			CardTaskInfo.cardinfo.lockst = SetCardPara[0];
			if (MTWrite(&CardTaskInfo.cardinfo) == 1){
				SetCardRet = 1;
			}
			SetCardFlag = 0;
		} else if (SetCardType == 2){//写金额
			memcpy(&CardTaskInfo.cardinfo.Money , SetCardPara, 4);
			if (MTWrite(&CardTaskInfo.cardinfo) == 1){
				SetCardRet = 1;
			}
			SetCardFlag = 0;
		} else if (SetCardType == 3){//写桩编号
			memcpy(&CardTaskInfo.cardinfo.PileNumber , SetCardPara, 4);
			if (MTWrite(&CardTaskInfo.cardinfo) == 1){
				SetCardRet = 1;
			}
			SetCardFlag = 0;
		} else if (SetCardType == 4){//写金额并解锁
			memcpy(&CardTaskInfo.cardinfo.Money , SetCardPara, 4);
			CardTaskInfo.cardinfo.lockst = UNLOCK;//解锁    
			if (MTWrite(&CardTaskInfo.cardinfo) == 1){
				SetCardRet = 1;
			}
			SetCardFlag = 0;
		}
		else if (SetCardType == 5){  //写桩号并加锁
			memcpy(&CardTaskInfo.cardinfo.PileNumber , SetCardPara, 4);
			CardTaskInfo.cardinfo.lockst = LOCK;//加锁
			if (MTWrite(&CardTaskInfo.cardinfo) == 1){
				SetCardRet = 1;
			}
			SetCardFlag = 0;
		}
	}
}

/************************************************************************************************************
** 函 数 名 : CardTask
** 功能描述 : 卡任务
** 输    入 : pdata 不使用
** 输    出 : 无

** 返    回 :	无
*************************************************************************************************************
*/
void CardTask(void *pdata)
{
	print("card task start...\r\n");
	MTInit();
	while(1){
		CardProc();
		OSTimeDlyHMSM (0, 0, 0, 15);
	}
}



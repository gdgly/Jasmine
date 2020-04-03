#ifndef _CARDTASK_H
#define _CARDTASK_H

#include "MT625.h"

// 任务状态
#define CARD_ST_INIT 		0		// 初始化
#define CARD_ST_READCARD    1		// 读卡
#define CARD_ST_WRITECARD   2		// 写卡

//卡状态
#define UNLOCK              1
#define LOCK                2


//卡类型
#define  TYPE_CARD_ONLINE  	1		// 在线卡
#define  TYPE_CARD_OFFLINE	2		// 离线卡

typedef struct
{
	uint8_t 	state;		    // 状态
	uint8_t 	swmark;		    // 是否有卡刷  0 没有刷卡  1 有刷卡
	uint8_t 	type;		    // 类型  在线卡或离线卡
	uint8_t 	id[10];		    // 卡ID 
	uint32_t    remoney;	    // 余额
	uint8_t     lockst;		    // 锁状态   0 未锁  1 有锁
	uint8_t 	lockctl;        // 控制 1 加锁  2 解锁
	uint32_t  conmoney;         // 消费金额		
	M1CARDVIF cardinfo;         // 卡信息
}CARDTASKINFO;


uint8_t CardSetMoneyAndUnlock(uint32_t id, uint32_t money);
uint8_t CardSetPileNumberAndLock(uint32_t id, uint8_t *data);
uint8_t CardGetState(void);
uint8_t CardGetType(void);
uint32_t CardGetId(void);
uint8_t CardSetLockState(uint32_t id, uint8_t state);
uint8_t CardGetLockState(void);
uint8_t CardSetMoney(uint32_t id, uint32_t money);
uint32_t CardGetMoney(void);
uint8_t CardSetPileNumber(uint32_t id, uint8_t *data);
void CardGetPileNumber(uint8_t *data);
void CardTask(void *pdata);

#endif

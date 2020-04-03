#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_
#include "Cfg.h"
#pragma pack(1)

#define STARTCODE1	0xAA //起始域1
#define STARTCODE2	0xF5 //起始域2
#define MSGCODE			0x10 //信息域

#define COMMBUFFERLEN 1024 //通信缓冲大小

#define CTL_NONE					0 //无操作
#define CTL_STARTCHARGE		1 //启动充电
#define CTL_STOPCHARGE		2 //停止充电

typedef struct{
	uint16_t index;//索引
//	uint8_t ReportState;//上报状态 0未上报 1上报等后台应答 2上报且已得到后台答应
//	uint8_t CardID[32];//充电卡号
//	uint8_t StartDateTime[8];//充电启动时间
//	uint8_t StopDateTime[8];//充电结束时间
}REPORTCTL;

typedef struct{
	uint32_t ReportOnlineCardID;//上报在线卡卡号
	uint8_t ReportFlag;//上报204报文标志 0不需上报 1需上报 2上报已得到回复
	uint8_t ReportRet;//上报204报文收到的结果 0有效帐户 1非法帐户 2余额不足 3把黑卡恢复成正常卡 4把正常卡变黑卡 5非法卡号 6挂失 7销卡 8密码错误 9该卡已经在别的桩使用
	uint32_t OnlineCardIDBalance;//在线卡余额 单位0.01元
	
}REportCMD204;

//通信数据
typedef struct{
	uint8_t buffer[COMMBUFFERLEN];
	uint16_t len;
	uint8_t SendCmd204;             //发送用户查询标志  0停止  1发送	
	uint8_t loginackflag;           //签到应答标志
	uint8_t heartbeatackflag;       //心跳应答标志	
	uint8_t startflag;              //启动充电标志 0无效 1启动
	uint8_t stopflag;               //停止充电标志 0无效 1停止	
	uint8_t AckCMD201Flag;          //平台应答201命令标志，即应答充电桩上报的充电信息报文	
	uint8_t chargestrategy;         //充电策略 0充满为止 1时间 2金额 3电量
	uint32_t chargepara;            //充电参数 时间单位为1秒 金额单位为0.01元 电量单位为0.01
	uint8_t TradeSerialNumber[32];  //交易流水号	
	uint8_t ChargeCardNumber[32];   //充电卡号	
	uint32_t ChargeRecordIndexBak;
	uint32_t ChargeRecordNumberBak;
	uint8_t NeedReportNum;
	uint8_t UserId[2];//用户ID
	uint8_t SendCmd104;             //发送状态信息标志  0停止  1发送
	REPORTCTL ReportCtl[REPORT_NUM];
}COMMINFO;


typedef struct{
	uint8_t UpdataFlag;
	uint16_t UserId;//用户ID
	uint32_t LastMoney;
	uint32_t NewMoney;
}MODIFINFO;


uint8_t GetOnlineState(uint8_t port);   
uint32_t  GetChPara(uint8_t port);
uint8_t  GetChStrategy(uint8_t port);
void GetUserId(uint8_t port, uint8_t* buf);
void TaskProtocolHandle(void *pdata);
void SendCMD204Flay(uint8_t port,uint8_t mark);
void SendCMD104Flay(uint8_t port,uint8_t mark);
void SetUserId(uint8_t port,uint16_t UserId);
uint8_t VerifyOnlineCardInfo(uint8_t port, uint32_t CardID, uint32_t *Balance);
uint32_t UpdateMoneyJudge(uint8_t port, uint16_t UserId, uint32_t RawMoney);
void TaskProtocolHandle(void *pdata);
uint16_t GetUpdataPre(void);
#endif

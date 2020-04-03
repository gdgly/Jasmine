/***********************************************************************************************************
* brief: AC monitor system
* create date: 2018-09-27
* create author: Huang Kong
************************************************************************************************************
* modify date: xxxx-xx-xx
* modify author: xxxxxx
* modify note: xx
************************************************************************************************************
* modify date: xxxx-xx-xx
* modify author: xxxxxx
* modify note: xx
************************************************************************************************************/

#include "app.h"
#include "Protocol.h"
#include "led.h"
#include "RTC.h"
#include "BUZZER.h"
#include "Timer.h"
#include "Record.h"
#include "xrd.h"
#include "MainTask.h"
#include "UART.h"
#include "RELAY.h"
#include "WDG.h"
#include "Para.h"
#include "Board.h"
#include "CardTask.h"
#include "ChgInterface.h"
#include "cfg.h"
#include "Display.h"
#include "ElmTask.h"
#include "LED.h"
#include "Cfg.h"
#include "ethernet.h"

static MAININFO MainInfo={0};

static uint8_t LEDState[2];

static uint8_t SetMainChargeFlag[2];//1启动 2停止
static uint8_t PlatformMode[2];//平台启动时的模式 0充满 1按电量 2按时间 3按金额
static uint32_t PlatformPara[2];//平台启动时的参数 按充满时精度为0.01元 按电量时精度为0.01kWh 按时间时精度为秒 按金额时精度为0.01元
static uint8_t PlatformCardOrDiscernNumber[2][32]; //平台启动时的充电卡号/用户识别号
static uint8_t PlatformUserID[2][2];//用户ID

static uint8_t OneGunState,TwoGunState;//单枪和双枪状态

static uint8_t LifeTimeExpireFlag;//使用期限到期标志 0未到期 1到期
static	uint32_t SerialNum;       //离线卡 交易流水号

static uint8_t IsLogin;  //查看充电记录相关变量
static uint32_t ShowTimer;//查看充电记录相关变量

static	uint8_t AQRCode[256];
static	uint8_t APileNumber[32];
static	uint8_t BQRCode[256];
static	uint8_t BPileNumber[32];

static uint8_t UpdataInFlag; //更新中标志

static uint8_t ReportChargeData[2];    //上报充电记录标志

static uint32_t TimerCntRun; 

static uint8_t StartingChg[2];   //充电启动中标志

/************************************************************************************************************
** 函 数 名 : GetStartType
** 功能描述 : 获取启动方式
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	STARTCHARGETYPE_OFFLINECARD STARTCHARGETYPE_ONLINECARD STARTCHARGETYPE_PLATFORM
*************************************************************************************************************
*/
uint8_t GetStartType(uint8_t port)
{
	return MainInfo.ChgDat[port].StartType;
}



/************************************************************************************************************
** 函 数 名 : GetStartingChg
** 功能描述 : 获取是否是启动中
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	0 不是启动中  1  启动中
*************************************************************************************************************
*/
uint8_t GetStartingChg(uint8_t port)
{
	return StartingChg[port];
}

/************************************************************************************************************
** 函 数 名 : GetStartCardID
** 功能描述 : 获取启动卡ID
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	卡ID
*************************************************************************************************************
*/
uint32_t GetStartCardID(uint8_t port)
{
	return MainInfo.ChgDat[port].StartCardID;
}

/************************************************************************************************************
** 函 数 名 : GetStartMode
** 功能描述 : 获取启动模式
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	0充满 1按电量 2按时间 3按金额
*************************************************************************************************************
*/
uint8_t GetStartMode(uint8_t port)
{
	return MainInfo.ChgDat[port].Mode;
}

/************************************************************************************************************
** 函 数 名 : GetStartPara
** 功能描述 : 获取启动参数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	
*************************************************************************************************************
*/
uint32_t GetStartPara(uint8_t port)
{
	return MainInfo.ChgDat[port].Para;
}

/************************************************************************************************************
** 函 数 名 : GetVoltage
** 功能描述 : 获取电压值
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : A,B,C相电压
** 返    回 :	无
*************************************************************************************************************
*/
void GetVoltage(uint8_t port, uint32_t *VolA, uint32_t *VolB, uint32_t *VolC)
{
	*VolA = MainInfo.GunInfo[port].VoltageA;
	*VolB = MainInfo.GunInfo[port].VoltageB;
	*VolC = MainInfo.GunInfo[port].VoltageC;
}

/************************************************************************************************************
** 函 数 名 : GetCurrent
** 功能描述 : 获取电流值
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : A,B,C相电流
** 返    回 :	无
*************************************************************************************************************
*/
void GetCurrent(uint8_t port, uint32_t *CurA, uint32_t *CurB, uint32_t *CurC)
{
	*CurA = MainInfo.GunInfo[port].CurrentA;
	*CurB = MainInfo.GunInfo[port].CurrentB;
	*CurC = MainInfo.GunInfo[port].CurrentC;
}

/************************************************************************************************************
** 函 数 名 : GetStartMeterEnergy
** 功能描述 : 获取启动时电表电量
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	电量 0.01KWH
*************************************************************************************************************
*/
uint32_t GetStartMeterEnergy(uint8_t port)
{
	return MainInfo.ChgDat[port].StartMeterEnergy;
}

/************************************************************************************************************
** 函 数 名 : GetStopMeterEnergy
** 功能描述 : 获取停止时电表电量
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	电量 0.01KWH
*************************************************************************************************************
*/
uint32_t GetStopMeterEnergy(uint8_t port)
{
	return MainInfo.ChgDat[port].StopMeterEnergy;
}

/************************************************************************************************************
** 函 数 名 : GetSumEnergy
** 功能描述 : 获取累计充电电量
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	电量 0.01KWH
*************************************************************************************************************
*/
uint32_t GetSumEnergy(uint8_t port)
{
	return MainInfo.ChgDat[port].SumEnergy;
}

/************************************************************************************************************
** 函 数 名 : GetSumMoney
** 功能描述 : 获取累计充电金额
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	金额 0.01元
*************************************************************************************************************
*/
uint32_t GetSumMoney(uint8_t port)
{
	return MainInfo.ChgDat[port].SumMoney;
}

/************************************************************************************************************
** 函 数 名 : GetPeriodEnergy
** 功能描述 : 获取时段电量
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	电量 0.01KWH
*************************************************************************************************************
*/
uint16_t GetPeriodEnergy(uint8_t port, uint8_t Period)
{
	return MainInfo.ChgDat[port].PeriodEnergy[Period];
}

/************************************************************************************************************
** 函 数 名 : GetSumTime
** 功能描述 : 获取累计时间
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 : 时间 
*************************************************************************************************************
*/
uint32_t GetSumTime(uint8_t port)
{
	return MainInfo.ChgDat[port].SumTime;
}

/************************************************************************************************************
** 函 数 名 : GetStartDateTime
** 功能描述 : 获取启动时日期时间
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 日期时间
** 返    回 : 无
*************************************************************************************************************
*/
void GetStartDateTime(uint8_t port, uint8_t *datetime)
{
	memcpy(datetime, MainInfo.ChgDat[port].StartDateTime, 6);
}

/************************************************************************************************************
** 函 数 名 : GetStopDateTime
** 功能描述 : 获取停止时日期时间
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 日期时间
** 返    回 : 无
*************************************************************************************************************
*/
void GetStopDateTime(uint8_t port, uint8_t *datetime)
{
	memcpy(datetime, MainInfo.ChgDat[port].StopDateTime, 6);
}

/************************************************************************************************************
** 函 数 名 : GetStartCardMoney
** 功能描述 : 获取启动时卡余额
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 : 余额 0.01元
*************************************************************************************************************
*/
uint32_t GetStartCardMoney(uint8_t port)
{
	return MainInfo.ChgDat[port].StartCardMoney;
}

/************************************************************************************************************
** 函 数 名 : GetCardOrDiscernNumber
** 功能描述 : 获取卡/识别码
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 卡/识别码
** 返    回 : 无
*************************************************************************************************************
*/
void GetCardOrDiscernNumber(uint8_t port, uint8_t *CardOrDiscernNumber)
{
	memcpy(CardOrDiscernNumber, MainInfo.ChgDat[port].CardOrDiscernNumber, 32);
}

/************************************************************************************************************
** 函 数 名 : SetLEDState
** 功能描述 : 设置灯板显示状态
** 输    入 : port state 0空闲 1充电 2故障
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SetLEDState(uint8_t port, uint8_t state)
{
	LEDState[port] = state;
}

/************************************************************************************************************
** 函 数 名 : Beep
** 功能描述 : 蜂鸣器响程序
** 输    入 : ms 响持续时间 毫秒
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void Beep(uint32_t ms)
{
	uint32_t timer;
	
	BuzzerOn();
	timer = TimerRead();
	while(TimerRead() - timer < T10MS * (ms / 10))
		OSTimeDlyHMSM (0, 0, 0, 5);
	BuzzerOff();
}

/************************************************************************************************************
** 函 数 名 : JudgePeriod
** 功能描述 : 判断时间函数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......    time 0-47
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static uint32_t CalPeriodMoney(uint8_t port, uint8_t time)
{
	uint32_t money = 0,i,starttime, stoptime,caltime;
	uint8_t starthour, startmin, stophour, stopmin;
	
	for (i = 0; i < 12; i++){
		ParaGetFeilvTime(port, i, &starthour, &startmin, &stophour, &stopmin);
		if ((starthour != stophour) || (startmin != stopmin)){
			starttime = starthour * 60 + startmin;
			stoptime = stophour * 60 + stopmin;
			caltime = time * 30;
			if ((caltime >= starttime) && (caltime < stoptime)){//前闭后开
				money = ParaGetFeilvMoney(port, i) / 100;
				break; 
			}
		}
	}
	return money;
}

/************************************************************************************************************
** 函 数 名 : GetOnlineCardInfo
** 功能描述 : 获取在线卡信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... CardID 卡号 CardBalance 余额
** 输    出 : 无
** 返    回 :	获取成功1 获取失败0
*************************************************************************************************************
*/
static uint8_t GetOnlineCardInfo(uint8_t port, uint32_t CardID, uint32_t *CardBalance)
{
	uint8_t verifyonlineret;
	uint8_t TmpBuf[48];

	//提示正在验证中，请稍等
	DispInVerify();
	//滴声提示已响应操作
	Beep(100);
	//核实卡信息
	if ((verifyonlineret = VerifyOnlineCardInfo(port, CardID, CardBalance)) == 0){
		return 1;
	} else {
		//滴声提示已响应操作
		Beep(100);
		//提示验证失败
		if (verifyonlineret == 0xff)
			sprintf((char *)TmpBuf, "响应超时");
		else if (verifyonlineret == 2)
			sprintf((char *)TmpBuf, "余额不足");
		else if (verifyonlineret == 5)
			sprintf((char *)TmpBuf, "卡号不存在");
		else if (verifyonlineret == 6)
			sprintf((char *)TmpBuf, "已挂失");
		else if (verifyonlineret == 7)
			sprintf((char *)TmpBuf, "已销卡");
		else if (verifyonlineret == 9)
			sprintf((char *)TmpBuf, "正在其他桩充电");
		else
			sprintf((char *)TmpBuf, "其他原因");
		DispVerifyFailure(TmpBuf);
		OSTimeDlyHMSM (0, 0, 5, 0);
		return 0;
	}
}

/************************************************************************************************************
** 函 数 名 : JudgeLifeTime
** 功能描述 : 判断使用期限
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void JudgeLifeTime(void)
{
	uint32_t LimitHours,  RunHours;
	LimitHours = ParaGetLimitHour();
	
	LifeTimeExpireFlag = 0;
	if( LimitHours ){	
		RunHours = ParaGetRunHour();
		if((TimerRead()- TimerCntRun) >= T1H){
			TimerCntRun = TimerRead();
			RunHours++;
			ParaSavaRunHour(RunHours);
		}
		if(RunHours >= LimitHours )
			LifeTimeExpireFlag = 1;
	}
}


/************************************************************************************************************
** 函 数 名 : GetLifeTime
** 功能描述 : 获取是否租赁到期
** 输    入 : 无
** 输    出 : 无
** 返    回 :	1 到期   0 正常
*************************************************************************************************************
*/
uint8_t GetLifeTime(void)
{
	return LifeTimeExpireFlag;
}

/************************************************************************************************************
** 函 数 名 : ShowRecord
** 功能描述 : 处理显示相关充电记录
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void ShowRecord(void)
{
	CHARGINGDATA ChgData;
	uint8_t Keys[8] = {0}; 
	uint8_t tempbuf[56]={0};
	uint8_t ErrTimes = 0;
	uint16_t KeyFun, index,Index, RecordNum, ShowNum;
	uint32_t Timer,CountDown;
	uint8_t ShowFlag = 0;
	uint8_t ShowDetails = 0;
	uint8_t Page = 0;
	uint8_t SumPage;
	uint8_t Line = 0;
	uint8_t i;

	if(GetTouchInfo(Keys,0,&KeyFun)){
		if(KeyFun == SHOW_LOG){
			if( IsLogin == 0){	
				XRD_Write_Addr(0x748,(char *)"   ", 16);	
		  	XRD_Load_Page(PAGE27);
			}else{
				ShowFlag = 1;
				Page = 1;	
			}
			RecordNum = GetChargeRecordNumber(0);
			index = GetChargeRecordIndex(0);
			Timer = TimerRead();
			CountDown =  60 -((TimerRead() - Timer)/T1S);
			while(CountDown){
				if(GetTouchInfo(Keys, 4, &KeyFun) || IsLogin){
					if(KeyFun == OK_BUTTON){
						KeyFun = 0;
						Timer = TimerRead();
						if(memcmp(Keys,"8888",4) == 0){				
							Page = 1;	
							ShowFlag = 1;			
							IsLogin = 1;									
						}else{
							ErrTimes++;
							sprintf((char *)tempbuf, "密码错误%d", ErrTimes);
							XRD_Write_Addr(0x748,(char *)tempbuf, 16);	
							//提示密码错误
							if(ErrTimes >= 3){
								break;  //退出
							}
						}
					}else if(KeyFun == NEXT_PAGE_BUTTON){   //下一页
						KeyFun = 0;
						Timer = TimerRead();						
						if(Page != 1){
							Page--;	
						}else{
							Page = SumPage;
						}
						ShowFlag = 1;							
					}else if(KeyFun == PRE_PAGR_BUTTON ){		  //上一页
						KeyFun = 0;
						Page++;	
						Timer = TimerRead();
						if(Page > SumPage)	
							Page = 1;
						ShowFlag = 1;						
					}else if(KeyFun == RETURN_BUTTON){   //返回按键
						KeyFun = 0;
						if( Line ){
							Line = 0;
							XRD_Load_Page(PAGE28);							
						}else{
							ShowTimer = TimerRead();	
							break;  //退出
						}					
					}else if(KeyFun == LINE1_BUTTON)	{
						KeyFun = 0;
						Timer = TimerRead();		
						Line = 1;			
						ShowDetails = 1;	
					}else if(KeyFun == LINE2_BUTTON)	{
						KeyFun = 0;
						Timer = TimerRead();	
						Line = 2;						
						ShowDetails = 1;							
					}else if(KeyFun == LINE3_BUTTON)	{
						KeyFun = 0;
						Timer = TimerRead();
						Line = 3;						
						ShowDetails = 1;							
					}else if(KeyFun == LINE4_BUTTON)	{
						KeyFun = 0;
						Timer = TimerRead();	
						Line = 4;						
						ShowDetails = 1;							
					}else if(KeyFun == LINE5_BUTTON){
						KeyFun = 0;
						Timer = TimerRead();
						Line = 5;						
						ShowDetails = 1;						
					}
				}		
				if(ShowDetails){
					ShowDetails = 0;
					ShowNum = RecordNum -((Page-1)*LINENUM)- Line + 1;   //显示的第几条数据
					if(ShowNum == 0)
						continue;
					if( index < (RecordNum - ShowNum) )
						Index =  index + ShowNum ;
					else
						Index = index - (RecordNum - ShowNum);						
					if (FindChargeRecord(Index,  (uint8_t *)&ChgData, sizeof(CHARGINGDATA)) == 1){	
						DispRecordMoreInfo(ChgData.ChgPort, ChgData.StartType, ChgData.StartCardID, ChgData.StartDateTime, ChgData.StopDateTime, ChgData.SumTime, ChgData.SumEnergy, ChgData.StartCardMoney, ChgData.SumMoney, ChgData.StopCause,ChgData.StartSOC,ChgData.StopSOC );			
					}			
				}
				if(ShowFlag){
					ShowFlag = 0;
					if((RecordNum % LINENUM)){
						SumPage =(RecordNum / LINENUM)+1;
					}else{
						SumPage =(RecordNum / LINENUM);
					}		
					if( RecordNum ){
						sprintf((char *)tempbuf, "%02d/%02d", Page, SumPage);
						XRD_Write_Addr(0x750,(char *)tempbuf, 8);				//显示第几页							 
						for(i = 0; i < LINENUM; i++){		
							ShowNum = RecordNum - ((Page - 1) * LINENUM + i);   //显示的第几条数据
							if(ShowNum == 0){
								DispRecordNullInfo(i);
								continue;
							}		
							if( index < (RecordNum - ShowNum) )
								Index =  index + ShowNum ;
							else
								Index = index - (RecordNum - ShowNum);	
							if (FindChargeRecord(Index,  (uint8_t *)&ChgData, sizeof(CHARGINGDATA)) == 1){
								DispRecordInfo(ChgData.StartType, ChgData.StartCardID, ChgData.StartDateTime, ChgData.SumTime, ChgData.SumEnergy, ChgData.SumMoney, ChgData.StopCause,i);
							}else{
								DispRecordNullInfo(i);
							}	
						}
						XRD_Load_Page(PAGE28);	
					}else{
						sprintf((char *)tempbuf, "暂无记录");
						XRD_Write_Addr(0x748,(char *)tempbuf, 16);
						XRD_Load_Page(PAGE27);						
						OSTimeDlyHMSM (0, 0, 3, 0);
						ShowTimer = TimerRead();	
						return;
					}									
				}		
				CountDown =  60 -((TimerRead() - Timer)/T1S);
				sprintf((char *)tempbuf, "%02d ", CountDown);
				XRD_Write_Addr(0x740, (char*)tempbuf, 8);
				OSTimeDlyHMSM (0, 0, 0, 20);		
			}
			XRDClearInput();
			ShowTimer = TimerRead();				
		}
	}
	
}



/************************************************************************************************************
** 函 数 名 : InitStartChgDate
** 功能描述 : 初始化开始充电数据
** 输    入 : Maininfo  充电信息结构体   
**  				:	Mode  0离线卡  3在线卡   1后台   port ==0 A枪   ==1B枪
**  				:	CardId   卡ID  Money == StartCardMoney 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void InitStartChgDate(MAININFO* Info, uint8_t Mode, uint32_t CardId, uint32_t Money, uint8_t port)
{
	Rtc datetime;
	ReportChargeData[port] = 0;
	memset(&Info->ChgDat[port], 0, sizeof(CHARGINGDATA));
	memset(&Info->GunInfo[port], 0, sizeof(GUNINFO));
	Info->ChgDat[port].ChgPort = port;
	RtcRead(&datetime);
	Info->ChgDat[port].StartDateTime[0] = datetime.year % 2000;
	Info->ChgDat[port].StartDateTime[1] = datetime.month;
	Info->ChgDat[port].StartDateTime[2] = datetime.day;
	Info->ChgDat[port].StartDateTime[3] = datetime.hour;
	Info->ChgDat[port].StartDateTime[4] = datetime.min;
	Info->ChgDat[port].StartDateTime[5] = datetime.sec;
	memcpy(Info->ChgDat[port].StopDateTime, Info->ChgDat[port].StartDateTime, 6);
	Info->ChgDat[port].StartMeterEnergy = ElmGetTotalElc(port);
	Info->ChgDat[port].StopMeterEnergy = Info->ChgDat[port].StartMeterEnergy;
	Info->GunInfo[port].StartTimer = TimerRead();
	Info->GunInfo[port].StopTimer = Info->GunInfo[port].StartTimer;
	Info->ChgDat[port].StartSOC = 0;
	Info->ChgDat[port].StopSOC = 0;
	Info->ChgDat[port].StartType = Mode;
	memset(Info->ChgDat[port].VIN, 0, 17);
	Info->ChgDat[port].StartCardID = CardId;
	Info->ChgDat[port].Mode = 0;
	Info->ChgDat[port].Para = 0;
	Info->ChgDat[port].StartCardMoney = Money;
	
	if(Mode == 0){   //  离线卡
		sprintf((char *)Info->ChgDat[port].CardOrDiscernNumber, "%010u", CardId);
		Info->ChgDat[port].SerialNum = SerialNum;		
	}else if(Mode == 3){     //  在线卡
		sprintf((char *)Info->ChgDat[port].CardOrDiscernNumber, "%010u", CardId);			
		Info->ChgDat[port].SerialNum = 0;	
	}else if(Mode == 1){   // 平台
		Info->ChgDat[port].Mode = PlatformMode[port];
		Info->ChgDat[port].Para = PlatformPara[port];
		Info->ChgDat[port].StartCardMoney = Money;
		memcpy(Info->ChgDat[port].CardOrDiscernNumber, PlatformCardOrDiscernNumber[port], 32);
		memcpy(Info->ChgDat[port].UserId, PlatformUserID[port], 2);
		Info->ChgDat[port].StartCardID = PlatformUserID[port][0] | PlatformUserID[port][1] << 8;
		Info->ChgDat[port].SerialNum = 0;
	}
	StartingChg[port] = TRUE;
}


/************************************************************************************************************
** 函 数 名 : UpdateChargeData
** 功能描述 : 更新充电时的相关数据
** 输    入 : ChgData  MAININFO结构体指针   
**  				:	info  CHARGEINFO结构体
**  				:	port ==0 A枪   ==1B枪
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void UpdateChargeData(MAININFO* ChgData, CHARGEINFO info, uint8_t port)
{
	uint32_t data32;
	uint32_t UpdateMoney = 0;
	Rtc datetime;
	uint8_t period, i;
	
			if( ChgData->ChgDat[port].StartType ==  STARTCHARGETYPE_PLATFORM ) {
				UpdateMoney = UpdateMoneyJudge(port, (ChgData->ChgDat[port].UserId[0] | (ChgData->ChgDat[port].UserId[1] << 8)),  ChgData->ChgDat[port].StartCardMoney);
				if( UpdateMoney ){
					if(ChgData->ChgDat[port].Mode == 3){ //按金额
						ChgData->ChgDat[port].Para = UpdateMoney;
					}
					ChgData->ChgDat[port].StartCardMoney = UpdateMoney;
					InsertChargingRecord(port, (uint8_t *)&ChgData->ChgDat[port], sizeof(CHARGINGDATA));
				}
			}
			//计算充电中数据
			ChgData->GunInfo[port].VoltageA = ElmGetVolA(port);
			ChgData->GunInfo[port].VoltageB = ElmGetVolB(port);
			ChgData->GunInfo[port].VoltageC = ElmGetVolC(port);
			ChgData->GunInfo[port].CurrentA = ElmGetCurA(port);
			ChgData->GunInfo[port].CurrentB = ElmGetCurB(port);
			ChgData->GunInfo[port].CurrentC = ElmGetCurC(port);
			data32 = ElmGetTotalElc(port);
			if (data32 != ChgData->ChgDat[port].StopMeterEnergy){
				RtcRead(&datetime);
				period = (datetime.hour * 60 + datetime.min) / 30;
				ChgData->ChgDat[port].PeriodEnergy[period] += data32 - ChgData->ChgDat[port].StopMeterEnergy;
			}
			ChgData->ChgDat[port].StopMeterEnergy = data32;
			ChgData->GunInfo[port].StopTimer = TimerRead();
			if (info.ChargeState == 1){
				ChgData->ChgDat[port].StartSOC = 0;//info[0].BatterySOC / 10;
				ChgData->ChgDat[port].StopSOC = 0;//info[0].SOC;
				//memcpy(MainInfo.ChgDat[0].VIN, info[0].VIN, 17);
				memset(ChgData->ChgDat[port].VIN, 0, 17);
			}
			ChgData->ChgDat[port].SumTime = (ChgData->GunInfo[port].StopTimer - ChgData->GunInfo[port].StartTimer) / T1S;
			data32 = 0;
			for (i = 0; i <  48; i++)
				data32 += ChgData->ChgDat[port].PeriodEnergy[i] * CalPeriodMoney(port, i);
			ChgData->ChgDat[port].SumMoney = (data32 + 900)/ 1000;//始终加0.009元，补后台的服务费和充电费分开的问题
			RtcRead(&datetime);
			ChgData->ChgDat[port].StopDateTime[0] = datetime.year % 2000;
			ChgData->ChgDat[port].StopDateTime[1] = datetime.month;
			ChgData->ChgDat[port].StopDateTime[2] = datetime.day;
			ChgData->ChgDat[port].StopDateTime[3] = datetime.hour;
			ChgData->ChgDat[port].StopDateTime[4] = datetime.min;
			ChgData->ChgDat[port].StopDateTime[5] = datetime.sec;
			if ( ChgData->ChgDat[port].SumEnergy != (ChgData->ChgDat[port].StopMeterEnergy - ChgData->ChgDat[port].StartMeterEnergy)){
				ChgData->ChgDat[port].SumEnergy = ChgData->ChgDat[port].StopMeterEnergy - ChgData->ChgDat[port].StartMeterEnergy;
				InsertChargingRecord(port, (uint8_t *)&ChgData->ChgDat[port], sizeof(CHARGINGDATA));
			}
}










/************************************************************************************************************
** 函 数 名 : ShowMenu
** 功能描述 : 显示 菜单栏相关信息
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void ShowMenu(void)
{
	uint8_t timebuf[32] = {0};
	uint8_t linebuf[32] = {0};
	Rtc DateAndTim;
	
	RtcRead(&DateAndTim);
	sprintf((char *)timebuf,"%02d:%02d",DateAndTim.hour,DateAndTim.min);

	if(ParaGetChgGunNumber()== 1 ){
		if (GetOnlineState(0) == 1)
			sprintf((char *)linebuf, "在线");
		else
			sprintf((char *)linebuf, "   ");
	}else{
		if((GetOnlineState(0) == 1) && (GetOnlineState(1) == 1)){
			sprintf((char *)linebuf, "在线");
		} else if( GetOnlineState(0) == 1){
			sprintf((char *)linebuf, "A在线");
		}	else if( GetOnlineState(1) == 1){
			sprintf((char *)linebuf, "B在线");
		}else {
			sprintf((char *)linebuf, "   ");
		}		
	}
	DispMenuInfo(timebuf, linebuf );		
}


/************************************************************************************************************
** 函 数 名 : UnLockCardPro
** 功能描述 : 处理锁卡相关流程
** 输    入 : Cardid   卡ID   Money  写的金额
** 输    出 : 无
** 返    回 :	1 解锁写金额成功  0 失败
*************************************************************************************************************
*/
static uint8_t UnLockCardPro(uint32_t cardid, uint32_t Money)
{
	uint32_t Timer;
	
	if (CardSetMoneyAndUnlock(cardid, Money)){//解锁卡并扣费成功
		return 1;
	}
	//显示操作中界面		
	DispOperationInfo();
	Timer = TimerRead();
	while((TimerRead() - Timer) < T1S * 10){
		OSTimeDlyHMSM (0, 0, 0, 20);
		if (CardSetMoneyAndUnlock(cardid, Money)){//解锁卡并扣费成功
			while((TimerRead() - Timer) < T1S * 2){
				OSTimeDlyHMSM (0, 0, 0, 20);
			}
			return 1;
		}		
	}
	return 0;
}




/************************************************************************************************************
** 函 数 名 : LockedCardHandle
** 功能描述 : 处理锁卡相关操作
** 输    入 : cardid   卡ID   money  写的金额
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void LockedCardHandle(uint32_t cardid, uint32_t money)
{
	uint8_t TmpBuf[257],PileNumber[32];
	uint8_t ret;
	uint32_t i,SerialNum,index;
	CHARGINGDATA chargingdata;

	memset(TmpBuf, 0, sizeof(TmpBuf));
	memset(PileNumber, 0, sizeof(PileNumber));
	ParaGetPileNo(0, TmpBuf);
	for (i = 0; i < 4; i++)
		PileNumber[i] = TmpBuf[strlen((const char *)TmpBuf) - 4 + i];
	memset(TmpBuf, 0, sizeof(TmpBuf));
	CardGetPileNumber(TmpBuf);
	if ((memcmp(TmpBuf, PileNumber, 4) == 0) || (FindLockCardRecord(cardid, &SerialNum) == 1)){//该卡被本桩锁住
		ret = 0;
		if (FindLockCardRecord(cardid, &SerialNum) == 1){
			index = GetChargeRecordIndex(0);
			for (i = 0; i < 100; i++){//从最近100条记录里面查找
				if (FindChargeRecord(index, (uint8_t *)&chargingdata, 1000) == 1){
					if (chargingdata.SerialNum == SerialNum){
						ret = 1;
						break;
					}
				}
				if (index == 0)
					index = RECORD_TOTAL_NUM;
				else
					index --;
			}
		}
		if (ret == 1){//记录查询成功
			if (money > chargingdata.SumMoney)
				money = money - chargingdata.SumMoney;
			else
				money = 0;		
			
			if(UnLockCardPro(cardid, money)){
				Beep(100);
				DeleteLockCardRecord(cardid);
				OSTimeDlyHMSM (0, 0, 2, 0);
				DispAccount(chargingdata.SumEnergy, chargingdata.SumMoney, money, chargingdata.SumTime, chargingdata.StopDateTime);
				OSTimeDlyHMSM (0, 0, 5, 0);
			}else{
				Beep(100);
				DispUnLockFailureInfo(UNLOCKCARDFAILCODE_WRITECARD);
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}
		} else {//找不到锁卡记录
			Beep(100);
			DispUnLockFailureInfo(UNLOCKCARDFAILCODE_NORECORD);
			OSTimeDlyHMSM (0, 0, 5, 0);
		}
	} else {//提示其他桩锁住
		Beep(100);	
		DispCardLockInfo(TmpBuf);
		OSTimeDlyHMSM (0, 0, 5, 0);
	}
}




/************************************************************************************************************
** 函 数 名 : LockCardPro
** 功能描述 : 处理锁卡相关流程
** 输    入 : Cardid   卡ID
** 输    出 : 无
** 返    回 :	1 锁卡成功  0 失败
*************************************************************************************************************
*/
static uint8_t LockCardPro(uint32_t Cardid)
{
	uint8_t PileNumber[32];
	uint8_t TmpBuf[56];
	uint8_t i;
	uint32_t Timer;
	Timer = TimerRead();
	memset(TmpBuf, 0, sizeof(TmpBuf));
	memset(PileNumber, 0, sizeof(PileNumber));
	ParaGetPileNo(0, TmpBuf);
	for (i = 0; i < 4; i++)
		PileNumber[i] = TmpBuf[strlen((const char *)TmpBuf) - 4 + i];
	if( CardSetPileNumberAndLock(Cardid, PileNumber) == 1){
		SerialNum = InsertLockCardRecord(Cardid );			//存储锁卡记录		
		return 1;
	}
	//显示操作中界面		
	DispOperationInfo();
	while((TimerRead() - Timer) < T1S * 10){
		OSTimeDlyHMSM (0, 0, 0, 20);
		if( CardSetPileNumberAndLock(Cardid, PileNumber) == 1){
			SerialNum = InsertLockCardRecord(Cardid );			//存储锁卡记录	
			while((TimerRead() - Timer) < T1S * 2){
				OSTimeDlyHMSM (0, 0, 0, 20);
			}
			return 1;
		}
	}
	return 0;
}




/************************************************************************************************************
** 函 数 名 : ConditionStop
** 功能描述 : 判断充电是否自动停止
** 输    入 : MainInf  充电相关结构体   port  0 ==A枪    1 ==B枪 
** 输    出 : 无
** 返    回 :	1 停止  0 正常
*************************************************************************************************************
*/
static uint8_t ConditionStop( MAININFO* MainInf ,uint8_t port)
{
	if ((MainInf->ChgDat[port].SumMoney + MINIMUM_CHARGE_MONEY) > MainInf->ChgDat[port].StartCardMoney){//按金额
		MainInf->ChgDat[port].StopCause = CAUSE_MONEY_FAULT;
		MainInf->ChgDat[port].SumMoney = MainInf->ChgDat[port].StartCardMoney;
		return 1;	
	}else if ((MainInf->ChgDat[port].Mode == 1) && (MainInf->ChgDat[port].SumEnergy >= MainInf->ChgDat[port].Para)){//按电量
		MainInf->ChgDat[port].StopCause = CAUSE_USER_NORMAL_STOP;
		return 1;
	}else if ((MainInf->ChgDat[port].Mode == 2) && (MainInf->ChgDat[port].SumTime >= MainInf->ChgDat[port].Para)){//按时间
		MainInf->ChgDat[port].StopCause = CAUSE_USER_NORMAL_STOP;
		return 1;
	}else if ((MainInf->ChgDat[port].Mode == 3) && (MainInf->ChgDat[port].SumMoney >= MainInf->ChgDat[port].Para)){//按金额
		MainInf->ChgDat[port].StopCause = CAUSE_USER_NORMAL_STOP;
		return 1;
	
	}else if (ElmGetCommState(port) == 1){
		MainInf->ChgDat[port].StopCause = CAUSE_ELM_FAULT;
		return 1;
	}else
		return 0;
}



/************************************************************************************************************
** 函 数 名 : OneGunHandle
** 功能描述 : 单枪处理函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void OneGunHandle(void)
{
	CHARGEINFO info[1];
	uint8_t TmpBuf[257],*pTmpBuf,PileNumber[32];
	static uint8_t cardinflag;
	uint8_t SwingOfflineCardFlag = 0;//刷离线卡标志 0未刷卡 1刷卡
	uint8_t SwingOnlineCardFlag = 0;//刷在线卡标志 0未刷卡 1刷卡
	uint8_t CardLockState;//卡锁状态 1正常 2锁卡
	uint32_t CardID;//卡号
	uint32_t CardBalance;//卡余额 2位小数
	uint8_t ret;
	uint32_t money;
	uint8_t QRCode[256];
	uint16_t Updata;
	StartingChg[PORTA] = FALSE;
	//获取卡信息
	if (CardGetState() == 1){
		if (cardinflag != 1){
			cardinflag = 1;
			CardID = CardGetId();
			CardBalance = CardGetMoney();
			CardLockState = CardGetLockState();
			if (CardGetType() == 1)
				SwingOfflineCardFlag = 1;
			else
				SwingOnlineCardFlag = 1;
		}
	} else {
		cardinflag = 0;
	}

	//查看记录时间处理
	if(IsLogin){
		if((TimerRead() - ShowTimer) > (T1S*60))
			IsLogin = 0;
	}
	ShowMenu();
	//计算使用期限
	JudgeLifeTime();
	ShowRecord();
	switch(OneGunState){
		case A_IDLE://空闲---------------------------------------------------------------------------------------------------------------------------------
			info[0] = GetChgInfo(0);
			Updata = GetUpdataPre();
			if(( Updata>> 8) == 1){   //升级中
				if(UpdataInFlag == 0)
					Beep(100);
				UpdataInFlag = 1;
				DispUpdataInfo(Updata&0xFF,(uint8_t* )"  ");
			}else if(UpdataInFlag){
				 //显示升级失败
				Beep(100);
				UpdataInFlag = 0;
				DispUpdataInfo(Updata&0xFF,(uint8_t* )"升级失败");
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if ((info[0].ScramState == 1) || (info[0].DoorState == 1) || (info[0].CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (info[0].ErrTemp==1)|| (info[0].HighVol == 1)|| (info[0].LowVol == 1)){//充电桩故障
				SetLEDState(0, 2);
				memset(TmpBuf, 0, sizeof(TmpBuf));
				pTmpBuf = TmpBuf;
				if (info[0].ScramState == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_SCRAM);
				if (info[0].DoorState == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_DOOR);
				if (info[0].CommunicateState == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_CHARGEBOARD_COMM);
				if (ElmGetCommState(0) == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_METER_COMM);
				if (LifeTimeExpireFlag == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_EXPIRE_COMM);
				if (info[0].ErrTemp == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_TEMP_ERR);
				if (info[0].HighVol == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_HIGHVOL);
				if (info[0].LowVol == 1)
					pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_LOWVOL);			
				DispDeviceFault(TmpBuf);
				if (SwingOfflineCardFlag == 1){   //A待机B待机刷离线卡
					SwingOfflineCardFlag = 0;
					if (CardLockState == 1){//正常	
						//滴声提示已响应操作
						Beep(100);
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);	
					}
					if (CardLockState == 2){//灰锁卡
						LockedCardHandle(CardID, CardBalance);
					}
				}
				if(SwingOnlineCardFlag == 1){
					SwingOnlineCardFlag = 0;
					//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}
				
			} else {//充电桩正常
				//更新指示灯显示状态
				SetLEDState(0, 0);
				
				//更新显示屏数据
				ParaGetQRCode(0, QRCode);
				ParaGetPileNo(0, PileNumber);
				strcat((char* )PileNumber,"A");
				DispAIdleInfo(QRCode, PileNumber, info[0].GunSeatState);
				
					//判断并处理刷离线卡操作
				if (SwingOfflineCardFlag == 1){//刷离线卡
					SwingOfflineCardFlag = 0;
				
					if (CardLockState == 1){//正常卡
						if (info[0].GunSeatState == 0){//未插枪
							//滴声提示已响应操作
							Beep(100);
							
							//提示未插枪
							DispInsertGunInfo();
							OSTimeDlyHMSM (0, 0, 5, 0);
							
						} else {						
							if( LockCardPro(CardID)){
								//滴声提示已响应操作
								Beep(100);
								//初始化枪数据								
								InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, 0);
								InsertChargingRecord(0, (uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));	
								
								//显示卡余额
								DispRemainMoneyInfo(CardBalance);
								OSTimeDlyHMSM (0, 0, 2, 0);	
								
								//显示启动中界面
								DispStartChgInfo(0);
								OSTimeDlyHMSM (0, 0, 0, 500);
								
								//启动界面
								ret = StartCharge(0);
								if (ret == 0){//启动成功
									OneGunState = A_CHARGE;
								} else {
									MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
									//生成充电记录,并删除充电过程数据
									ReportChargeData[0] = 1;
									InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
									DeleteChargingRecord(0);
									OneGunState = A_STOP;
									DispStartFailureInfo(); //显示启动失败
									OSTimeDlyHMSM (0, 0, 2, 0);
								}
							}else{
								DispStartFailureInfo();
								OSTimeDlyHMSM (0, 0, 5, 0);
							}		
						}
					}
					if (CardLockState == 2){//灰锁卡
						LockedCardHandle(CardID, CardBalance);
					}
				}	else if (SwingOnlineCardFlag == 1){//刷在线卡
					SwingOnlineCardFlag = 0;	
					if (info[0].GunSeatState == 0){//未插枪
						//滴声提示已响应操作
						Beep(100);
						//提示未插枪
						DispInsertGunInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
						
					} else {
						//获取在线卡信息
						if (GetOnlineCardInfo(0, CardID, &CardBalance) == 1){
							//初始化枪数据
							InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, 0);
							InsertChargingRecord(0, (uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));						
							//显示卡余额
							DispVerifySuccesInfo(CardBalance );
							OSTimeDlyHMSM (0, 0, 2, 0);							
							//先提示启动充电中界面
							DispStartChgInfo(0);
							OSTimeDlyHMSM (0, 0, 0, 500);						
							//启动充电
							ret = StartCharge(0);
							if (ret == 0){//启动成功
								OneGunState = A_CHARGE;
							} else {
								MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
								//生成充电记录,并删除充电过程数据
								ReportChargeData[0] = 1;
								InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
								DeleteChargingRecord(0);
								OneGunState = A_STOP;
							}
						}else {
						
						}
					}
				} else if (SetMainChargeFlag[0] == 1) {//后台启动     				//判断并处理后台操作
					SetMainChargeFlag[0] = 0;				
					if (info[0].GunSeatState == 0){//未插枪
						//滴声提示已响应操作
						Beep(100);
						//提示未插枪
						DispInsertGunInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);		
					} else {
						//滴声提示已响应后台操作
						Beep(100);
						//初始化枪数据
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[0], 0);
						InsertChargingRecord(0, (uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));						
						//显示余额
						DispRemainMoneyInfo(PlatformPara[0]);
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//先提示启动充电中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);				
						//启动充电
						ret = StartCharge(0);
						if (ret == 0){//启动成功
							OneGunState = A_CHARGE;
						} else {
							MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[0] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
							DeleteChargingRecord(0);
							OneGunState = A_STOP;
						}
					}
					
				}
					
			}
			break;
		
		case A_CHARGE://充电---------------------------------------------------------------------------------------------------------------------------------
			info[0] = GetChgInfo(0);
			//设置指示灯状态
			if ((info[0].ScramState == 1) || (info[0].DoorState == 1) || (info[0].CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (info[0].HighVol == 1)|| (info[0].LowVol == 1)){//充电桩故障
				SetLEDState(0, 2);
			} else {//充电桩正常
				SetLEDState(0, 1);
			}	
			UpdateChargeData(&MainInfo, info[0], 0);
			//更新显示屏数据
			DispAChargeInfo(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, MainInfo.GunInfo[0].CurrentA, MainInfo.GunInfo[0].CurrentB, MainInfo.GunInfo[0].CurrentC,\
																																								 MainInfo.GunInfo[0].VoltageA, MainInfo.GunInfo[0].VoltageB,  MainInfo.GunInfo[0].VoltageC,  MainInfo.ChgDat[0].SumTime);
		
			//判断并处理刷离线卡操作
			if (SwingOfflineCardFlag == 1){//刷离线卡
				SwingOfflineCardFlag = 0;
			
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//刷卡停止
					//解锁
					money = 0;
					if (CardBalance > MainInfo.ChgDat[0].SumMoney)
						money = CardBalance - MainInfo.ChgDat[0].SumMoney;
					if(UnLockCardPro(CardID, money)){
						DeleteLockCardRecord(CardID);	
						//滴声提示已响应操作
						Beep(100);
						//停止充电
						StopCharge(0);
						MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;		
						//生成充电记录,并删除充电过程数据
						ReportChargeData[0] = 1;
						InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
						DeleteChargingRecord(0);
						//设置状态
						OneGunState = A_IDLE;
						//显示结算界面
						DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
						OSTimeDlyHMSM (0, 0, 8, 0);
					}else{			
						Beep(100);
						//停止充电
						StopCharge(0);
						MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;
						//设置状态
						OneGunState = A_STOP;					
						//生成充电记录,并删除充电过程数据
						ReportChargeData[0] = 1;
						InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
						DeleteChargingRecord(0);
					}
				} else if(CardLockState == 2){//是被锁定的卡
					//进入解锁卡程序
					LockedCardHandle(CardID, CardBalance);
					
				} else {
					//滴声提示已响应操作
					Beep(100);			
					//提示无空闲车位
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}
			
			//判断并处理刷在线卡操作
			if (SwingOnlineCardFlag == 1){//刷在线卡
				SwingOnlineCardFlag = 0;
				
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//刷卡停止
					//计算余额
					money = 0;
					if (MainInfo.ChgDat[0].StartCardMoney > MainInfo.ChgDat[0].SumMoney)
						money = MainInfo.ChgDat[0].StartCardMoney - MainInfo.ChgDat[0].SumMoney;
					
					//滴声提示已响应操作
					Beep(100);
					
					//停止充电
					StopCharge(0);
					MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;
					//生成充电记录,并删除充电过程数据
					ReportChargeData[0] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
					DeleteChargingRecord(0);					
					//设置状态
					OneGunState = A_IDLE;
					//显示结算界面
					DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else {
					//滴声提示已响应操作
					Beep(100);			
					//提示无空闲车位
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}
			
			//判断并处理后台停止操作
			if ((SetMainChargeFlag[0] == 2) && (MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_PLATFORM)){//后台停止
				SetMainChargeFlag[0] = 0;			
				//计算余额
				money = MainInfo.ChgDat[0].StartCardMoney - MainInfo.ChgDat[0].SumMoney;				
				//峰鸣器响提示后台操作已被处理
				Beep(100);				
				//停止充电
				StopCharge(0);
				MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;					
				//生成充电记录,并删除充电过程数据
				ReportChargeData[0] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
				DeleteChargingRecord(0);
				//设置为空闲状态
				OneGunState = A_IDLE;						
				//显示结算界面
				DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}
			if (ConditionStop(&MainInfo, 0) == 1){//达到条件
				//峰鸣器响
				Beep(100);				
				//停止充电
				StopCharge(0);
				//生成充电记录,并删除充电过程数据
				ReportChargeData[0] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
				DeleteChargingRecord(0);				
				//设置状态
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD))
					OneGunState = A_STOP;
				else
					OneGunState = A_IDLE;
			}		
			//判断A枪充电是否自动停止
			if (info[0].ChargeState != 1){
					//收集停止原因
				MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
				//生成充电记录,并删除充电过程数据
				ReportChargeData[0] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
				DeleteChargingRecord(0);
				//设置状态
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD))
					OneGunState = A_STOP;
				else
					OneGunState = A_IDLE;
			}			
			break;
			
		case A_STOP://停止---------------------------------------------------------------------------------------------------------------------------------
			//获取充电板信息
			info[0] = GetChgInfo(0);
			//设置指示灯状态
			if ((info[0].ScramState == 1) || (info[0].DoorState == 1) || (info[0].CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (info[0].HighVol == 1)|| (info[0].LowVol == 1)){//充电桩故障
				SetLEDState(0, 2);
			} else {//充电桩正常
				SetLEDState(0, 0);
			}
			//更新显示屏数据
			if( MainInfo.ChgDat[0].StartType ==  STARTCHARGETYPE_PLATFORM)
				DispAStopInfo((uint8_t *)"  ", (uint8_t *)stopcausetab[MainInfo.ChgDat[0].StopCause], MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney);		
			else
				DispAStopInfo((uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[0].StopCause], MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney);		
		
			
			if (info[0].GunSeatState == 0) {//未插枪
				//设置状态
				OSTimeDlyHMSM (0, 0, 5, 0);
				OneGunState = A_IDLE;
			} 			
			//判断并处理刷离线卡操作
			if (SwingOfflineCardFlag == 1){//刷离线卡
				SwingOfflineCardFlag = 0;			
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//刷卡停止
					//解锁
					money = 0;
					if (CardBalance > MainInfo.ChgDat[0].SumMoney)
						money = CardBalance - MainInfo.ChgDat[0].SumMoney;
					if(UnLockCardPro(CardID, money)){
						Beep(100);					
						//删除存储器里面的锁卡信息
						DeleteLockCardRecord(CardID);			
						//显示结算信息
						DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
						OSTimeDlyHMSM (0, 0, 8, 0);
						//设置状态
						OneGunState = A_IDLE;
					}else{
//						OSTimeDlyHMSM (0, 0, 2, 0);
					}
				} else if(CardLockState == 2){//是被锁定的卡
					//进入解锁卡程序
					LockedCardHandle(CardID, CardBalance);
				
				} else {
					//滴声提示已响应操作
					Beep(100);			
					//提示无空闲车位
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}		
			//判断并处理刷在线卡操作
			if (SwingOnlineCardFlag == 1){//刷在线卡
				SwingOnlineCardFlag = 0;		
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//刷卡停止
					//计算余额
					money = 0;
					if (MainInfo.ChgDat[0].StartCardMoney > MainInfo.ChgDat[0].SumMoney)
						money = MainInfo.ChgDat[0].StartCardMoney - MainInfo.ChgDat[0].SumMoney;						
					//滴声提示已响应操作
					Beep(100);
					//设置状态
					OneGunState = A_IDLE;			
					//显示结算界面
					DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				} else {
					//滴声提示已响应操作
					Beep(100);			
					//提示无空闲车位
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}			
			break;
	}
}


/************************************************************************************************************
** 函 数 名 : JudgeErrInfo
** 功能描述 : 判断是否发生故障 并写入相关显示内容
** 输    入 : CHARGEINFO   充电信息结构体  port == 0A    ===1B
** 输    出 : 1发生故障
** 返    回 :	无
*************************************************************************************************************
*/

static uint8_t JudgeErrInfo(CHARGEINFO Info, uint8_t port)
{

	uint8_t TmpBuf[56];
	uint8_t *pTmpBuf;
	if ((Info.ScramState == 1) || (Info.DoorState == 1) || (Info.CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (Info.HighVol == 1)|| (Info.LowVol == 1)){//充电桩故障
		memset(TmpBuf, 0, sizeof(TmpBuf));
		pTmpBuf = TmpBuf;
		pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "故障:");
		if (Info.ScramState == 1)
			pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_SCRAM);
		if (Info.DoorState == 1)
			pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_DOOR);
		if (Info.CommunicateState == 1)
			pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_CHARGEBOARD_COMM);
		if (ElmGetCommState(port) == 1)
			pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_METER_COMM);
		if (LifeTimeExpireFlag == 1)
			pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_EXPIRE_COMM);
		if (Info.HighVol == 1)
			pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_HIGHVOL);
		if (Info.LowVol == 1)
			pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "%02d ", FAULTCODE_LOWVOL);				
		DispErrIdGunInfo(port, TmpBuf);
		return 1;
	}else{
		DispErrIdGunInfo(port, (uint8_t *)"  ");
		return 0;
	} 
}






/************************************************************************************************************
** 函 数 名 : TwoGunHandle
** 功能描述 : 双枪处理函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/


static void TwoGunHandle(void)
{
	CHARGEINFO info[2];
	uint8_t port = 0;
	uint32_t money;
	static uint8_t cardinflag;
	uint8_t SwingOfflineCardFlag = 0;//刷离线卡标志 0未刷卡 1刷卡
	uint8_t SwingOnlineCardFlag = 0;//刷在线卡标志 0未刷卡 1刷卡
	uint8_t CardLockState;//卡锁状态 1正常 2锁卡
	uint32_t CardID;//卡号
	uint32_t CardBalance;//卡余额 2位小数
	uint8_t ret;
	uint8_t ErrGun[2];
	uint16_t Updata;
	StartingChg[PORTA] = FALSE;
	StartingChg[PORTB] = FALSE;
	//获取卡信息
	if (CardGetState() == 1){
		if (cardinflag != 1){
			cardinflag = 1;
			CardID = CardGetId();
			CardBalance = CardGetMoney();
			CardLockState = CardGetLockState();
			if (CardGetType() == 1)
				SwingOfflineCardFlag = 1;
			else
				SwingOnlineCardFlag = 1;
		}
	} else {
		cardinflag = 0;
	}
	
	//查看记录时间处理
	if(IsLogin){
		if((TimerRead() - ShowTimer) > (T1S*60))
			IsLogin = 0;
	}
	//显示菜单框信息
	ShowMenu();
	ShowRecord();
	//计算使用期限
	JudgeLifeTime();
		//获取充电板数据
	info[PORTA] = GetChgInfo(PORTA);
	info[PORTB] = GetChgInfo(PORTB);
	//设置指示灯状态
	ErrGun[PORTA] = JudgeErrInfo(info[PORTA], PORTA);
	ErrGun[PORTB] = JudgeErrInfo(info[PORTB], PORTB);
	//A空闲 B空闲--------------------------------------------------------------------------------------------------------------------------------------------
	if (TwoGunState == A_IDLE_B_IDLE){
		Updata = GetUpdataPre();
		if(( Updata>> 8) == 1){
			if(UpdataInFlag == 0)
				Beep(100);
			UpdataInFlag = 1;
			DispUpdataInfo(Updata&0xFF,(uint8_t* )"  ");
			
		}else if(UpdataInFlag){
			 //显示升级失败
			Beep(100);
			UpdataInFlag = 0;
			DispUpdataInfo(Updata&0xFF,(uint8_t* )"升级失败");
			OSTimeDlyHMSM (0, 0, 5, 0);	
		}else{
			DispAIdleBIdleInfo(AQRCode, APileNumber, info[PORTA].GunSeatState, BQRCode, BPileNumber, info[PORTB].GunSeatState);
			SetLEDState(PORTA, 0);
			SetLEDState(PORTB, 0);
			if(ErrGun[PORTA]){
				SetLEDState(PORTA, 2);
			}
			if(ErrGun[PORTB]){
				SetLEDState(PORTB, 2);
			}	
			if (SwingOfflineCardFlag == 1){   //A待机B待机刷离线卡
				SwingOfflineCardFlag = 0;
				if (CardLockState == 1){//正常			
					if ((info[0].GunSeatState == 0) && (info[1].GunSeatState == 0 ) && ((ErrGun[PORTA] + (ErrGun[PORTB]) != 2)) ){//未插枪		
						//滴声提示已响应操作
						Beep(100);
						//提示未插枪
						DispInsertGunInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);			
					} else if( ((ErrGun[PORTA] == 1) && (ErrGun[PORTB] == 1) ) ||((ErrGun[PORTA] == 1)&& (info[PORTA].GunSeatState == 1) && (info[PORTB].GunSeatState == 0))||((ErrGun[PORTB] == 1)&& (info[PORTB].GunSeatState == 1) && (info[PORTA].GunSeatState == 0)) ){	
						//滴声提示已响应操作
						Beep(100);
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);	
					}else { 
						if((info[PORTA].GunSeatState == 1) && (ErrGun[PORTA] == 0))//默认从A枪开始
							port = PORTA;
						else if ((info[PORTB].GunSeatState == 1)&& (ErrGun[PORTB] == 0))
							port = PORTB;	
						if( LockCardPro(CardID)){
							//滴声提示已响应操作
							Beep(100);
							//初始化枪数据								
							InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, port);
							InsertChargingRecord(port, (uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));	
							
							//显示卡余额
							DispRemainMoneyInfo(CardBalance);
							OSTimeDlyHMSM (0, 0, 2, 0);	
							
							//显示启动中界面
							DispStartChgInfo(0);
							OSTimeDlyHMSM (0, 0, 0, 500);
							
							//启动界面
							ret = StartCharge(port);
							if (ret == 0){//启动成功
								if (port == PORTA)
									TwoGunState = A_CHARGE_B_IDLE;
								else
									TwoGunState = A_IDLE_B_CHARGE;
							} else {
								MainInfo.ChgDat[port].StopCause = GetStopCause(port);//GetCCBStopCause(0);
								//生成充电记录,并删除充电过程数据
								ReportChargeData[port] = 1;
								InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));
								DeleteChargingRecord(port);
								if (port == PORTA)
									TwoGunState = A_STOP_B_IDLE;
								else
									TwoGunState = A_IDLE_B_STOP;
								DispStartFailureInfo(); //显示启动失败
								OSTimeDlyHMSM (0, 0, 2, 0);
							}
						}else{
							DispStartFailureInfo();
							OSTimeDlyHMSM (0, 0, 5, 0);
						}		
					}
				}
				
				if (CardLockState == 2){//灰锁卡
					LockedCardHandle(CardID, CardBalance);
				}
			}else if (SwingOnlineCardFlag == 1){////A待机B待机刷在线卡
				SwingOnlineCardFlag = 0;	
				if ((info[0].GunSeatState == 0) && (info[1].GunSeatState == 0 ) && ((ErrGun[PORTA] + (ErrGun[PORTB]) != 2)) ){//未插枪		
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				} else if( ((ErrGun[PORTA] == 1) && (ErrGun[PORTB] == 1) ) ||((ErrGun[PORTA] == 1)&& (info[PORTA].GunSeatState == 1) && (info[PORTB].GunSeatState == 0))||((ErrGun[PORTB] == 1)&& (info[PORTB].GunSeatState == 1) && (info[PORTA].GunSeatState == 0)) ){	
					//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}	else {
					if((info[PORTA].GunSeatState == 1) && (ErrGun[PORTA] == 0))//默认从A枪开始
						port = PORTA;
					else if ((info[PORTB].GunSeatState == 1)&& (ErrGun[PORTB] == 0))
						port = PORTB;	
					//获取在线卡信息
					if (GetOnlineCardInfo(port, CardID, &CardBalance) == 1){
						//初始化枪数据
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, port);
						InsertChargingRecord(port, (uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));							
						//显示卡余额
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//先提示启动充电中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//启动充电
						ret = StartCharge(port);
						if (ret == 0){//启动成功
							if (port == 0)
								TwoGunState = A_CHARGE_B_IDLE;
							else
								TwoGunState = A_IDLE_B_CHARGE;
						} else {
							MainInfo.ChgDat[port].StopCause = GetStopCause(port);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[port] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));
							DeleteChargingRecord(port);
							if (port == PORTA)
								TwoGunState = A_STOP_B_IDLE;
							else if (port == PORTB)
								TwoGunState = A_IDLE_B_STOP;
						}
					}else {
					
					}
				}
			}else if ((SetMainChargeFlag[PORTA] == 1) || (SetMainChargeFlag[PORTB] == 1)) {//后台启动     				//判断并处理后台操作
				if (SetMainChargeFlag[PORTA] == 1){
					SetMainChargeFlag[PORTA] = 0;
					port = PORTA;
				} else 	if (SetMainChargeFlag[PORTB] == 1){
					SetMainChargeFlag[PORTB] = 0;
					port = PORTB;
				}					
				//滴声提示已响应后台操作
				Beep(100);
				//初始化枪数据
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[port], port);
				InsertChargingRecord(port, (uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));
				
				//显示余额
				DispRemainMoneyInfo(PlatformPara[port]);
				OSTimeDlyHMSM (0, 0, 2, 0);				
		
				//先提示启动充电中界面
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//启动充电
				ret = StartCharge(port);
				if (ret == 0){//启动成功
					if (port == 0)
						TwoGunState = A_CHARGE_B_IDLE;
					else
						TwoGunState = A_IDLE_B_CHARGE;
				} else {
					MainInfo.ChgDat[port].StopCause = GetStopCause(port);//GetCCBStopCause(0);
					//生成充电记录,并删除充电过程数据
					ReportChargeData[port] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));
					DeleteChargingRecord(port);
					if (port == 0)
						TwoGunState = A_STOP_B_IDLE;
					else
						TwoGunState = A_IDLE_B_STOP;
				}				
			}
		
		}
	
	}
	//A枪空闲 B枪充电--------------------------------------------------------------------------------------------------------------------------------------------
	else if (TwoGunState == A_IDLE_B_CHARGE){
		//更新充电中的数据
		UpdateChargeData(&MainInfo, info[PORTB], PORTB);
		//更新显示屏数据
		DispAIdleBChgInfo(AQRCode, APileNumber, info[PORTA].GunSeatState, \
										 	MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, 
																																												 MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);			
		//设置指示灯状态
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 1);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}		
		//判断并处理刷离线卡操作
		if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) && (CardLockState == LOCK)){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//滴声提示已响应操作
					Beep(100);
					//停止充电
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;		
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);	
					//设置状态
					TwoGunState = A_IDLE_B_IDLE;	
					//显示结算界面
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//停止充电
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;		
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//设置状态
					TwoGunState = A_IDLE_B_STOP;	
				}
			} else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);
				
			} else {
				if(ErrGun[PORTA] ){
					//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0 ){
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);		
				}else{
					if( LockCardPro(CardID)){
						//滴声提示已响应操作
						Beep(100);
						//初始化枪数据								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTA);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));	
						
						//显示卡余额
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);	
						
						//显示启动中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);
						
						//启动界面
						ret = StartCharge(PORTA);
						if (ret == 0){//启动成功
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);//GetCCBStopCause(0);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTA] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTA);
							TwoGunState = A_STOP_B_CHARGE;
							DispStartFailureInfo(); //显示启动失败
							OSTimeDlyHMSM (0, 0, 2, 0);
						}
					}else{
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
					}		
				}
			}
		}else if (SwingOnlineCardFlag == 1){
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//刷卡停止
				//计算余额
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//滴声提示已响应操作
				Beep(100);				
				//停止充电
				StopCharge(PORTB);		
				MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;	
				//生成充电记录,并删除充电过程数据
				ReportChargeData[PORTB] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTB);
				//设置状态
				TwoGunState = A_IDLE_B_IDLE;
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				if(ErrGun[PORTA] ){
							//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0  ){//未插枪
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);					
				}else {		
					//获取在线卡信息
					if (GetOnlineCardInfo(PORTA, CardID, &CardBalance) == 1){
						//初始化枪数据
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTA);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));							
						//显示卡余额
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//先提示启动充电中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//启动充电
						ret = StartCharge(PORTA);
						if (ret == 0){//启动成功
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTA] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTA);
							TwoGunState = A_STOP_B_CHARGE;
						}
					}else {
					
					}
				}
			}
		}
		else if (SetMainChargeFlag[PORTA] == 1) {//后台启动     				//判断并处理后台操作
			SetMainChargeFlag[PORTA] = 0;
			if(ErrGun[PORTA] ){
				//滴声提示已响应操作
				Beep(100);
				DispStartFailureInfo();
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTA].GunSeatState == 1){
				//滴声提示已响应后台操作
				Beep(100);
				//初始化枪数据
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTA], PORTA);
				InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));			
				//显示余额
				DispRemainMoneyInfo(PlatformPara[PORTA]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//先提示启动充电中界面
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//启动充电
				ret = StartCharge(PORTA);
				if (ret == 0){//启动成功
					TwoGunState = A_CHARGE_B_CHARGE;
				} else {
					MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);//GetCCBStopCause(0);
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					TwoGunState = A_STOP_B_CHARGE;
				}	
			}	else{
			//提示插枪
			}
		}else if ((SetMainChargeFlag[PORTB] == 2) && (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_PLATFORM)){//后台停止
			SetMainChargeFlag[PORTB] = 0;			
			//计算余额
			money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;			
			//峰鸣器响提示后台操作已被处理
			Beep(100);			
			//停止充电
			StopCharge(PORTB);
			MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;		
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//设置状态
			TwoGunState = A_IDLE_B_IDLE;
			//显示结算界面
			DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTB)){
			Beep(100);
			//停止充电
			StopCharge(PORTB);
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//设置状态
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_IDLE_B_STOP;
			else
				TwoGunState = A_IDLE_B_IDLE;	

		}else if (info[PORTB].ChargeState != 1){
			//收集停止原因
			MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);	
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);						
			//设置状态
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_IDLE_B_STOP;
			else
				TwoGunState = A_IDLE_B_IDLE;
		}
				
	}
	else if(TwoGunState == A_IDLE_B_STOP){	
		if( MainInfo.ChgDat[PORTB].StartType ==  STARTCHARGETYPE_PLATFORM){
				DispAIdleBStopInfo(AQRCode, APileNumber, info[PORTA].GunSeatState, \
					(uint8_t *)"   ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);
		}else{
				DispAIdleBStopInfo(AQRCode, APileNumber, info[PORTA].GunSeatState, \
					(uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);
		}

		//设置指示灯状态
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		if (info[PORTB].GunSeatState == 0) {//未插枪
			//设置状态
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_IDLE;
		} 			
		//判断并处理刷离线卡操作
		else if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//刷卡停止
				//解锁
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//删除存储器里面的锁卡信息
					DeleteLockCardRecord(CardID);			
					//显示结算信息
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//设置状态
					TwoGunState = A_IDLE_B_IDLE;
				}else{
//						OSTimeDlyHMSM (0, 0, 2, 0);
				}
			} else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);		
			} else {
				if(ErrGun[PORTA] ){
					//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0 ){
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);		
				}else{
					if( LockCardPro(CardID)){
						//滴声提示已响应操作
						Beep(100);
						//初始化枪数据								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTA);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));							
						//显示卡余额
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//显示启动中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);					
						//启动界面
						ret = StartCharge(PORTA);
						if (ret == 0){//启动成功
							TwoGunState = A_CHARGE_B_STOP;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTA] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTA);
							TwoGunState = A_STOP_B_STOP;
							DispStartFailureInfo(); //显示启动失败
							OSTimeDlyHMSM (0, 0, 2, 0);
						}
					}else{
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
					}		
				}
			}
		}else if (SwingOnlineCardFlag == 1){//刷在线卡
			SwingOnlineCardFlag = 0;		
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//滴声提示已响应操作
				Beep(100);
				//设置状态
				TwoGunState = A_IDLE_B_IDLE;
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			} else {
				if(ErrGun[PORTA] ){
					//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0  ){//未插枪
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);					
				}else {				
					//获取在线卡信息
					if (GetOnlineCardInfo(PORTA	, CardID, &CardBalance) == 1){
						//初始化枪数据
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTA	);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));							
						//显示卡余额
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//先提示启动充电中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//启动充电
						ret = StartCharge(PORTA);
						if (ret == 0){//启动成功
							TwoGunState = A_CHARGE_B_STOP;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTA] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTA);
							TwoGunState = A_STOP_B_STOP;
						}
					}else {
						//
					}
				}
			}
		}else if (SetMainChargeFlag[PORTA] == 1) {//后台启动     				//判断并处理后台操作
			SetMainChargeFlag[PORTA] = 0;
			if(ErrGun[PORTA] ){
							//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTA].GunSeatState == 1){
				//滴声提示已响应后台操作
				Beep(100);
				//初始化枪数据
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTA], PORTA);
				InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));			
				//显示余额
				DispRemainMoneyInfo(PlatformPara[PORTA]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//先提示启动充电中界面
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//启动充电
				ret = StartCharge(PORTA);
				if (ret == 0){//启动成功
					TwoGunState = A_CHARGE_B_STOP;
				} else {
					MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);//GetCCBStopCause(0);
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					TwoGunState = A_STOP_B_STOP;
				}	
			}	else{
			//提示插枪
			}
		}
	}
	else if(TwoGunState == A_CHARGE_B_IDLE){
		//更新充电中的数据
		UpdateChargeData(&MainInfo, info[PORTA], PORTA);			
		//更新显示屏数据
		DispAChgBIdleInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC, 
																																												 MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime, \
											BQRCode, BPileNumber, info[PORTB].GunSeatState);
		//设置指示灯状态
		SetLEDState(PORTA, 1);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		//判断并处理刷离线卡操作
		if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//滴声提示已响应操作
					Beep(100);
					//停止充电
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;			
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//设置状态
					TwoGunState = A_IDLE_B_IDLE;	
					//显示结算界面
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//停止充电
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;			
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//设置状态
					TwoGunState = A_STOP_B_IDLE;
				}
			} else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);			
			} else {
				if(ErrGun[PORTB] ){	
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0 ){
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);		
				}else{
					if( LockCardPro(CardID)){
						//滴声提示已响应操作
						Beep(100);
						//初始化枪数据								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTB);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));						
						//显示卡余额
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);						
						//显示启动中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);					
						//启动界面
						ret = StartCharge(PORTB);
						if (ret == 0){//启动成功
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);//GetCCBStopCause(0);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTB] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTB);
							TwoGunState = A_CHARGE_B_STOP;
							DispStartFailureInfo(); //显示启动失败
							OSTimeDlyHMSM (0, 0, 2, 0);
						}
					}else{
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
					}		
				}
			}
		}else if (SwingOnlineCardFlag == 1){
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//滴声提示已响应操作
				Beep(100);				
				//停止充电
				StopCharge(PORTA);	
				MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;				
				//生成充电记录,并删除充电过程数据
				ReportChargeData[PORTA] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTA);
				//设置状态
				TwoGunState = A_IDLE_B_IDLE;
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				if(ErrGun[PORTB] ){
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0  ){//未插枪
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				}else {		
					//获取在线卡信息
					if (GetOnlineCardInfo(PORTB, CardID, &CardBalance) == 1){
						//初始化枪数据
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTB);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));							
						//显示卡余额
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//先提示启动充电中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//启动充电
						ret = StartCharge(PORTB);
						if (ret == 0){//启动成功
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTB] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTB);
							TwoGunState = A_CHARGE_B_STOP;
						}
					}else {
					
					}
				}
			}
		}else if (SetMainChargeFlag[PORTB] == 1) {//后台启动     				//判断并处理后台操作
			SetMainChargeFlag[PORTB] = 0;
			if(ErrGun[PORTB] ){
				Beep(100);
				DispStartFailureInfo();
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTB].GunSeatState == 1){	
				Beep(100);
				//初始化枪数据
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTB], PORTB);
				InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));			
				//显示余额
				DispRemainMoneyInfo(PlatformPara[PORTB]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//先提示启动充电中界面
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//启动充电
				ret = StartCharge(PORTB);
				if (ret == 0){//启动成功
					TwoGunState = A_CHARGE_B_CHARGE;
				} else {
					MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);//GetCCBStopCause(0);
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					TwoGunState = A_CHARGE_B_STOP;
				}	
			}	else{
			//提示插枪
			}
		}else if ((SetMainChargeFlag[PORTA] == 2) && (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_PLATFORM)){//后台停止
			SetMainChargeFlag[PORTA] = 0;			
			//计算余额
			money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;		
			//峰鸣器响提示后台操作已被处理
			Beep(100);			
			//停止充电
			StopCharge(PORTA);
			MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;	
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//设置为空闲状态
			TwoGunState = A_IDLE_B_IDLE;
			//显示结算界面
			DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}	else if(ConditionStop(&MainInfo, PORTA)){
			Beep(100);
			//停止充电
			StopCharge(PORTA);
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//设置状态
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_IDLE;
			else
				TwoGunState = A_IDLE_B_IDLE;	
		}else if (info[PORTA].ChargeState != 1){
			//收集停止原因
			MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);	
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);							
			//设置状态
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_IDLE;
			else
				TwoGunState = A_IDLE_B_IDLE;
		}
	}
	else if(TwoGunState == A_CHARGE_B_CHARGE){
		//更新充电中的数据
		UpdateChargeData(&MainInfo, info[PORTA], PORTA);
		UpdateChargeData(&MainInfo, info[PORTB], PORTB);		
		DispAChgBChgInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC,
																																												MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime,
										 MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, 
																																												MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);
		//设置指示灯状态
		SetLEDState(PORTA, 1);
		SetLEDState(PORTB, 1);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		//判断并处理刷离线卡操作
		if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) ){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//滴声提示已响应操作
					Beep(100);
					//停止充电
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//设置状态
					TwoGunState = A_IDLE_B_CHARGE;		
					//显示结算界面
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//停止充电
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//设置状态
					TwoGunState = A_STOP_B_CHARGE;	
				}
			}else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) && (CardLockState == LOCK)){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//滴声提示已响应操作
					Beep(100);
					//停止充电
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;					
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//设置状态
					TwoGunState = A_CHARGE_B_IDLE;		
					//显示结算界面
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//停止充电
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;	
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//设置状态
					TwoGunState = A_CHARGE_B_STOP;		
				}
			}	else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);		
			} else {
				//滴声提示已响应操作
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}else if (SwingOnlineCardFlag == 1){//A充电B待机刷在线卡
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//滴声提示已响应操作
				Beep(100);				
				//停止充电
				StopCharge(PORTA);
				MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;	
				//生成充电记录,并删除充电过程数据
				ReportChargeData[PORTA] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTA);				
				//设置状态
				TwoGunState = A_IDLE_B_CHARGE;
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//滴声提示已响应操作
				Beep(100);				
				//停止充电
				StopCharge(PORTB);
				MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
				//生成充电记录,并删除充电过程数据
				ReportChargeData[PORTB] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTB);								
				//设置状态
				TwoGunState = A_CHARGE_B_IDLE;
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}
			else {
				//滴声提示已响应操作
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}	else if ((SetMainChargeFlag[PORTA] == 2) && (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_PLATFORM)){//后台停止
			SetMainChargeFlag[PORTA] = 0;			
			//计算余额
			money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;				
			//峰鸣器响提示后台操作已被处理
			Beep(100);				
			//停止充电
			StopCharge(PORTA);
			MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;			
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);
				//设置为空闲状态
			TwoGunState = A_IDLE_B_CHARGE;			
			//显示结算界面
			DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTA)){
			Beep(100);
			//停止充电
			StopCharge(PORTA);
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//设置状态
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_CHARGE;
			else
				TwoGunState = A_IDLE_B_CHARGE;	

		}	else if (info[PORTA].ChargeState != 1){
			//收集停止原因
			MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);	
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);				
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_CHARGE;
			else
				TwoGunState = A_IDLE_B_CHARGE;
		}	else if ((SetMainChargeFlag[PORTB] == 2) && (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_PLATFORM)){//后台停止
			SetMainChargeFlag[PORTB] = 0;
			//计算余额
			money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;
			Beep(100);			
			//停止充电
			StopCharge(PORTB);
			MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//设置为空闲状态
			TwoGunState = A_CHARGE_B_IDLE;	
			//显示结算界面
			DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTB)){
			Beep(100);
			//停止充电
			StopCharge(PORTB);
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);		
			//设置状态
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_CHARGE_B_STOP;
			else
				TwoGunState = A_CHARGE_B_IDLE;	
		}else if (info[PORTB].ChargeState != 1){
			//收集停止原因
			MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);		
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);				
			//设置状态
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_CHARGE_B_STOP;
			else
				TwoGunState = A_CHARGE_B_IDLE;
		}				
	}
	else if(TwoGunState == A_CHARGE_B_STOP){
		//更新充电中的数据
		UpdateChargeData(&MainInfo, info[PORTA], PORTA);		
		if( MainInfo.ChgDat[PORTB].StartType ==  STARTCHARGETYPE_PLATFORM){
			DispAChgBStopInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC, \
																																													 MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime, \
												(uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);
		}else{
			DispAChgBStopInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC, \
																																											 MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime, \
										(uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);
		}
				//设置指示灯状态
		SetLEDState(PORTA, 1);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}		
		if (info[PORTB].GunSeatState == 0) {//未插枪
			//设置状态
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_CHARGE_B_IDLE;
		}else if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) && (CardLockState == LOCK)){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//滴声提示已响应操作
					Beep(100);
					//停止充电
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//设置状态
					TwoGunState = A_IDLE_B_STOP;		
					//显示结算界面
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//停止充电
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//设置状态
					TwoGunState = A_STOP_B_STOP;	
				}
			}else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) ){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//删除存储器里面的锁卡信息
					DeleteLockCardRecord(CardID);			
					//显示结算信息
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//设置状态
					TwoGunState = A_CHARGE_B_IDLE;
				}else{
					
				}
			}
			else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);				
			} else {
					//滴声提示已响应操作
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}else if (SwingOnlineCardFlag == 1){//A充电停止刷在线卡
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				//计算余额
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//滴声提示已响应操作
				Beep(100);				
				//停止充电
				StopCharge(PORTA);
				MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
				//生成充电记录,并删除充电过程数据
				ReportChargeData[PORTA] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTA);					
				//设置状态
				TwoGunState = A_IDLE_B_STOP;
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//设置状态
				TwoGunState = A_CHARGE_B_IDLE;
				//滴声提示已响应操作
				Beep(100);
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				//滴声提示已响应操作
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}	else if ((SetMainChargeFlag[PORTA] == 2) && (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_PLATFORM)){//后台停止
			SetMainChargeFlag[PORTA] = 0;			
			//计算余额
			money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;		
			//峰鸣器响提示后台操作已被处理
			Beep(100);			
			//停止充电
			StopCharge(PORTA);
			MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);
			//设置为空闲状态
			TwoGunState = A_IDLE_B_STOP;			
			//显示结算界面
			DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTA)){
			Beep(100);
			//停止充电
			StopCharge(PORTA);
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//设置状态
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_STOP;
			else
				TwoGunState = A_IDLE_B_STOP;	
		}else if (info[PORTA].ChargeState != 1){
			//收集停止原因
			MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);	
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);							
			//设置状态
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_STOP;	
			else
				TwoGunState = A_IDLE_B_STOP;	
		}			
	}
	else if(TwoGunState == A_STOP_B_IDLE){
		if( MainInfo.ChgDat[PORTA].StartType ==  STARTCHARGETYPE_PLATFORM){
			DispAStopBIdleInfo((uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,			
												BQRCode, BPileNumber, info[PORTB].GunSeatState);
		}else{
			DispAStopBIdleInfo((uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,			
										BQRCode, BPileNumber, info[PORTB].GunSeatState);
		}
		//设置指示灯状态
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		
		if (info[PORTA].GunSeatState == 0) {//未插枪
			//设置状态
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_IDLE;
		}else if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//删除存储器里面的锁卡信息
					DeleteLockCardRecord(CardID);			
					//显示结算信息
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//设置状态
					TwoGunState = A_IDLE_B_IDLE;
				}else{
 
				}
			} else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);	
			} else {
				if(ErrGun[PORTB] ){
					//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0 ){
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				}else{
					if( LockCardPro(CardID)){
						//滴声提示已响应操作
						Beep(100);
						//初始化枪数据								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTB);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));						
						//显示卡余额
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//显示启动中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);					
						//启动界面
						ret = StartCharge(PORTB);
						if (ret == 0){//启动成功
							TwoGunState = A_STOP_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTB] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTB);
							TwoGunState = A_STOP_B_STOP;
							DispStartFailureInfo(); //显示启动失败
							OSTimeDlyHMSM (0, 0, 2, 0);
						}
					}else{
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
					}		
				}
			}
		}else if (SwingOnlineCardFlag == 1){//刷在线卡
			SwingOnlineCardFlag = 0;		
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				//计算余额
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//设置状态
				TwoGunState = A_IDLE_B_IDLE;
				//滴声提示已响应操作
				Beep(100);
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			} else {
				if(ErrGun[PORTB] ){
							//滴声提示已响应操作
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0  ){//未插枪
					//滴声提示已响应操作
					Beep(100);
					//提示未插枪
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				}else {				
					//获取在线卡信息
					if (GetOnlineCardInfo(PORTB	, CardID, &CardBalance) == 1){
						//初始化枪数据
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTB	);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));							
						//显示卡余额
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//先提示启动充电中界面
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//启动充电
						ret = StartCharge(PORTB);
						if (ret == 0){//启动成功
							TwoGunState = A_STOP_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);
							//生成充电记录,并删除充电过程数据
							ReportChargeData[PORTB] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTB);
							TwoGunState = A_STOP_B_STOP;
						}
					}else {
						//
					}
				}
			}
		}else if (SetMainChargeFlag[PORTB] == 1) {//后台启动     				//判断并处理后台操作
			SetMainChargeFlag[PORTB] = 0;
			if(ErrGun[PORTB] ){
				//滴声提示已响应操作
				Beep(100);
				DispStartFailureInfo();
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTB].GunSeatState == 1){
				//滴声提示已响应后台操作
				Beep(100);
				//初始化枪数据
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTB], PORTB);
				InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));			
				//显示余额
				DispRemainMoneyInfo(PlatformPara[PORTB]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//先提示启动充电中界面
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//启动充电
				ret = StartCharge(PORTB);
				if (ret == 0){//启动成功
					TwoGunState = A_STOP_B_CHARGE;
				} else {
					MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);//GetCCBStopCause(0);
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					TwoGunState = A_STOP_B_STOP;
				}	
			}	else{
			//提示插枪
			}
		}			
	
	}
	else if(TwoGunState == A_STOP_B_CHARGE){
		//更新充电中的数据
		UpdateChargeData(&MainInfo, info[PORTB], PORTB);
		if( MainInfo.ChgDat[PORTA].StartType ==  STARTCHARGETYPE_PLATFORM){
				DispAStopBChgInfo((uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,\
											MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, \
																																												 MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);
		}else{
			DispAStopBChgInfo((uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,\
											MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, \
																																												 MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);
		}	
		//设置指示灯状态
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 1);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}			
		if (info[PORTA].GunSeatState == 0) {//未插枪
			//设置状态
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_CHARGE;
		}else if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) && (CardLockState == LOCK)){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//滴声提示已响应操作
					Beep(100);
					//停止充电
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//设置状态
					TwoGunState = A_STOP_B_IDLE;		
					//显示结算界面
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//停止充电
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;			
					//生成充电记录,并删除充电过程数据
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//设置状态
					TwoGunState = A_STOP_B_STOP;
				}
			}else if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) ){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//删除存储器里面的锁卡信息
					DeleteLockCardRecord(CardID);			
					//显示结算信息
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//设置状态
					TwoGunState = A_IDLE_B_CHARGE;
				}else{
 
				}
			}else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);	
			} else {
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}
		else if (SwingOnlineCardFlag == 1){//A充电停止刷在线卡
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//滴声提示已响应操作
				Beep(100);				
					//停止充电
				StopCharge(PORTB);
				MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
				//生成充电记录,并删除充电过程数据
				ReportChargeData[PORTB] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTB);
				TwoGunState = A_STOP_B_IDLE;
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//设置状态
				TwoGunState = A_IDLE_B_CHARGE;
				//滴声提示已响应操作
				Beep(100);
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}else if ((SetMainChargeFlag[PORTB] == 2) && (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_PLATFORM)){//后台停止
			SetMainChargeFlag[PORTB] = 0;
			money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;			
			//峰鸣器响提示后台操作已被处理
			Beep(100);			
			//停止充电
			StopCharge(PORTB);
			MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;	
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);		
			//设置状态
			TwoGunState = A_STOP_B_IDLE;				
			//显示结算界面
			DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTB)){
			Beep(100);
			//停止充电
			StopCharge(PORTB);
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//设置状态
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_STOP;
			else
				TwoGunState = A_STOP_B_IDLE;	
		}else if (info[PORTB].ChargeState != 1){
			//收集停止原因
			MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);		
			//生成充电记录,并删除充电过程数据
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);						
			//设置状态
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_STOP;
			else
				TwoGunState = A_STOP_B_IDLE;
		}		
	}
	else if(TwoGunState == A_STOP_B_STOP){

	if( (MainInfo.ChgDat[PORTA].StartType ==  STARTCHARGETYPE_PLATFORM) && (MainInfo.ChgDat[PORTB].StartType ==  STARTCHARGETYPE_PLATFORM)){		
			DispAStopBStopInfo((uint8_t *)"   ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,
												 (uint8_t *)"   ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);	
	}else if( (MainInfo.ChgDat[PORTA].StartType ==  STARTCHARGETYPE_PLATFORM) && (MainInfo.ChgDat[PORTB].StartType !=  STARTCHARGETYPE_PLATFORM)){		
			DispAStopBStopInfo((uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,
												 (uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);	
	}else if( (MainInfo.ChgDat[PORTA].StartType !=  STARTCHARGETYPE_PLATFORM) && (MainInfo.ChgDat[PORTB].StartType ==  STARTCHARGETYPE_PLATFORM)){		
			DispAStopBStopInfo((uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,
												 (uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);	
	}else if( (MainInfo.ChgDat[PORTA].StartType !=  STARTCHARGETYPE_PLATFORM) && (MainInfo.ChgDat[PORTB].StartType !=  STARTCHARGETYPE_PLATFORM)){		
			DispAStopBStopInfo((uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,
												 (uint8_t *)"请刷卡结算", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);	
	}

		//设置指示灯状态
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		if (info[PORTA].GunSeatState == 0) {//未插枪
			//设置状态
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_STOP;
		}else if(info[PORTB].GunSeatState == 0){
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_STOP_B_IDLE;
		}			
		//判断并处理刷离线卡操作
		else if (SwingOfflineCardFlag == 1){//刷离线卡
			SwingOfflineCardFlag = 0;		
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) && (CardLockState == LOCK)){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//删除存储器里面的锁卡信息
					DeleteLockCardRecord(CardID);			
					//显示结算信息
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//设置状态
					TwoGunState = A_IDLE_B_STOP;
				}else{

				}
			}else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) ){//刷卡停止
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//删除存储器里面的锁卡信息
					DeleteLockCardRecord(CardID);			
					//显示结算信息
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//设置状态
					TwoGunState = A_STOP_B_IDLE;
				}else{
   
				}
			}else if(CardLockState == 2){//是被锁定的卡
				//进入解锁卡程序
				LockedCardHandle(CardID, CardBalance);
			} else {
					//滴声提示已响应操作
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}	else if (SwingOnlineCardFlag == 1){//A充电停止刷在线卡
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//刷卡停止
				//计算余额
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//设置状态
				TwoGunState = A_IDLE_B_STOP;
				//滴声提示已响应操作
				Beep(100);
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//刷卡停止
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//设置状态
				TwoGunState = A_STOP_B_IDLE;
				//滴声提示已响应操作
				Beep(100);
				//显示结算界面
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				Beep(100);			
				//提示无空闲车位
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}
	}
}

/************************************************************************************************************
** 函 数 名 : BoardRunLedControl
** 功能描述 : 主板运行指示灯控制函数 1秒亮 1秒灭
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void BoardRunLedControl(void)
{
	static uint8_t state,poweron = 0x5a;
	static uint32_t timer;
	uint8_t i;
	
	if (poweron == 0x5a){
		poweron = 0;
		//运行指示灯上电快闪10次
		for(i=0;i<10;i++){
			LedRunOn();
			OSTimeDlyHMSM (0, 0, 0, 100);
			LedRunOff();
			OSTimeDlyHMSM (0, 0, 0, 100);
		}
	}
	if (TimerRead() - timer > T1S * 1){
		timer = TimerRead();
		if (state == 0){
			state = 1;
			LedRunOn();
		} else {
			state = 0;
			LedRunOff();
		}
	}
}



/************************************************************************************************************
** 函 数 名 : LEDBoardControl
** 功能描述 : 三色灯板控制函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void LEDBoardControl(void)
{
	uint8_t i;
	uint8_t PortNum;
	static uint32_t timer;
	static uint8_t count;
	
	PortNum = ParaGetChgGunNumber();
	SetRelayState(0, 1);
	if ((LEDState[0] == 2) || ((LEDState[1] == 2))){//故障
		SetRelayState(3, 1);
		LedWarnOff();
		LedChargeOn();
		for (i = 0; i < 8; i++){
			if (i % 2){
				LedPwrOn();
			} else {
				LedPwrOff();
			}
		}
	} else {
		SetRelayState(3, 0);
	} 
	if (PortNum == 1){//单枪
		if (LEDState[0] == 1){//A枪充电
			SetRelayState(1, 0);
			SetRelayState(2, 1);
			if (TimerRead() - timer > T100MS * 3){
				timer = TimerRead();
				if (count++ >= 7){
					count = 0;
					LedWarnOff();
					OSTimeDlyHMSM (0, 0, 0, 1);
				}
				LedWarnOn();
				LedChargeOff();
				LedPwrOff();
				OSTimeDlyHMSM (0, 0, 0, 1);
				LedPwrOn();
			}	
		} else if (LEDState[0] == 0){//A枪待机
			SetRelayState(1, 1);
			SetRelayState(2, 0);
			LedWarnOn();
			LedChargeOff();
			for (i = 0; i < 8; i++){
				if (i % 2){
					LedPwrOn();
				} else {
					LedPwrOff();
				}
			}
		}else if(LEDState[0] == 2){ //故障
			SetRelayState(1, 0);
			SetRelayState(2, 0);
			LedWarnOff();
			LedChargeOn();
			for (i = 0; i < 8; i++){
				if (i % 2){
					LedPwrOn();
				} else {
					LedPwrOff();
				}
			}
		
		}
	} else {//双枪
		if (LEDState[0] == 1){
			SetRelayState(1, 1);
			if (TimerRead() - timer > T100MS * 3){
				timer = TimerRead();
				if (count++ >= 7){
					count = 0;
					LedWarnOff();
					OSTimeDlyHMSM (0, 0, 0, 1);
				}
				LedWarnOn();
				LedChargeOff();
				LedPwrOff();
				OSTimeDlyHMSM (0, 0, 0, 1);
				LedPwrOn();
			}
		}else{//A枪待机
			SetRelayState(1, 0);
			LedWarnOn();
			LedChargeOff();
			for (i = 0; i < 8; i++){
				if (i % 2){
					LedPwrOn();
				} else {
					LedPwrOff();
				}
			}
		}
		if (LEDState[1] == 1){
			SetRelayState(2, 1);
			if (TimerRead() - timer > T100MS * 3){
				timer = TimerRead();
				if (count++ >= 7){
					count = 0;
					LedWarnOff();
					OSTimeDlyHMSM (0, 0, 0, 1);
				}
				LedWarnOn();
				LedChargeOff();
				LedPwrOff();
				OSTimeDlyHMSM (0, 0, 0, 1);
				LedPwrOn();
			}
		}else{//B枪待机
			SetRelayState(2, 0);
			LedWarnOn();
			LedChargeOff();
			for (i = 0; i < 8; i++){
				if (i % 2){
					LedPwrOn();
				} else {
					LedPwrOff();
				}
			}
		}
	}

}

/************************************************************************************************************
** 函 数 名 : SetMainCharge
** 功能描述 : 设置主充电函数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......  setcmd SET_START_CHARGE/SET_STOP_CHARGE
							mode 模式 0充满 1按电量 2按时间 3按金额
							para 参数 按充满时精度为0.01元 按电量时精度为0.01kWh 按时间时精度为秒 按金额时精度为0.01元
							CardOrDiscernNumber 充电卡号/用户识别号 32个字节ASCII
							userid 用户ID
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SetMainCharge(uint8_t port, uint8_t setcmd, uint8_t mode, uint32_t para, uint8_t *CardOrDiscernNumber, uint8_t *userid)
{
	if (port < 2){
		if (setcmd == SET_START_CHARGE){
			SetMainChargeFlag[port] = 1;
			PlatformMode[port] = mode;//平台启动时的模式 0充满 1按电量 2按时间 3按金额
			PlatformPara[port] = para;//平台启动时的参数 按充满时精度为0.01元 按电量时精度为0.01kWh 按时间时精度为秒 按金额时精度为0.01元
			memcpy(PlatformCardOrDiscernNumber[port], CardOrDiscernNumber, 32);//平台启动时的充电卡号/用户识别号
			memcpy(PlatformUserID[port], userid, 2);//用户ID
		} else {
			SetMainChargeFlag[port] = 2;
		}
	}
}

/************************************************************************************************************
** 函 数 名 : GetMainCharge
** 功能描述 : 获取主充电状态
** 输    入 : 无
** 输    出 : 无
** 返    回 :	STATE_IDLE/STATE_CHARGE/STATE_COMPLETE
*************************************************************************************************************
*/
uint8_t GetMainCharge(uint8_t port)
{
	uint8_t PortNum = ParaGetChgGunNumber();
	
	if (PortNum == 1){
		if (port < 2){
			if (OneGunState == A_IDLE){
				return STATE_IDLE;
			} else if (OneGunState == A_CHARGE){
				return STATE_CHARGE;
			} else {
				return STATE_COMPLETE;
			}
		}
	} else {
		if (port == 0){
			if (TwoGunState == A_IDLE_B_IDLE					)//A枪空闲 B枪空闲
				return STATE_IDLE;
			else if(TwoGunState == A_IDLE_B_CHARGE		)//A枪空闲 B枪充电
				return STATE_IDLE;
			else if(TwoGunState == A_IDLE_B_STOP			)//A枪空闲 B枪停止
				return STATE_IDLE;
			else if(TwoGunState == A_CHARGE_B_IDLE		)//A枪充电 B枪空闲
				return STATE_CHARGE;
			else if(TwoGunState == A_CHARGE_B_CHARGE	)//A枪充电 B枪充电
				return STATE_CHARGE;
			else if(TwoGunState == A_CHARGE_B_STOP		)//A枪充电 B枪停止
				return STATE_CHARGE;
			else if(TwoGunState == A_STOP_B_IDLE			)//A枪停止 B枪空闲
				return STATE_COMPLETE;
			else if(TwoGunState == A_STOP_B_CHARGE		)//A枪停止 B枪充电
				return STATE_COMPLETE;
			else if(TwoGunState == A_STOP_B_STOP			)//A枪停止 B枪停止
				return STATE_COMPLETE;
		} else if(port == 1){
			if (TwoGunState == A_IDLE_B_IDLE					)//A枪空闲 B枪空闲
				return STATE_IDLE;
			else if(TwoGunState == A_IDLE_B_CHARGE		)//A枪空闲 B枪充电
				return STATE_CHARGE;
			else if(TwoGunState == A_IDLE_B_STOP			)//A枪空闲 B枪停止
				return STATE_COMPLETE;
			else if(TwoGunState == A_CHARGE_B_IDLE		)//A枪充电 B枪空闲
				return STATE_IDLE;
			else if(TwoGunState == A_CHARGE_B_CHARGE	)//A枪充电 B枪充电
				return STATE_CHARGE;
			else if(TwoGunState == A_CHARGE_B_STOP		)//A枪充电 B枪停止
				return STATE_COMPLETE;
			else if(TwoGunState == A_STOP_B_IDLE			)//A枪停止 B枪空闲
				return STATE_IDLE;
			else if(TwoGunState == A_STOP_B_CHARGE		)//A枪停止 B枪充电
				return STATE_CHARGE;
			else if(TwoGunState == A_STOP_B_STOP			)//A枪停止 B枪停止
				return STATE_COMPLETE;
		}
	}
	return 0;
}



/************************************************************************************************************
** 函 数 名 : GetChargeRecord
** 功能描述 : 从内存中获取充电记录数据
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪  ChgData  存储的充电记录数据地址
** 输    出 : 无
** 返    回 :	0  = 获取失败   1 = 获取成功 
*************************************************************************************************************
*/
uint8_t GetChargeRecord(uint8_t port, CHARGINGDATA* ChgData)
{
	uint8_t StartDateTime[6];
	memset(StartDateTime, 0, 6);
	
	if(ReportChargeData[port] == 0)
		return 0;
	if((MainInfo.ChgDat[port].ChgPort != port)||(memcmp(MainInfo.ChgDat[port].StartDateTime, StartDateTime, 6)== 0))
		return 0;
	memcpy(ChgData, &MainInfo.ChgDat[port], sizeof(CHARGINGDATA));
	return 1;
}




/************************************************************************************************************
** 函 数 名 : MainTask
** 功能描述 : 主任务
** 输    入 : pdata 未使用
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/


void MainTask(void *pdata)
{
	uint8_t i = 10;
	uint8_t PortNum = ParaGetChgGunNumber();
	Beep(100);
	while(i){
		DispStartDeviceInfo(_VERSION_MAJOR, _VERSION_MINOR, _VERSION_PATCH);	//显示加载界面
		OSTimeDlyHMSM (0, 0, 0, 500);
		i--;
	}
	if(GetUpdata() == 0xAA){
		SetUpdataFlag(0);
		DispUpdataInfo(100, (uint8_t* )"升级成功 ");
		OSTimeDlyHMSM (0, 0, 5, 0);
	}

	TimerCntRun = TimerRead();
	while(1){
		ParaGetQRCode(0, AQRCode);
		ParaGetPileNo(0, APileNumber);
		strcat((char* )APileNumber,"A");
		ParaGetQRCode(1, BQRCode);
		ParaGetPileNo(1, BPileNumber);
		strcat((char* )BPileNumber,"B");
		if (PortNum == 1)
			OneGunHandle();
		else
			TwoGunHandle();
		
		WdgFeed();
		
		BoardRunLedControl();
		
		AnalyseMsgFromDebugPlatform();
		AnalyseMsgFromUdpPlatform();
		
		LEDBoardControl();  //三色灯

		
		OSTimeDlyHMSM (0, 0, 0, 10);
	}
}

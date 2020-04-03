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

static uint8_t SetMainChargeFlag[2];//1���� 2ֹͣ
static uint8_t PlatformMode[2];//ƽ̨����ʱ��ģʽ 0���� 1������ 2��ʱ�� 3�����
static uint32_t PlatformPara[2];//ƽ̨����ʱ�Ĳ��� ������ʱ����Ϊ0.01Ԫ ������ʱ����Ϊ0.01kWh ��ʱ��ʱ����Ϊ�� �����ʱ����Ϊ0.01Ԫ
static uint8_t PlatformCardOrDiscernNumber[2][32]; //ƽ̨����ʱ�ĳ�翨��/�û�ʶ���
static uint8_t PlatformUserID[2][2];//�û�ID

static uint8_t OneGunState,TwoGunState;//��ǹ��˫ǹ״̬

static uint8_t LifeTimeExpireFlag;//ʹ�����޵��ڱ�־ 0δ���� 1����
static	uint32_t SerialNum;       //���߿� ������ˮ��

static uint8_t IsLogin;  //�鿴����¼��ر���
static uint32_t ShowTimer;//�鿴����¼��ر���

static	uint8_t AQRCode[256];
static	uint8_t APileNumber[32];
static	uint8_t BQRCode[256];
static	uint8_t BPileNumber[32];

static uint8_t UpdataInFlag; //�����б�־

static uint8_t ReportChargeData[2];    //�ϱ�����¼��־

static uint32_t TimerCntRun; 

static uint8_t StartingChg[2];   //��������б�־

/************************************************************************************************************
** �� �� �� : GetStartType
** �������� : ��ȡ������ʽ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	STARTCHARGETYPE_OFFLINECARD STARTCHARGETYPE_ONLINECARD STARTCHARGETYPE_PLATFORM
*************************************************************************************************************
*/
uint8_t GetStartType(uint8_t port)
{
	return MainInfo.ChgDat[port].StartType;
}



/************************************************************************************************************
** �� �� �� : GetStartingChg
** �������� : ��ȡ�Ƿ���������
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	0 ����������  1  ������
*************************************************************************************************************
*/
uint8_t GetStartingChg(uint8_t port)
{
	return StartingChg[port];
}

/************************************************************************************************************
** �� �� �� : GetStartCardID
** �������� : ��ȡ������ID
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	��ID
*************************************************************************************************************
*/
uint32_t GetStartCardID(uint8_t port)
{
	return MainInfo.ChgDat[port].StartCardID;
}

/************************************************************************************************************
** �� �� �� : GetStartMode
** �������� : ��ȡ����ģʽ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	0���� 1������ 2��ʱ�� 3�����
*************************************************************************************************************
*/
uint8_t GetStartMode(uint8_t port)
{
	return MainInfo.ChgDat[port].Mode;
}

/************************************************************************************************************
** �� �� �� : GetStartPara
** �������� : ��ȡ��������
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	
*************************************************************************************************************
*/
uint32_t GetStartPara(uint8_t port)
{
	return MainInfo.ChgDat[port].Para;
}

/************************************************************************************************************
** �� �� �� : GetVoltage
** �������� : ��ȡ��ѹֵ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : A,B,C���ѹ
** ��    �� :	��
*************************************************************************************************************
*/
void GetVoltage(uint8_t port, uint32_t *VolA, uint32_t *VolB, uint32_t *VolC)
{
	*VolA = MainInfo.GunInfo[port].VoltageA;
	*VolB = MainInfo.GunInfo[port].VoltageB;
	*VolC = MainInfo.GunInfo[port].VoltageC;
}

/************************************************************************************************************
** �� �� �� : GetCurrent
** �������� : ��ȡ����ֵ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : A,B,C�����
** ��    �� :	��
*************************************************************************************************************
*/
void GetCurrent(uint8_t port, uint32_t *CurA, uint32_t *CurB, uint32_t *CurC)
{
	*CurA = MainInfo.GunInfo[port].CurrentA;
	*CurB = MainInfo.GunInfo[port].CurrentB;
	*CurC = MainInfo.GunInfo[port].CurrentC;
}

/************************************************************************************************************
** �� �� �� : GetStartMeterEnergy
** �������� : ��ȡ����ʱ������
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	���� 0.01KWH
*************************************************************************************************************
*/
uint32_t GetStartMeterEnergy(uint8_t port)
{
	return MainInfo.ChgDat[port].StartMeterEnergy;
}

/************************************************************************************************************
** �� �� �� : GetStopMeterEnergy
** �������� : ��ȡֹͣʱ������
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	���� 0.01KWH
*************************************************************************************************************
*/
uint32_t GetStopMeterEnergy(uint8_t port)
{
	return MainInfo.ChgDat[port].StopMeterEnergy;
}

/************************************************************************************************************
** �� �� �� : GetSumEnergy
** �������� : ��ȡ�ۼƳ�����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	���� 0.01KWH
*************************************************************************************************************
*/
uint32_t GetSumEnergy(uint8_t port)
{
	return MainInfo.ChgDat[port].SumEnergy;
}

/************************************************************************************************************
** �� �� �� : GetSumMoney
** �������� : ��ȡ�ۼƳ����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	��� 0.01Ԫ
*************************************************************************************************************
*/
uint32_t GetSumMoney(uint8_t port)
{
	return MainInfo.ChgDat[port].SumMoney;
}

/************************************************************************************************************
** �� �� �� : GetPeriodEnergy
** �������� : ��ȡʱ�ε���
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	���� 0.01KWH
*************************************************************************************************************
*/
uint16_t GetPeriodEnergy(uint8_t port, uint8_t Period)
{
	return MainInfo.ChgDat[port].PeriodEnergy[Period];
}

/************************************************************************************************************
** �� �� �� : GetSumTime
** �������� : ��ȡ�ۼ�ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� : ʱ�� 
*************************************************************************************************************
*/
uint32_t GetSumTime(uint8_t port)
{
	return MainInfo.ChgDat[port].SumTime;
}

/************************************************************************************************************
** �� �� �� : GetStartDateTime
** �������� : ��ȡ����ʱ����ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ����ʱ��
** ��    �� : ��
*************************************************************************************************************
*/
void GetStartDateTime(uint8_t port, uint8_t *datetime)
{
	memcpy(datetime, MainInfo.ChgDat[port].StartDateTime, 6);
}

/************************************************************************************************************
** �� �� �� : GetStopDateTime
** �������� : ��ȡֹͣʱ����ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ����ʱ��
** ��    �� : ��
*************************************************************************************************************
*/
void GetStopDateTime(uint8_t port, uint8_t *datetime)
{
	memcpy(datetime, MainInfo.ChgDat[port].StopDateTime, 6);
}

/************************************************************************************************************
** �� �� �� : GetStartCardMoney
** �������� : ��ȡ����ʱ�����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� : ��� 0.01Ԫ
*************************************************************************************************************
*/
uint32_t GetStartCardMoney(uint8_t port)
{
	return MainInfo.ChgDat[port].StartCardMoney;
}

/************************************************************************************************************
** �� �� �� : GetCardOrDiscernNumber
** �������� : ��ȡ��/ʶ����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��/ʶ����
** ��    �� : ��
*************************************************************************************************************
*/
void GetCardOrDiscernNumber(uint8_t port, uint8_t *CardOrDiscernNumber)
{
	memcpy(CardOrDiscernNumber, MainInfo.ChgDat[port].CardOrDiscernNumber, 32);
}

/************************************************************************************************************
** �� �� �� : SetLEDState
** �������� : ���õư���ʾ״̬
** ��    �� : port state 0���� 1��� 2����
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
static void SetLEDState(uint8_t port, uint8_t state)
{
	LEDState[port] = state;
}

/************************************************************************************************************
** �� �� �� : Beep
** �������� : �����������
** ��    �� : ms �����ʱ�� ����
** ��    �� : ��
** ��    �� :	��
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
** �� �� �� : JudgePeriod
** �������� : �ж�ʱ�亯��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......    time 0-47
** ��    �� : ��
** ��    �� :	��
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
			if ((caltime >= starttime) && (caltime < stoptime)){//ǰ�պ�
				money = ParaGetFeilvMoney(port, i) / 100;
				break; 
			}
		}
	}
	return money;
}

/************************************************************************************************************
** �� �� �� : GetOnlineCardInfo
** �������� : ��ȡ���߿���Ϣ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... CardID ���� CardBalance ���
** ��    �� : ��
** ��    �� :	��ȡ�ɹ�1 ��ȡʧ��0
*************************************************************************************************************
*/
static uint8_t GetOnlineCardInfo(uint8_t port, uint32_t CardID, uint32_t *CardBalance)
{
	uint8_t verifyonlineret;
	uint8_t TmpBuf[48];

	//��ʾ������֤�У����Ե�
	DispInVerify();
	//������ʾ����Ӧ����
	Beep(100);
	//��ʵ����Ϣ
	if ((verifyonlineret = VerifyOnlineCardInfo(port, CardID, CardBalance)) == 0){
		return 1;
	} else {
		//������ʾ����Ӧ����
		Beep(100);
		//��ʾ��֤ʧ��
		if (verifyonlineret == 0xff)
			sprintf((char *)TmpBuf, "��Ӧ��ʱ");
		else if (verifyonlineret == 2)
			sprintf((char *)TmpBuf, "����");
		else if (verifyonlineret == 5)
			sprintf((char *)TmpBuf, "���Ų�����");
		else if (verifyonlineret == 6)
			sprintf((char *)TmpBuf, "�ѹ�ʧ");
		else if (verifyonlineret == 7)
			sprintf((char *)TmpBuf, "������");
		else if (verifyonlineret == 9)
			sprintf((char *)TmpBuf, "��������׮���");
		else
			sprintf((char *)TmpBuf, "����ԭ��");
		DispVerifyFailure(TmpBuf);
		OSTimeDlyHMSM (0, 0, 5, 0);
		return 0;
	}
}

/************************************************************************************************************
** �� �� �� : JudgeLifeTime
** �������� : �ж�ʹ������
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
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
** �� �� �� : GetLifeTime
** �������� : ��ȡ�Ƿ����޵���
** ��    �� : ��
** ��    �� : ��
** ��    �� :	1 ����   0 ����
*************************************************************************************************************
*/
uint8_t GetLifeTime(void)
{
	return LifeTimeExpireFlag;
}

/************************************************************************************************************
** �� �� �� : ShowRecord
** �������� : ������ʾ��س���¼
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
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
							sprintf((char *)tempbuf, "�������%d", ErrTimes);
							XRD_Write_Addr(0x748,(char *)tempbuf, 16);	
							//��ʾ�������
							if(ErrTimes >= 3){
								break;  //�˳�
							}
						}
					}else if(KeyFun == NEXT_PAGE_BUTTON){   //��һҳ
						KeyFun = 0;
						Timer = TimerRead();						
						if(Page != 1){
							Page--;	
						}else{
							Page = SumPage;
						}
						ShowFlag = 1;							
					}else if(KeyFun == PRE_PAGR_BUTTON ){		  //��һҳ
						KeyFun = 0;
						Page++;	
						Timer = TimerRead();
						if(Page > SumPage)	
							Page = 1;
						ShowFlag = 1;						
					}else if(KeyFun == RETURN_BUTTON){   //���ذ���
						KeyFun = 0;
						if( Line ){
							Line = 0;
							XRD_Load_Page(PAGE28);							
						}else{
							ShowTimer = TimerRead();	
							break;  //�˳�
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
					ShowNum = RecordNum -((Page-1)*LINENUM)- Line + 1;   //��ʾ�ĵڼ�������
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
						XRD_Write_Addr(0x750,(char *)tempbuf, 8);				//��ʾ�ڼ�ҳ							 
						for(i = 0; i < LINENUM; i++){		
							ShowNum = RecordNum - ((Page - 1) * LINENUM + i);   //��ʾ�ĵڼ�������
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
						sprintf((char *)tempbuf, "���޼�¼");
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
** �� �� �� : InitStartChgDate
** �������� : ��ʼ����ʼ�������
** ��    �� : Maininfo  �����Ϣ�ṹ��   
**  				:	Mode  0���߿�  3���߿�   1��̨   port ==0 Aǹ   ==1Bǹ
**  				:	CardId   ��ID  Money == StartCardMoney 
** ��    �� : ��
** ��    �� :	��
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
	
	if(Mode == 0){   //  ���߿�
		sprintf((char *)Info->ChgDat[port].CardOrDiscernNumber, "%010u", CardId);
		Info->ChgDat[port].SerialNum = SerialNum;		
	}else if(Mode == 3){     //  ���߿�
		sprintf((char *)Info->ChgDat[port].CardOrDiscernNumber, "%010u", CardId);			
		Info->ChgDat[port].SerialNum = 0;	
	}else if(Mode == 1){   // ƽ̨
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
** �� �� �� : UpdateChargeData
** �������� : ���³��ʱ���������
** ��    �� : ChgData  MAININFO�ṹ��ָ��   
**  				:	info  CHARGEINFO�ṹ��
**  				:	port ==0 Aǹ   ==1Bǹ
** ��    �� : ��
** ��    �� :	��
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
					if(ChgData->ChgDat[port].Mode == 3){ //�����
						ChgData->ChgDat[port].Para = UpdateMoney;
					}
					ChgData->ChgDat[port].StartCardMoney = UpdateMoney;
					InsertChargingRecord(port, (uint8_t *)&ChgData->ChgDat[port], sizeof(CHARGINGDATA));
				}
			}
			//������������
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
			ChgData->ChgDat[port].SumMoney = (data32 + 900)/ 1000;//ʼ�ռ�0.009Ԫ������̨�ķ���Ѻͳ��ѷֿ�������
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
** �� �� �� : ShowMenu
** �������� : ��ʾ �˵��������Ϣ
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
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
			sprintf((char *)linebuf, "����");
		else
			sprintf((char *)linebuf, "   ");
	}else{
		if((GetOnlineState(0) == 1) && (GetOnlineState(1) == 1)){
			sprintf((char *)linebuf, "����");
		} else if( GetOnlineState(0) == 1){
			sprintf((char *)linebuf, "A����");
		}	else if( GetOnlineState(1) == 1){
			sprintf((char *)linebuf, "B����");
		}else {
			sprintf((char *)linebuf, "   ");
		}		
	}
	DispMenuInfo(timebuf, linebuf );		
}


/************************************************************************************************************
** �� �� �� : UnLockCardPro
** �������� : ���������������
** ��    �� : Cardid   ��ID   Money  д�Ľ��
** ��    �� : ��
** ��    �� :	1 ����д���ɹ�  0 ʧ��
*************************************************************************************************************
*/
static uint8_t UnLockCardPro(uint32_t cardid, uint32_t Money)
{
	uint32_t Timer;
	
	if (CardSetMoneyAndUnlock(cardid, Money)){//���������۷ѳɹ�
		return 1;
	}
	//��ʾ�����н���		
	DispOperationInfo();
	Timer = TimerRead();
	while((TimerRead() - Timer) < T1S * 10){
		OSTimeDlyHMSM (0, 0, 0, 20);
		if (CardSetMoneyAndUnlock(cardid, Money)){//���������۷ѳɹ�
			while((TimerRead() - Timer) < T1S * 2){
				OSTimeDlyHMSM (0, 0, 0, 20);
			}
			return 1;
		}		
	}
	return 0;
}




/************************************************************************************************************
** �� �� �� : LockedCardHandle
** �������� : ����������ز���
** ��    �� : cardid   ��ID   money  д�Ľ��
** ��    �� : ��
** ��    �� :	��
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
	if ((memcmp(TmpBuf, PileNumber, 4) == 0) || (FindLockCardRecord(cardid, &SerialNum) == 1)){//�ÿ�����׮��ס
		ret = 0;
		if (FindLockCardRecord(cardid, &SerialNum) == 1){
			index = GetChargeRecordIndex(0);
			for (i = 0; i < 100; i++){//�����100����¼�������
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
		if (ret == 1){//��¼��ѯ�ɹ�
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
		} else {//�Ҳ���������¼
			Beep(100);
			DispUnLockFailureInfo(UNLOCKCARDFAILCODE_NORECORD);
			OSTimeDlyHMSM (0, 0, 5, 0);
		}
	} else {//��ʾ����׮��ס
		Beep(100);	
		DispCardLockInfo(TmpBuf);
		OSTimeDlyHMSM (0, 0, 5, 0);
	}
}




/************************************************************************************************************
** �� �� �� : LockCardPro
** �������� : ���������������
** ��    �� : Cardid   ��ID
** ��    �� : ��
** ��    �� :	1 �����ɹ�  0 ʧ��
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
		SerialNum = InsertLockCardRecord(Cardid );			//�洢������¼		
		return 1;
	}
	//��ʾ�����н���		
	DispOperationInfo();
	while((TimerRead() - Timer) < T1S * 10){
		OSTimeDlyHMSM (0, 0, 0, 20);
		if( CardSetPileNumberAndLock(Cardid, PileNumber) == 1){
			SerialNum = InsertLockCardRecord(Cardid );			//�洢������¼	
			while((TimerRead() - Timer) < T1S * 2){
				OSTimeDlyHMSM (0, 0, 0, 20);
			}
			return 1;
		}
	}
	return 0;
}




/************************************************************************************************************
** �� �� �� : ConditionStop
** �������� : �жϳ���Ƿ��Զ�ֹͣ
** ��    �� : MainInf  �����ؽṹ��   port  0 ==Aǹ    1 ==Bǹ 
** ��    �� : ��
** ��    �� :	1 ֹͣ  0 ����
*************************************************************************************************************
*/
static uint8_t ConditionStop( MAININFO* MainInf ,uint8_t port)
{
	if ((MainInf->ChgDat[port].SumMoney + MINIMUM_CHARGE_MONEY) > MainInf->ChgDat[port].StartCardMoney){//�����
		MainInf->ChgDat[port].StopCause = CAUSE_MONEY_FAULT;
		MainInf->ChgDat[port].SumMoney = MainInf->ChgDat[port].StartCardMoney;
		return 1;	
	}else if ((MainInf->ChgDat[port].Mode == 1) && (MainInf->ChgDat[port].SumEnergy >= MainInf->ChgDat[port].Para)){//������
		MainInf->ChgDat[port].StopCause = CAUSE_USER_NORMAL_STOP;
		return 1;
	}else if ((MainInf->ChgDat[port].Mode == 2) && (MainInf->ChgDat[port].SumTime >= MainInf->ChgDat[port].Para)){//��ʱ��
		MainInf->ChgDat[port].StopCause = CAUSE_USER_NORMAL_STOP;
		return 1;
	}else if ((MainInf->ChgDat[port].Mode == 3) && (MainInf->ChgDat[port].SumMoney >= MainInf->ChgDat[port].Para)){//�����
		MainInf->ChgDat[port].StopCause = CAUSE_USER_NORMAL_STOP;
		return 1;
	
	}else if (ElmGetCommState(port) == 1){
		MainInf->ChgDat[port].StopCause = CAUSE_ELM_FAULT;
		return 1;
	}else
		return 0;
}



/************************************************************************************************************
** �� �� �� : OneGunHandle
** �������� : ��ǹ������
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
static void OneGunHandle(void)
{
	CHARGEINFO info[1];
	uint8_t TmpBuf[257],*pTmpBuf,PileNumber[32];
	static uint8_t cardinflag;
	uint8_t SwingOfflineCardFlag = 0;//ˢ���߿���־ 0δˢ�� 1ˢ��
	uint8_t SwingOnlineCardFlag = 0;//ˢ���߿���־ 0δˢ�� 1ˢ��
	uint8_t CardLockState;//����״̬ 1���� 2����
	uint32_t CardID;//����
	uint32_t CardBalance;//����� 2λС��
	uint8_t ret;
	uint32_t money;
	uint8_t QRCode[256];
	uint16_t Updata;
	StartingChg[PORTA] = FALSE;
	//��ȡ����Ϣ
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

	//�鿴��¼ʱ�䴦��
	if(IsLogin){
		if((TimerRead() - ShowTimer) > (T1S*60))
			IsLogin = 0;
	}
	ShowMenu();
	//����ʹ������
	JudgeLifeTime();
	ShowRecord();
	switch(OneGunState){
		case A_IDLE://����---------------------------------------------------------------------------------------------------------------------------------
			info[0] = GetChgInfo(0);
			Updata = GetUpdataPre();
			if(( Updata>> 8) == 1){   //������
				if(UpdataInFlag == 0)
					Beep(100);
				UpdataInFlag = 1;
				DispUpdataInfo(Updata&0xFF,(uint8_t* )"  ");
			}else if(UpdataInFlag){
				 //��ʾ����ʧ��
				Beep(100);
				UpdataInFlag = 0;
				DispUpdataInfo(Updata&0xFF,(uint8_t* )"����ʧ��");
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if ((info[0].ScramState == 1) || (info[0].DoorState == 1) || (info[0].CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (info[0].ErrTemp==1)|| (info[0].HighVol == 1)|| (info[0].LowVol == 1)){//���׮����
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
				if (SwingOfflineCardFlag == 1){   //A����B����ˢ���߿�
					SwingOfflineCardFlag = 0;
					if (CardLockState == 1){//����	
						//������ʾ����Ӧ����
						Beep(100);
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);	
					}
					if (CardLockState == 2){//������
						LockedCardHandle(CardID, CardBalance);
					}
				}
				if(SwingOnlineCardFlag == 1){
					SwingOnlineCardFlag = 0;
					//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}
				
			} else {//���׮����
				//����ָʾ����ʾ״̬
				SetLEDState(0, 0);
				
				//������ʾ������
				ParaGetQRCode(0, QRCode);
				ParaGetPileNo(0, PileNumber);
				strcat((char* )PileNumber,"A");
				DispAIdleInfo(QRCode, PileNumber, info[0].GunSeatState);
				
					//�жϲ�����ˢ���߿�����
				if (SwingOfflineCardFlag == 1){//ˢ���߿�
					SwingOfflineCardFlag = 0;
				
					if (CardLockState == 1){//������
						if (info[0].GunSeatState == 0){//δ��ǹ
							//������ʾ����Ӧ����
							Beep(100);
							
							//��ʾδ��ǹ
							DispInsertGunInfo();
							OSTimeDlyHMSM (0, 0, 5, 0);
							
						} else {						
							if( LockCardPro(CardID)){
								//������ʾ����Ӧ����
								Beep(100);
								//��ʼ��ǹ����								
								InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, 0);
								InsertChargingRecord(0, (uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));	
								
								//��ʾ�����
								DispRemainMoneyInfo(CardBalance);
								OSTimeDlyHMSM (0, 0, 2, 0);	
								
								//��ʾ�����н���
								DispStartChgInfo(0);
								OSTimeDlyHMSM (0, 0, 0, 500);
								
								//��������
								ret = StartCharge(0);
								if (ret == 0){//�����ɹ�
									OneGunState = A_CHARGE;
								} else {
									MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
									//���ɳ���¼,��ɾ������������
									ReportChargeData[0] = 1;
									InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
									DeleteChargingRecord(0);
									OneGunState = A_STOP;
									DispStartFailureInfo(); //��ʾ����ʧ��
									OSTimeDlyHMSM (0, 0, 2, 0);
								}
							}else{
								DispStartFailureInfo();
								OSTimeDlyHMSM (0, 0, 5, 0);
							}		
						}
					}
					if (CardLockState == 2){//������
						LockedCardHandle(CardID, CardBalance);
					}
				}	else if (SwingOnlineCardFlag == 1){//ˢ���߿�
					SwingOnlineCardFlag = 0;	
					if (info[0].GunSeatState == 0){//δ��ǹ
						//������ʾ����Ӧ����
						Beep(100);
						//��ʾδ��ǹ
						DispInsertGunInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
						
					} else {
						//��ȡ���߿���Ϣ
						if (GetOnlineCardInfo(0, CardID, &CardBalance) == 1){
							//��ʼ��ǹ����
							InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, 0);
							InsertChargingRecord(0, (uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));						
							//��ʾ�����
							DispVerifySuccesInfo(CardBalance );
							OSTimeDlyHMSM (0, 0, 2, 0);							
							//����ʾ��������н���
							DispStartChgInfo(0);
							OSTimeDlyHMSM (0, 0, 0, 500);						
							//�������
							ret = StartCharge(0);
							if (ret == 0){//�����ɹ�
								OneGunState = A_CHARGE;
							} else {
								MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
								//���ɳ���¼,��ɾ������������
								ReportChargeData[0] = 1;
								InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
								DeleteChargingRecord(0);
								OneGunState = A_STOP;
							}
						}else {
						
						}
					}
				} else if (SetMainChargeFlag[0] == 1) {//��̨����     				//�жϲ������̨����
					SetMainChargeFlag[0] = 0;				
					if (info[0].GunSeatState == 0){//δ��ǹ
						//������ʾ����Ӧ����
						Beep(100);
						//��ʾδ��ǹ
						DispInsertGunInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);		
					} else {
						//������ʾ����Ӧ��̨����
						Beep(100);
						//��ʼ��ǹ����
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[0], 0);
						InsertChargingRecord(0, (uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));						
						//��ʾ���
						DispRemainMoneyInfo(PlatformPara[0]);
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//����ʾ��������н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);				
						//�������
						ret = StartCharge(0);
						if (ret == 0){//�����ɹ�
							OneGunState = A_CHARGE;
						} else {
							MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
							//���ɳ���¼,��ɾ������������
							ReportChargeData[0] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
							DeleteChargingRecord(0);
							OneGunState = A_STOP;
						}
					}
					
				}
					
			}
			break;
		
		case A_CHARGE://���---------------------------------------------------------------------------------------------------------------------------------
			info[0] = GetChgInfo(0);
			//����ָʾ��״̬
			if ((info[0].ScramState == 1) || (info[0].DoorState == 1) || (info[0].CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (info[0].HighVol == 1)|| (info[0].LowVol == 1)){//���׮����
				SetLEDState(0, 2);
			} else {//���׮����
				SetLEDState(0, 1);
			}	
			UpdateChargeData(&MainInfo, info[0], 0);
			//������ʾ������
			DispAChargeInfo(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, MainInfo.GunInfo[0].CurrentA, MainInfo.GunInfo[0].CurrentB, MainInfo.GunInfo[0].CurrentC,\
																																								 MainInfo.GunInfo[0].VoltageA, MainInfo.GunInfo[0].VoltageB,  MainInfo.GunInfo[0].VoltageC,  MainInfo.ChgDat[0].SumTime);
		
			//�жϲ�����ˢ���߿�����
			if (SwingOfflineCardFlag == 1){//ˢ���߿�
				SwingOfflineCardFlag = 0;
			
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//ˢ��ֹͣ
					//����
					money = 0;
					if (CardBalance > MainInfo.ChgDat[0].SumMoney)
						money = CardBalance - MainInfo.ChgDat[0].SumMoney;
					if(UnLockCardPro(CardID, money)){
						DeleteLockCardRecord(CardID);	
						//������ʾ����Ӧ����
						Beep(100);
						//ֹͣ���
						StopCharge(0);
						MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;		
						//���ɳ���¼,��ɾ������������
						ReportChargeData[0] = 1;
						InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
						DeleteChargingRecord(0);
						//����״̬
						OneGunState = A_IDLE;
						//��ʾ�������
						DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
						OSTimeDlyHMSM (0, 0, 8, 0);
					}else{			
						Beep(100);
						//ֹͣ���
						StopCharge(0);
						MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;
						//����״̬
						OneGunState = A_STOP;					
						//���ɳ���¼,��ɾ������������
						ReportChargeData[0] = 1;
						InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
						DeleteChargingRecord(0);
					}
				} else if(CardLockState == 2){//�Ǳ������Ŀ�
					//�������������
					LockedCardHandle(CardID, CardBalance);
					
				} else {
					//������ʾ����Ӧ����
					Beep(100);			
					//��ʾ�޿��г�λ
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}
			
			//�жϲ�����ˢ���߿�����
			if (SwingOnlineCardFlag == 1){//ˢ���߿�
				SwingOnlineCardFlag = 0;
				
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//ˢ��ֹͣ
					//�������
					money = 0;
					if (MainInfo.ChgDat[0].StartCardMoney > MainInfo.ChgDat[0].SumMoney)
						money = MainInfo.ChgDat[0].StartCardMoney - MainInfo.ChgDat[0].SumMoney;
					
					//������ʾ����Ӧ����
					Beep(100);
					
					//ֹͣ���
					StopCharge(0);
					MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;
					//���ɳ���¼,��ɾ������������
					ReportChargeData[0] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
					DeleteChargingRecord(0);					
					//����״̬
					OneGunState = A_IDLE;
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else {
					//������ʾ����Ӧ����
					Beep(100);			
					//��ʾ�޿��г�λ
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}
			
			//�жϲ������ֹ̨ͣ����
			if ((SetMainChargeFlag[0] == 2) && (MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_PLATFORM)){//��ֹ̨ͣ
				SetMainChargeFlag[0] = 0;			
				//�������
				money = MainInfo.ChgDat[0].StartCardMoney - MainInfo.ChgDat[0].SumMoney;				
				//����������ʾ��̨�����ѱ�����
				Beep(100);				
				//ֹͣ���
				StopCharge(0);
				MainInfo.ChgDat[0].StopCause = CAUSE_USER_NORMAL_STOP;					
				//���ɳ���¼,��ɾ������������
				ReportChargeData[0] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
				DeleteChargingRecord(0);
				//����Ϊ����״̬
				OneGunState = A_IDLE;						
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}
			if (ConditionStop(&MainInfo, 0) == 1){//�ﵽ����
				//��������
				Beep(100);				
				//ֹͣ���
				StopCharge(0);
				//���ɳ���¼,��ɾ������������
				ReportChargeData[0] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
				DeleteChargingRecord(0);				
				//����״̬
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD))
					OneGunState = A_STOP;
				else
					OneGunState = A_IDLE;
			}		
			//�ж�Aǹ����Ƿ��Զ�ֹͣ
			if (info[0].ChargeState != 1){
					//�ռ�ֹͣԭ��
				MainInfo.ChgDat[0].StopCause = GetStopCause(0);//GetCCBStopCause(0);
				//���ɳ���¼,��ɾ������������
				ReportChargeData[0] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[0], sizeof(CHARGINGDATA));
				DeleteChargingRecord(0);
				//����״̬
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD))
					OneGunState = A_STOP;
				else
					OneGunState = A_IDLE;
			}			
			break;
			
		case A_STOP://ֹͣ---------------------------------------------------------------------------------------------------------------------------------
			//��ȡ������Ϣ
			info[0] = GetChgInfo(0);
			//����ָʾ��״̬
			if ((info[0].ScramState == 1) || (info[0].DoorState == 1) || (info[0].CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (info[0].HighVol == 1)|| (info[0].LowVol == 1)){//���׮����
				SetLEDState(0, 2);
			} else {//���׮����
				SetLEDState(0, 0);
			}
			//������ʾ������
			if( MainInfo.ChgDat[0].StartType ==  STARTCHARGETYPE_PLATFORM)
				DispAStopInfo((uint8_t *)"  ", (uint8_t *)stopcausetab[MainInfo.ChgDat[0].StopCause], MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney);		
			else
				DispAStopInfo((uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[0].StopCause], MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney);		
		
			
			if (info[0].GunSeatState == 0) {//δ��ǹ
				//����״̬
				OSTimeDlyHMSM (0, 0, 5, 0);
				OneGunState = A_IDLE;
			} 			
			//�жϲ�����ˢ���߿�����
			if (SwingOfflineCardFlag == 1){//ˢ���߿�
				SwingOfflineCardFlag = 0;			
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//ˢ��ֹͣ
					//����
					money = 0;
					if (CardBalance > MainInfo.ChgDat[0].SumMoney)
						money = CardBalance - MainInfo.ChgDat[0].SumMoney;
					if(UnLockCardPro(CardID, money)){
						Beep(100);					
						//ɾ���洢�������������Ϣ
						DeleteLockCardRecord(CardID);			
						//��ʾ������Ϣ
						DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
						OSTimeDlyHMSM (0, 0, 8, 0);
						//����״̬
						OneGunState = A_IDLE;
					}else{
//						OSTimeDlyHMSM (0, 0, 2, 0);
					}
				} else if(CardLockState == 2){//�Ǳ������Ŀ�
					//�������������
					LockedCardHandle(CardID, CardBalance);
				
				} else {
					//������ʾ����Ӧ����
					Beep(100);			
					//��ʾ�޿��г�λ
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}		
			//�жϲ�����ˢ���߿�����
			if (SwingOnlineCardFlag == 1){//ˢ���߿�
				SwingOnlineCardFlag = 0;		
				if ((MainInfo.ChgDat[0].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[0].StartCardID)){//ˢ��ֹͣ
					//�������
					money = 0;
					if (MainInfo.ChgDat[0].StartCardMoney > MainInfo.ChgDat[0].SumMoney)
						money = MainInfo.ChgDat[0].StartCardMoney - MainInfo.ChgDat[0].SumMoney;						
					//������ʾ����Ӧ����
					Beep(100);
					//����״̬
					OneGunState = A_IDLE;			
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[0].SumEnergy, MainInfo.ChgDat[0].SumMoney, money, MainInfo.ChgDat[0].SumTime, MainInfo.ChgDat[0].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				} else {
					//������ʾ����Ӧ����
					Beep(100);			
					//��ʾ�޿��г�λ
					DispNoPileUsed();
					OSTimeDlyHMSM (0, 0, 5, 0);
				}
			}			
			break;
	}
}


/************************************************************************************************************
** �� �� �� : JudgeErrInfo
** �������� : �ж��Ƿ������� ��д�������ʾ����
** ��    �� : CHARGEINFO   �����Ϣ�ṹ��  port == 0A    ===1B
** ��    �� : 1��������
** ��    �� :	��
*************************************************************************************************************
*/

static uint8_t JudgeErrInfo(CHARGEINFO Info, uint8_t port)
{

	uint8_t TmpBuf[56];
	uint8_t *pTmpBuf;
	if ((Info.ScramState == 1) || (Info.DoorState == 1) || (Info.CommunicateState == 1) || (ElmGetCommState(0) == 1) || (LifeTimeExpireFlag == 1) || (Info.HighVol == 1)|| (Info.LowVol == 1)){//���׮����
		memset(TmpBuf, 0, sizeof(TmpBuf));
		pTmpBuf = TmpBuf;
		pTmpBuf = pTmpBuf + sprintf((char *)pTmpBuf, "����:");
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
** �� �� �� : TwoGunHandle
** �������� : ˫ǹ������
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/


static void TwoGunHandle(void)
{
	CHARGEINFO info[2];
	uint8_t port = 0;
	uint32_t money;
	static uint8_t cardinflag;
	uint8_t SwingOfflineCardFlag = 0;//ˢ���߿���־ 0δˢ�� 1ˢ��
	uint8_t SwingOnlineCardFlag = 0;//ˢ���߿���־ 0δˢ�� 1ˢ��
	uint8_t CardLockState;//����״̬ 1���� 2����
	uint32_t CardID;//����
	uint32_t CardBalance;//����� 2λС��
	uint8_t ret;
	uint8_t ErrGun[2];
	uint16_t Updata;
	StartingChg[PORTA] = FALSE;
	StartingChg[PORTB] = FALSE;
	//��ȡ����Ϣ
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
	
	//�鿴��¼ʱ�䴦��
	if(IsLogin){
		if((TimerRead() - ShowTimer) > (T1S*60))
			IsLogin = 0;
	}
	//��ʾ�˵�����Ϣ
	ShowMenu();
	ShowRecord();
	//����ʹ������
	JudgeLifeTime();
		//��ȡ��������
	info[PORTA] = GetChgInfo(PORTA);
	info[PORTB] = GetChgInfo(PORTB);
	//����ָʾ��״̬
	ErrGun[PORTA] = JudgeErrInfo(info[PORTA], PORTA);
	ErrGun[PORTB] = JudgeErrInfo(info[PORTB], PORTB);
	//A���� B����--------------------------------------------------------------------------------------------------------------------------------------------
	if (TwoGunState == A_IDLE_B_IDLE){
		Updata = GetUpdataPre();
		if(( Updata>> 8) == 1){
			if(UpdataInFlag == 0)
				Beep(100);
			UpdataInFlag = 1;
			DispUpdataInfo(Updata&0xFF,(uint8_t* )"  ");
			
		}else if(UpdataInFlag){
			 //��ʾ����ʧ��
			Beep(100);
			UpdataInFlag = 0;
			DispUpdataInfo(Updata&0xFF,(uint8_t* )"����ʧ��");
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
			if (SwingOfflineCardFlag == 1){   //A����B����ˢ���߿�
				SwingOfflineCardFlag = 0;
				if (CardLockState == 1){//����			
					if ((info[0].GunSeatState == 0) && (info[1].GunSeatState == 0 ) && ((ErrGun[PORTA] + (ErrGun[PORTB]) != 2)) ){//δ��ǹ		
						//������ʾ����Ӧ����
						Beep(100);
						//��ʾδ��ǹ
						DispInsertGunInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);			
					} else if( ((ErrGun[PORTA] == 1) && (ErrGun[PORTB] == 1) ) ||((ErrGun[PORTA] == 1)&& (info[PORTA].GunSeatState == 1) && (info[PORTB].GunSeatState == 0))||((ErrGun[PORTB] == 1)&& (info[PORTB].GunSeatState == 1) && (info[PORTA].GunSeatState == 0)) ){	
						//������ʾ����Ӧ����
						Beep(100);
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);	
					}else { 
						if((info[PORTA].GunSeatState == 1) && (ErrGun[PORTA] == 0))//Ĭ�ϴ�Aǹ��ʼ
							port = PORTA;
						else if ((info[PORTB].GunSeatState == 1)&& (ErrGun[PORTB] == 0))
							port = PORTB;	
						if( LockCardPro(CardID)){
							//������ʾ����Ӧ����
							Beep(100);
							//��ʼ��ǹ����								
							InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, port);
							InsertChargingRecord(port, (uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));	
							
							//��ʾ�����
							DispRemainMoneyInfo(CardBalance);
							OSTimeDlyHMSM (0, 0, 2, 0);	
							
							//��ʾ�����н���
							DispStartChgInfo(0);
							OSTimeDlyHMSM (0, 0, 0, 500);
							
							//��������
							ret = StartCharge(port);
							if (ret == 0){//�����ɹ�
								if (port == PORTA)
									TwoGunState = A_CHARGE_B_IDLE;
								else
									TwoGunState = A_IDLE_B_CHARGE;
							} else {
								MainInfo.ChgDat[port].StopCause = GetStopCause(port);//GetCCBStopCause(0);
								//���ɳ���¼,��ɾ������������
								ReportChargeData[port] = 1;
								InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));
								DeleteChargingRecord(port);
								if (port == PORTA)
									TwoGunState = A_STOP_B_IDLE;
								else
									TwoGunState = A_IDLE_B_STOP;
								DispStartFailureInfo(); //��ʾ����ʧ��
								OSTimeDlyHMSM (0, 0, 2, 0);
							}
						}else{
							DispStartFailureInfo();
							OSTimeDlyHMSM (0, 0, 5, 0);
						}		
					}
				}
				
				if (CardLockState == 2){//������
					LockedCardHandle(CardID, CardBalance);
				}
			}else if (SwingOnlineCardFlag == 1){////A����B����ˢ���߿�
				SwingOnlineCardFlag = 0;	
				if ((info[0].GunSeatState == 0) && (info[1].GunSeatState == 0 ) && ((ErrGun[PORTA] + (ErrGun[PORTB]) != 2)) ){//δ��ǹ		
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				} else if( ((ErrGun[PORTA] == 1) && (ErrGun[PORTB] == 1) ) ||((ErrGun[PORTA] == 1)&& (info[PORTA].GunSeatState == 1) && (info[PORTB].GunSeatState == 0))||((ErrGun[PORTB] == 1)&& (info[PORTB].GunSeatState == 1) && (info[PORTA].GunSeatState == 0)) ){	
					//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}	else {
					if((info[PORTA].GunSeatState == 1) && (ErrGun[PORTA] == 0))//Ĭ�ϴ�Aǹ��ʼ
						port = PORTA;
					else if ((info[PORTB].GunSeatState == 1)&& (ErrGun[PORTB] == 0))
						port = PORTB;	
					//��ȡ���߿���Ϣ
					if (GetOnlineCardInfo(port, CardID, &CardBalance) == 1){
						//��ʼ��ǹ����
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, port);
						InsertChargingRecord(port, (uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));							
						//��ʾ�����
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//����ʾ��������н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//�������
						ret = StartCharge(port);
						if (ret == 0){//�����ɹ�
							if (port == 0)
								TwoGunState = A_CHARGE_B_IDLE;
							else
								TwoGunState = A_IDLE_B_CHARGE;
						} else {
							MainInfo.ChgDat[port].StopCause = GetStopCause(port);
							//���ɳ���¼,��ɾ������������
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
			}else if ((SetMainChargeFlag[PORTA] == 1) || (SetMainChargeFlag[PORTB] == 1)) {//��̨����     				//�жϲ������̨����
				if (SetMainChargeFlag[PORTA] == 1){
					SetMainChargeFlag[PORTA] = 0;
					port = PORTA;
				} else 	if (SetMainChargeFlag[PORTB] == 1){
					SetMainChargeFlag[PORTB] = 0;
					port = PORTB;
				}					
				//������ʾ����Ӧ��̨����
				Beep(100);
				//��ʼ��ǹ����
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[port], port);
				InsertChargingRecord(port, (uint8_t *)&MainInfo.ChgDat[port], sizeof(CHARGINGDATA));
				
				//��ʾ���
				DispRemainMoneyInfo(PlatformPara[port]);
				OSTimeDlyHMSM (0, 0, 2, 0);				
		
				//����ʾ��������н���
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//�������
				ret = StartCharge(port);
				if (ret == 0){//�����ɹ�
					if (port == 0)
						TwoGunState = A_CHARGE_B_IDLE;
					else
						TwoGunState = A_IDLE_B_CHARGE;
				} else {
					MainInfo.ChgDat[port].StopCause = GetStopCause(port);//GetCCBStopCause(0);
					//���ɳ���¼,��ɾ������������
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
	//Aǹ���� Bǹ���--------------------------------------------------------------------------------------------------------------------------------------------
	else if (TwoGunState == A_IDLE_B_CHARGE){
		//���³���е�����
		UpdateChargeData(&MainInfo, info[PORTB], PORTB);
		//������ʾ������
		DispAIdleBChgInfo(AQRCode, APileNumber, info[PORTA].GunSeatState, \
										 	MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, 
																																												 MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);			
		//����ָʾ��״̬
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 1);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}		
		//�жϲ�����ˢ���߿�����
		if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) && (CardLockState == LOCK)){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//������ʾ����Ӧ����
					Beep(100);
					//ֹͣ���
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;		
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);	
					//����״̬
					TwoGunState = A_IDLE_B_IDLE;	
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//ֹͣ���
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;		
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//����״̬
					TwoGunState = A_IDLE_B_STOP;	
				}
			} else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);
				
			} else {
				if(ErrGun[PORTA] ){
					//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0 ){
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);		
				}else{
					if( LockCardPro(CardID)){
						//������ʾ����Ӧ����
						Beep(100);
						//��ʼ��ǹ����								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTA);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));	
						
						//��ʾ�����
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);	
						
						//��ʾ�����н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);
						
						//��������
						ret = StartCharge(PORTA);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);//GetCCBStopCause(0);
							//���ɳ���¼,��ɾ������������
							ReportChargeData[PORTA] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTA);
							TwoGunState = A_STOP_B_CHARGE;
							DispStartFailureInfo(); //��ʾ����ʧ��
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
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//ˢ��ֹͣ
				//�������
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//������ʾ����Ӧ����
				Beep(100);				
				//ֹͣ���
				StopCharge(PORTB);		
				MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;	
				//���ɳ���¼,��ɾ������������
				ReportChargeData[PORTB] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTB);
				//����״̬
				TwoGunState = A_IDLE_B_IDLE;
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				if(ErrGun[PORTA] ){
							//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0  ){//δ��ǹ
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);					
				}else {		
					//��ȡ���߿���Ϣ
					if (GetOnlineCardInfo(PORTA, CardID, &CardBalance) == 1){
						//��ʼ��ǹ����
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTA);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));							
						//��ʾ�����
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//����ʾ��������н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//�������
						ret = StartCharge(PORTA);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);
							//���ɳ���¼,��ɾ������������
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
		else if (SetMainChargeFlag[PORTA] == 1) {//��̨����     				//�жϲ������̨����
			SetMainChargeFlag[PORTA] = 0;
			if(ErrGun[PORTA] ){
				//������ʾ����Ӧ����
				Beep(100);
				DispStartFailureInfo();
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTA].GunSeatState == 1){
				//������ʾ����Ӧ��̨����
				Beep(100);
				//��ʼ��ǹ����
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTA], PORTA);
				InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));			
				//��ʾ���
				DispRemainMoneyInfo(PlatformPara[PORTA]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//����ʾ��������н���
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//�������
				ret = StartCharge(PORTA);
				if (ret == 0){//�����ɹ�
					TwoGunState = A_CHARGE_B_CHARGE;
				} else {
					MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);//GetCCBStopCause(0);
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					TwoGunState = A_STOP_B_CHARGE;
				}	
			}	else{
			//��ʾ��ǹ
			}
		}else if ((SetMainChargeFlag[PORTB] == 2) && (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_PLATFORM)){//��ֹ̨ͣ
			SetMainChargeFlag[PORTB] = 0;			
			//�������
			money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;			
			//����������ʾ��̨�����ѱ�����
			Beep(100);			
			//ֹͣ���
			StopCharge(PORTB);
			MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;		
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//����״̬
			TwoGunState = A_IDLE_B_IDLE;
			//��ʾ�������
			DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTB)){
			Beep(100);
			//ֹͣ���
			StopCharge(PORTB);
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//����״̬
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_IDLE_B_STOP;
			else
				TwoGunState = A_IDLE_B_IDLE;	

		}else if (info[PORTB].ChargeState != 1){
			//�ռ�ֹͣԭ��
			MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);	
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);						
			//����״̬
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
					(uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);
		}

		//����ָʾ��״̬
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		if (info[PORTB].GunSeatState == 0) {//δ��ǹ
			//����״̬
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_IDLE;
		} 			
		//�жϲ�����ˢ���߿�����
		else if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//ˢ��ֹͣ
				//����
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//ɾ���洢�������������Ϣ
					DeleteLockCardRecord(CardID);			
					//��ʾ������Ϣ
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//����״̬
					TwoGunState = A_IDLE_B_IDLE;
				}else{
//						OSTimeDlyHMSM (0, 0, 2, 0);
				}
			} else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);		
			} else {
				if(ErrGun[PORTA] ){
					//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0 ){
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);		
				}else{
					if( LockCardPro(CardID)){
						//������ʾ����Ӧ����
						Beep(100);
						//��ʼ��ǹ����								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTA);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));							
						//��ʾ�����
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//��ʾ�����н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);					
						//��������
						ret = StartCharge(PORTA);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_CHARGE_B_STOP;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);
							//���ɳ���¼,��ɾ������������
							ReportChargeData[PORTA] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTA);
							TwoGunState = A_STOP_B_STOP;
							DispStartFailureInfo(); //��ʾ����ʧ��
							OSTimeDlyHMSM (0, 0, 2, 0);
						}
					}else{
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
					}		
				}
			}
		}else if (SwingOnlineCardFlag == 1){//ˢ���߿�
			SwingOnlineCardFlag = 0;		
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//������ʾ����Ӧ����
				Beep(100);
				//����״̬
				TwoGunState = A_IDLE_B_IDLE;
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			} else {
				if(ErrGun[PORTA] ){
					//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTA].GunSeatState == 0  ){//δ��ǹ
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);					
				}else {				
					//��ȡ���߿���Ϣ
					if (GetOnlineCardInfo(PORTA	, CardID, &CardBalance) == 1){
						//��ʼ��ǹ����
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTA	);
						InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));							
						//��ʾ�����
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//����ʾ��������н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//�������
						ret = StartCharge(PORTA);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_CHARGE_B_STOP;
						} else {
							MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);
							//���ɳ���¼,��ɾ������������
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
		}else if (SetMainChargeFlag[PORTA] == 1) {//��̨����     				//�жϲ������̨����
			SetMainChargeFlag[PORTA] = 0;
			if(ErrGun[PORTA] ){
							//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTA].GunSeatState == 1){
				//������ʾ����Ӧ��̨����
				Beep(100);
				//��ʼ��ǹ����
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTA], PORTA);
				InsertChargingRecord(PORTA, (uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));			
				//��ʾ���
				DispRemainMoneyInfo(PlatformPara[PORTA]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//����ʾ��������н���
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//�������
				ret = StartCharge(PORTA);
				if (ret == 0){//�����ɹ�
					TwoGunState = A_CHARGE_B_STOP;
				} else {
					MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);//GetCCBStopCause(0);
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					TwoGunState = A_STOP_B_STOP;
				}	
			}	else{
			//��ʾ��ǹ
			}
		}
	}
	else if(TwoGunState == A_CHARGE_B_IDLE){
		//���³���е�����
		UpdateChargeData(&MainInfo, info[PORTA], PORTA);			
		//������ʾ������
		DispAChgBIdleInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC, 
																																												 MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime, \
											BQRCode, BPileNumber, info[PORTB].GunSeatState);
		//����ָʾ��״̬
		SetLEDState(PORTA, 1);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		//�жϲ�����ˢ���߿�����
		if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//������ʾ����Ӧ����
					Beep(100);
					//ֹͣ���
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;			
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//����״̬
					TwoGunState = A_IDLE_B_IDLE;	
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//ֹͣ���
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;			
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//����״̬
					TwoGunState = A_STOP_B_IDLE;
				}
			} else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);			
			} else {
				if(ErrGun[PORTB] ){	
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0 ){
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);		
				}else{
					if( LockCardPro(CardID)){
						//������ʾ����Ӧ����
						Beep(100);
						//��ʼ��ǹ����								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTB);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));						
						//��ʾ�����
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);						
						//��ʾ�����н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);					
						//��������
						ret = StartCharge(PORTB);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);//GetCCBStopCause(0);
							//���ɳ���¼,��ɾ������������
							ReportChargeData[PORTB] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTB);
							TwoGunState = A_CHARGE_B_STOP;
							DispStartFailureInfo(); //��ʾ����ʧ��
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
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//������ʾ����Ӧ����
				Beep(100);				
				//ֹͣ���
				StopCharge(PORTA);	
				MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;				
				//���ɳ���¼,��ɾ������������
				ReportChargeData[PORTA] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTA);
				//����״̬
				TwoGunState = A_IDLE_B_IDLE;
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				if(ErrGun[PORTB] ){
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0  ){//δ��ǹ
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				}else {		
					//��ȡ���߿���Ϣ
					if (GetOnlineCardInfo(PORTB, CardID, &CardBalance) == 1){
						//��ʼ��ǹ����
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTB);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));							
						//��ʾ�����
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//����ʾ��������н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//�������
						ret = StartCharge(PORTB);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_CHARGE_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);
							//���ɳ���¼,��ɾ������������
							ReportChargeData[PORTB] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTB);
							TwoGunState = A_CHARGE_B_STOP;
						}
					}else {
					
					}
				}
			}
		}else if (SetMainChargeFlag[PORTB] == 1) {//��̨����     				//�жϲ������̨����
			SetMainChargeFlag[PORTB] = 0;
			if(ErrGun[PORTB] ){
				Beep(100);
				DispStartFailureInfo();
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTB].GunSeatState == 1){	
				Beep(100);
				//��ʼ��ǹ����
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTB], PORTB);
				InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));			
				//��ʾ���
				DispRemainMoneyInfo(PlatformPara[PORTB]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//����ʾ��������н���
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//�������
				ret = StartCharge(PORTB);
				if (ret == 0){//�����ɹ�
					TwoGunState = A_CHARGE_B_CHARGE;
				} else {
					MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);//GetCCBStopCause(0);
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					TwoGunState = A_CHARGE_B_STOP;
				}	
			}	else{
			//��ʾ��ǹ
			}
		}else if ((SetMainChargeFlag[PORTA] == 2) && (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_PLATFORM)){//��ֹ̨ͣ
			SetMainChargeFlag[PORTA] = 0;			
			//�������
			money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;		
			//����������ʾ��̨�����ѱ�����
			Beep(100);			
			//ֹͣ���
			StopCharge(PORTA);
			MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;	
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//����Ϊ����״̬
			TwoGunState = A_IDLE_B_IDLE;
			//��ʾ�������
			DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}	else if(ConditionStop(&MainInfo, PORTA)){
			Beep(100);
			//ֹͣ���
			StopCharge(PORTA);
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//����״̬
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_IDLE;
			else
				TwoGunState = A_IDLE_B_IDLE;	
		}else if (info[PORTA].ChargeState != 1){
			//�ռ�ֹͣԭ��
			MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);	
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);							
			//����״̬
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_IDLE;
			else
				TwoGunState = A_IDLE_B_IDLE;
		}
	}
	else if(TwoGunState == A_CHARGE_B_CHARGE){
		//���³���е�����
		UpdateChargeData(&MainInfo, info[PORTA], PORTA);
		UpdateChargeData(&MainInfo, info[PORTB], PORTB);		
		DispAChgBChgInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC,
																																												MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime,
										 MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, 
																																												MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);
		//����ָʾ��״̬
		SetLEDState(PORTA, 1);
		SetLEDState(PORTB, 1);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		//�жϲ�����ˢ���߿�����
		if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) ){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//������ʾ����Ӧ����
					Beep(100);
					//ֹͣ���
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//����״̬
					TwoGunState = A_IDLE_B_CHARGE;		
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//ֹͣ���
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//����״̬
					TwoGunState = A_STOP_B_CHARGE;	
				}
			}else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) && (CardLockState == LOCK)){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//������ʾ����Ӧ����
					Beep(100);
					//ֹͣ���
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;					
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//����״̬
					TwoGunState = A_CHARGE_B_IDLE;		
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//ֹͣ���
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;	
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//����״̬
					TwoGunState = A_CHARGE_B_STOP;		
				}
			}	else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);		
			} else {
				//������ʾ����Ӧ����
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}else if (SwingOnlineCardFlag == 1){//A���B����ˢ���߿�
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//������ʾ����Ӧ����
				Beep(100);				
				//ֹͣ���
				StopCharge(PORTA);
				MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;	
				//���ɳ���¼,��ɾ������������
				ReportChargeData[PORTA] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTA);				
				//����״̬
				TwoGunState = A_IDLE_B_CHARGE;
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//������ʾ����Ӧ����
				Beep(100);				
				//ֹͣ���
				StopCharge(PORTB);
				MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
				//���ɳ���¼,��ɾ������������
				ReportChargeData[PORTB] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTB);								
				//����״̬
				TwoGunState = A_CHARGE_B_IDLE;
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}
			else {
				//������ʾ����Ӧ����
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}	else if ((SetMainChargeFlag[PORTA] == 2) && (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_PLATFORM)){//��ֹ̨ͣ
			SetMainChargeFlag[PORTA] = 0;			
			//�������
			money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;				
			//����������ʾ��̨�����ѱ�����
			Beep(100);				
			//ֹͣ���
			StopCharge(PORTA);
			MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;			
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);
				//����Ϊ����״̬
			TwoGunState = A_IDLE_B_CHARGE;			
			//��ʾ�������
			DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTA)){
			Beep(100);
			//ֹͣ���
			StopCharge(PORTA);
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//����״̬
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_CHARGE;
			else
				TwoGunState = A_IDLE_B_CHARGE;	

		}	else if (info[PORTA].ChargeState != 1){
			//�ռ�ֹͣԭ��
			MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);	
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);				
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_CHARGE;
			else
				TwoGunState = A_IDLE_B_CHARGE;
		}	else if ((SetMainChargeFlag[PORTB] == 2) && (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_PLATFORM)){//��ֹ̨ͣ
			SetMainChargeFlag[PORTB] = 0;
			//�������
			money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;
			Beep(100);			
			//ֹͣ���
			StopCharge(PORTB);
			MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//����Ϊ����״̬
			TwoGunState = A_CHARGE_B_IDLE;	
			//��ʾ�������
			DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTB)){
			Beep(100);
			//ֹͣ���
			StopCharge(PORTB);
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);		
			//����״̬
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_CHARGE_B_STOP;
			else
				TwoGunState = A_CHARGE_B_IDLE;	
		}else if (info[PORTB].ChargeState != 1){
			//�ռ�ֹͣԭ��
			MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);		
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);				
			//����״̬
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_CHARGE_B_STOP;
			else
				TwoGunState = A_CHARGE_B_IDLE;
		}				
	}
	else if(TwoGunState == A_CHARGE_B_STOP){
		//���³���е�����
		UpdateChargeData(&MainInfo, info[PORTA], PORTA);		
		if( MainInfo.ChgDat[PORTB].StartType ==  STARTCHARGETYPE_PLATFORM){
			DispAChgBStopInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC, \
																																													 MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime, \
												(uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);
		}else{
			DispAChgBStopInfo(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, MainInfo.GunInfo[PORTA].CurrentA, MainInfo.GunInfo[PORTA].CurrentB, MainInfo.GunInfo[PORTA].CurrentC, \
																																											 MainInfo.GunInfo[PORTA].VoltageA, MainInfo.GunInfo[PORTA].VoltageB, MainInfo.GunInfo[PORTA].VoltageC,  MainInfo.ChgDat[PORTA].SumTime, \
										(uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);
		}
				//����ָʾ��״̬
		SetLEDState(PORTA, 1);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}		
		if (info[PORTB].GunSeatState == 0) {//δ��ǹ
			//����״̬
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_CHARGE_B_IDLE;
		}else if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) && (CardLockState == LOCK)){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//������ʾ����Ӧ����
					Beep(100);
					//ֹͣ���
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//����״̬
					TwoGunState = A_IDLE_B_STOP;		
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//ֹͣ���
					StopCharge(PORTA);
					MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTA] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTA);
					//����״̬
					TwoGunState = A_STOP_B_STOP;	
				}
			}else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) ){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//ɾ���洢�������������Ϣ
					DeleteLockCardRecord(CardID);			
					//��ʾ������Ϣ
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//����״̬
					TwoGunState = A_CHARGE_B_IDLE;
				}else{
					
				}
			}
			else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);				
			} else {
					//������ʾ����Ӧ����
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}else if (SwingOnlineCardFlag == 1){//A���ֹͣˢ���߿�
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				//�������
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//������ʾ����Ӧ����
				Beep(100);				
				//ֹͣ���
				StopCharge(PORTA);
				MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
				//���ɳ���¼,��ɾ������������
				ReportChargeData[PORTA] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTA);					
				//����״̬
				TwoGunState = A_IDLE_B_STOP;
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//����״̬
				TwoGunState = A_CHARGE_B_IDLE;
				//������ʾ����Ӧ����
				Beep(100);
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				//������ʾ����Ӧ����
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}	else if ((SetMainChargeFlag[PORTA] == 2) && (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_PLATFORM)){//��ֹ̨ͣ
			SetMainChargeFlag[PORTA] = 0;			
			//�������
			money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;		
			//����������ʾ��̨�����ѱ�����
			Beep(100);			
			//ֹͣ���
			StopCharge(PORTA);
			MainInfo.ChgDat[PORTA].StopCause = CAUSE_USER_NORMAL_STOP;		
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);
			//����Ϊ����״̬
			TwoGunState = A_IDLE_B_STOP;			
			//��ʾ�������
			DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTA)){
			Beep(100);
			//ֹͣ���
			StopCharge(PORTA);
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);			
			//����״̬
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_STOP;
			else
				TwoGunState = A_IDLE_B_STOP;	
		}else if (info[PORTA].ChargeState != 1){
			//�ռ�ֹͣԭ��
			MainInfo.ChgDat[PORTA].StopCause = GetStopCause(PORTA);	
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTA] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTA], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTA);							
			//����״̬
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
			DispAStopBIdleInfo((uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,			
										BQRCode, BPileNumber, info[PORTB].GunSeatState);
		}
		//����ָʾ��״̬
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		
		if (info[PORTA].GunSeatState == 0) {//δ��ǹ
			//����״̬
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_IDLE;
		}else if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//ɾ���洢�������������Ϣ
					DeleteLockCardRecord(CardID);			
					//��ʾ������Ϣ
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//����״̬
					TwoGunState = A_IDLE_B_IDLE;
				}else{
 
				}
			} else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);	
			} else {
				if(ErrGun[PORTB] ){
					//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0 ){
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				}else{
					if( LockCardPro(CardID)){
						//������ʾ����Ӧ����
						Beep(100);
						//��ʼ��ǹ����								
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_OFFLINECARD, CardID, CardBalance, PORTB);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));						
						//��ʾ�����
						DispRemainMoneyInfo(CardBalance);
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//��ʾ�����н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);					
						//��������
						ret = StartCharge(PORTB);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_STOP_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);
							//���ɳ���¼,��ɾ������������
							ReportChargeData[PORTB] = 1;
							InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
							DeleteChargingRecord(PORTB);
							TwoGunState = A_STOP_B_STOP;
							DispStartFailureInfo(); //��ʾ����ʧ��
							OSTimeDlyHMSM (0, 0, 2, 0);
						}
					}else{
						DispStartFailureInfo();
						OSTimeDlyHMSM (0, 0, 5, 0);
					}		
				}
			}
		}else if (SwingOnlineCardFlag == 1){//ˢ���߿�
			SwingOnlineCardFlag = 0;		
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				//�������
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//����״̬
				TwoGunState = A_IDLE_B_IDLE;
				//������ʾ����Ӧ����
				Beep(100);
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			} else {
				if(ErrGun[PORTB] ){
							//������ʾ����Ӧ����
					Beep(100);
					DispStartFailureInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);	
				}else if (info[PORTB].GunSeatState == 0  ){//δ��ǹ
					//������ʾ����Ӧ����
					Beep(100);
					//��ʾδ��ǹ
					DispInsertGunInfo();
					OSTimeDlyHMSM (0, 0, 5, 0);			
				}else {				
					//��ȡ���߿���Ϣ
					if (GetOnlineCardInfo(PORTB	, CardID, &CardBalance) == 1){
						//��ʼ��ǹ����
						InitStartChgDate( &MainInfo, STARTCHARGETYPE_ONLINECARD, CardID, CardBalance, PORTB	);
						InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));							
						//��ʾ�����
						DispVerifySuccesInfo(CardBalance );
						OSTimeDlyHMSM (0, 0, 2, 0);							
						//����ʾ��������н���
						DispStartChgInfo(0);
						OSTimeDlyHMSM (0, 0, 0, 500);							
						//�������
						ret = StartCharge(PORTB);
						if (ret == 0){//�����ɹ�
							TwoGunState = A_STOP_B_CHARGE;
						} else {
							MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);
							//���ɳ���¼,��ɾ������������
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
		}else if (SetMainChargeFlag[PORTB] == 1) {//��̨����     				//�жϲ������̨����
			SetMainChargeFlag[PORTB] = 0;
			if(ErrGun[PORTB] ){
				//������ʾ����Ӧ����
				Beep(100);
				DispStartFailureInfo();
				OSTimeDlyHMSM (0, 0, 5, 0);	
			}else if(info[PORTB].GunSeatState == 1){
				//������ʾ����Ӧ��̨����
				Beep(100);
				//��ʼ��ǹ����
				InitStartChgDate( &MainInfo, STARTCHARGETYPE_PLATFORM, 0, PlatformPara[PORTB], PORTB);
				InsertChargingRecord(PORTB, (uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));			
				//��ʾ���
				DispRemainMoneyInfo(PlatformPara[PORTB]);
				OSTimeDlyHMSM (0, 0, 2, 0);						
				//����ʾ��������н���
				DispStartChgInfo(0);
				OSTimeDlyHMSM (0, 0, 0, 500);				
				//�������
				ret = StartCharge(PORTB);
				if (ret == 0){//�����ɹ�
					TwoGunState = A_STOP_B_CHARGE;
				} else {
					MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);//GetCCBStopCause(0);
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					TwoGunState = A_STOP_B_STOP;
				}	
			}	else{
			//��ʾ��ǹ
			}
		}			
	
	}
	else if(TwoGunState == A_STOP_B_CHARGE){
		//���³���е�����
		UpdateChargeData(&MainInfo, info[PORTB], PORTB);
		if( MainInfo.ChgDat[PORTA].StartType ==  STARTCHARGETYPE_PLATFORM){
				DispAStopBChgInfo((uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,\
											MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, \
																																												 MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);
		}else{
			DispAStopBChgInfo((uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,\
											MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, MainInfo.GunInfo[PORTB].CurrentA, MainInfo.GunInfo[PORTB].CurrentB, MainInfo.GunInfo[PORTB].CurrentC, \
																																												 MainInfo.GunInfo[PORTB].VoltageA, MainInfo.GunInfo[PORTB].VoltageB, MainInfo.GunInfo[PORTB].VoltageC,  MainInfo.ChgDat[PORTB].SumTime);
		}	
		//����ָʾ��״̬
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 1);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}			
		if (info[PORTA].GunSeatState == 0) {//δ��ǹ
			//����״̬
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_CHARGE;
		}else if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) && (CardLockState == LOCK)){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					DeleteLockCardRecord(CardID);	
					//������ʾ����Ӧ����
					Beep(100);
					//ֹͣ���
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//����״̬
					TwoGunState = A_STOP_B_IDLE;		
					//��ʾ�������
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
				}else{			
					Beep(100);
					//ֹͣ���
					StopCharge(PORTB);
					MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;			
					//���ɳ���¼,��ɾ������������
					ReportChargeData[PORTB] = 1;
					InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
					DeleteChargingRecord(PORTB);
					//����״̬
					TwoGunState = A_STOP_B_STOP;
				}
			}else if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) ){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//ɾ���洢�������������Ϣ
					DeleteLockCardRecord(CardID);			
					//��ʾ������Ϣ
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//����״̬
					TwoGunState = A_IDLE_B_CHARGE;
				}else{
 
				}
			}else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);	
			} else {
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}
		else if (SwingOnlineCardFlag == 1){//A���ֹͣˢ���߿�
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//������ʾ����Ӧ����
				Beep(100);				
					//ֹͣ���
				StopCharge(PORTB);
				MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;
				//���ɳ���¼,��ɾ������������
				ReportChargeData[PORTB] = 1;
				InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
				DeleteChargingRecord(PORTB);
				TwoGunState = A_STOP_B_IDLE;
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//����״̬
				TwoGunState = A_IDLE_B_CHARGE;
				//������ʾ����Ӧ����
				Beep(100);
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}else if ((SetMainChargeFlag[PORTB] == 2) && (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_PLATFORM)){//��ֹ̨ͣ
			SetMainChargeFlag[PORTB] = 0;
			money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;			
			//����������ʾ��̨�����ѱ�����
			Beep(100);			
			//ֹͣ���
			StopCharge(PORTB);
			MainInfo.ChgDat[PORTB].StopCause = CAUSE_USER_NORMAL_STOP;	
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);		
			//����״̬
			TwoGunState = A_STOP_B_IDLE;				
			//��ʾ�������
			DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
			OSTimeDlyHMSM (0, 0, 8, 0);
		}else if(ConditionStop(&MainInfo, PORTB)){
			Beep(100);
			//ֹͣ���
			StopCharge(PORTB);
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);			
			//����״̬
			if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) || (MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD))
				TwoGunState = A_STOP_B_STOP;
			else
				TwoGunState = A_STOP_B_IDLE;	
		}else if (info[PORTB].ChargeState != 1){
			//�ռ�ֹͣԭ��
			MainInfo.ChgDat[PORTB].StopCause = GetStopCause(PORTB);		
			//���ɳ���¼,��ɾ������������
			ReportChargeData[PORTB] = 1;
			InsertChargeRecord((uint8_t *)&MainInfo.ChgDat[PORTB], sizeof(CHARGINGDATA));
			DeleteChargingRecord(PORTB);						
			//����״̬
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
												 (uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);	
	}else if( (MainInfo.ChgDat[PORTA].StartType !=  STARTCHARGETYPE_PLATFORM) && (MainInfo.ChgDat[PORTB].StartType ==  STARTCHARGETYPE_PLATFORM)){		
			DispAStopBStopInfo((uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,
												 (uint8_t *)"    ", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);	
	}else if( (MainInfo.ChgDat[PORTA].StartType !=  STARTCHARGETYPE_PLATFORM) && (MainInfo.ChgDat[PORTB].StartType !=  STARTCHARGETYPE_PLATFORM)){		
			DispAStopBStopInfo((uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTA].StopCause], MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney,
												 (uint8_t *)"��ˢ������", (uint8_t *)stopcausetab[MainInfo.ChgDat[PORTB].StopCause], MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney);	
	}

		//����ָʾ��״̬
		SetLEDState(PORTA, 0);
		SetLEDState(PORTB, 0);	
		if(ErrGun[PORTA]){
			SetLEDState(PORTA, 2);
		}
		if(ErrGun[PORTB]){
			SetLEDState(PORTB, 2);
		}	
		if (info[PORTA].GunSeatState == 0) {//δ��ǹ
			//����״̬
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_IDLE_B_STOP;
		}else if(info[PORTB].GunSeatState == 0){
			OSTimeDlyHMSM (0, 0, 5, 0);
			TwoGunState = A_STOP_B_IDLE;
		}			
		//�жϲ�����ˢ���߿�����
		else if (SwingOfflineCardFlag == 1){//ˢ���߿�
			SwingOfflineCardFlag = 0;		
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID) && (CardLockState == LOCK)){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTA].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTA].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//ɾ���洢�������������Ϣ
					DeleteLockCardRecord(CardID);			
					//��ʾ������Ϣ
					DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//����״̬
					TwoGunState = A_IDLE_B_STOP;
				}else{

				}
			}else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_OFFLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID) ){//ˢ��ֹͣ
				money = 0;
				if (CardBalance > MainInfo.ChgDat[PORTB].SumMoney)
					money = CardBalance - MainInfo.ChgDat[PORTB].SumMoney;
				if(UnLockCardPro(CardID, money)){
					Beep(100);					
					//ɾ���洢�������������Ϣ
					DeleteLockCardRecord(CardID);			
					//��ʾ������Ϣ
					DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
					OSTimeDlyHMSM (0, 0, 8, 0);
					//����״̬
					TwoGunState = A_STOP_B_IDLE;
				}else{
   
				}
			}else if(CardLockState == 2){//�Ǳ������Ŀ�
				//�������������
				LockedCardHandle(CardID, CardBalance);
			} else {
					//������ʾ����Ӧ����
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}	else if (SwingOnlineCardFlag == 1){//A���ֹͣˢ���߿�
			SwingOnlineCardFlag = 0;	
			if ((MainInfo.ChgDat[PORTA].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTA].StartCardID)){//ˢ��ֹͣ
				//�������
				money = 0;
				if (MainInfo.ChgDat[PORTA].StartCardMoney > MainInfo.ChgDat[PORTA].SumMoney)
					money = MainInfo.ChgDat[PORTA].StartCardMoney - MainInfo.ChgDat[PORTA].SumMoney;	
				//����״̬
				TwoGunState = A_IDLE_B_STOP;
				//������ʾ����Ӧ����
				Beep(100);
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTA].SumEnergy, MainInfo.ChgDat[PORTA].SumMoney, money, MainInfo.ChgDat[PORTA].SumTime, MainInfo.ChgDat[PORTA].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}	else if ((MainInfo.ChgDat[PORTB].StartType == STARTCHARGETYPE_ONLINECARD) && (CardID == MainInfo.ChgDat[PORTB].StartCardID)){//ˢ��ֹͣ
				money = 0;
				if (MainInfo.ChgDat[PORTB].StartCardMoney > MainInfo.ChgDat[PORTB].SumMoney)
					money = MainInfo.ChgDat[PORTB].StartCardMoney - MainInfo.ChgDat[PORTB].SumMoney;	
				//����״̬
				TwoGunState = A_STOP_B_IDLE;
				//������ʾ����Ӧ����
				Beep(100);
				//��ʾ�������
				DispAccount(MainInfo.ChgDat[PORTB].SumEnergy, MainInfo.ChgDat[PORTB].SumMoney, money, MainInfo.ChgDat[PORTB].SumTime, MainInfo.ChgDat[PORTB].StopDateTime);
				OSTimeDlyHMSM (0, 0, 8, 0);
			}else {
				Beep(100);			
				//��ʾ�޿��г�λ
				DispNoPileUsed();
				OSTimeDlyHMSM (0, 0, 5, 0);
			}
		}
	}
}

/************************************************************************************************************
** �� �� �� : BoardRunLedControl
** �������� : ��������ָʾ�ƿ��ƺ��� 1���� 1����
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void BoardRunLedControl(void)
{
	static uint8_t state,poweron = 0x5a;
	static uint32_t timer;
	uint8_t i;
	
	if (poweron == 0x5a){
		poweron = 0;
		//����ָʾ���ϵ����10��
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
** �� �� �� : LEDBoardControl
** �������� : ��ɫ�ư���ƺ���
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
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
	if ((LEDState[0] == 2) || ((LEDState[1] == 2))){//����
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
	if (PortNum == 1){//��ǹ
		if (LEDState[0] == 1){//Aǹ���
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
		} else if (LEDState[0] == 0){//Aǹ����
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
		}else if(LEDState[0] == 2){ //����
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
	} else {//˫ǹ
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
		}else{//Aǹ����
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
		}else{//Bǹ����
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
** �� �� �� : SetMainCharge
** �������� : ��������纯��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......  setcmd SET_START_CHARGE/SET_STOP_CHARGE
							mode ģʽ 0���� 1������ 2��ʱ�� 3�����
							para ���� ������ʱ����Ϊ0.01Ԫ ������ʱ����Ϊ0.01kWh ��ʱ��ʱ����Ϊ�� �����ʱ����Ϊ0.01Ԫ
							CardOrDiscernNumber ��翨��/�û�ʶ��� 32���ֽ�ASCII
							userid �û�ID
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void SetMainCharge(uint8_t port, uint8_t setcmd, uint8_t mode, uint32_t para, uint8_t *CardOrDiscernNumber, uint8_t *userid)
{
	if (port < 2){
		if (setcmd == SET_START_CHARGE){
			SetMainChargeFlag[port] = 1;
			PlatformMode[port] = mode;//ƽ̨����ʱ��ģʽ 0���� 1������ 2��ʱ�� 3�����
			PlatformPara[port] = para;//ƽ̨����ʱ�Ĳ��� ������ʱ����Ϊ0.01Ԫ ������ʱ����Ϊ0.01kWh ��ʱ��ʱ����Ϊ�� �����ʱ����Ϊ0.01Ԫ
			memcpy(PlatformCardOrDiscernNumber[port], CardOrDiscernNumber, 32);//ƽ̨����ʱ�ĳ�翨��/�û�ʶ���
			memcpy(PlatformUserID[port], userid, 2);//�û�ID
		} else {
			SetMainChargeFlag[port] = 2;
		}
	}
}

/************************************************************************************************************
** �� �� �� : GetMainCharge
** �������� : ��ȡ�����״̬
** ��    �� : ��
** ��    �� : ��
** ��    �� :	STATE_IDLE/STATE_CHARGE/STATE_COMPLETE
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
			if (TwoGunState == A_IDLE_B_IDLE					)//Aǹ���� Bǹ����
				return STATE_IDLE;
			else if(TwoGunState == A_IDLE_B_CHARGE		)//Aǹ���� Bǹ���
				return STATE_IDLE;
			else if(TwoGunState == A_IDLE_B_STOP			)//Aǹ���� Bǹֹͣ
				return STATE_IDLE;
			else if(TwoGunState == A_CHARGE_B_IDLE		)//Aǹ��� Bǹ����
				return STATE_CHARGE;
			else if(TwoGunState == A_CHARGE_B_CHARGE	)//Aǹ��� Bǹ���
				return STATE_CHARGE;
			else if(TwoGunState == A_CHARGE_B_STOP		)//Aǹ��� Bǹֹͣ
				return STATE_CHARGE;
			else if(TwoGunState == A_STOP_B_IDLE			)//Aǹֹͣ Bǹ����
				return STATE_COMPLETE;
			else if(TwoGunState == A_STOP_B_CHARGE		)//Aǹֹͣ Bǹ���
				return STATE_COMPLETE;
			else if(TwoGunState == A_STOP_B_STOP			)//Aǹֹͣ Bǹֹͣ
				return STATE_COMPLETE;
		} else if(port == 1){
			if (TwoGunState == A_IDLE_B_IDLE					)//Aǹ���� Bǹ����
				return STATE_IDLE;
			else if(TwoGunState == A_IDLE_B_CHARGE		)//Aǹ���� Bǹ���
				return STATE_CHARGE;
			else if(TwoGunState == A_IDLE_B_STOP			)//Aǹ���� Bǹֹͣ
				return STATE_COMPLETE;
			else if(TwoGunState == A_CHARGE_B_IDLE		)//Aǹ��� Bǹ����
				return STATE_IDLE;
			else if(TwoGunState == A_CHARGE_B_CHARGE	)//Aǹ��� Bǹ���
				return STATE_CHARGE;
			else if(TwoGunState == A_CHARGE_B_STOP		)//Aǹ��� Bǹֹͣ
				return STATE_COMPLETE;
			else if(TwoGunState == A_STOP_B_IDLE			)//Aǹֹͣ Bǹ����
				return STATE_IDLE;
			else if(TwoGunState == A_STOP_B_CHARGE		)//Aǹֹͣ Bǹ���
				return STATE_CHARGE;
			else if(TwoGunState == A_STOP_B_STOP			)//Aǹֹͣ Bǹֹͣ
				return STATE_COMPLETE;
		}
	}
	return 0;
}



/************************************************************************************************************
** �� �� �� : GetChargeRecord
** �������� : ���ڴ��л�ȡ����¼����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ  ChgData  �洢�ĳ���¼���ݵ�ַ
** ��    �� : ��
** ��    �� :	0  = ��ȡʧ��   1 = ��ȡ�ɹ� 
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
** �� �� �� : MainTask
** �������� : ������
** ��    �� : pdata δʹ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/


void MainTask(void *pdata)
{
	uint8_t i = 10;
	uint8_t PortNum = ParaGetChgGunNumber();
	Beep(100);
	while(i){
		DispStartDeviceInfo(_VERSION_MAJOR, _VERSION_MINOR, _VERSION_PATCH);	//��ʾ���ؽ���
		OSTimeDlyHMSM (0, 0, 0, 500);
		i--;
	}
	if(GetUpdata() == 0xAA){
		SetUpdataFlag(0);
		DispUpdataInfo(100, (uint8_t* )"�����ɹ� ");
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
		
		LEDBoardControl();  //��ɫ��

		
		OSTimeDlyHMSM (0, 0, 0, 10);
	}
}

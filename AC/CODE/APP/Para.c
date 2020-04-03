#include "App.h"
#include "EEPROM.h"
#include "UART.h"
#include <LPC177x_8x.H>
#include "RTC.h"
#include "Para.h"
#include <string.h>
#include "cfg.h"
#include "Board.h"
#include <stdio.h>
#include "includes.h"
#include "record.h"
#include "Timer.h"
#include "EXT_Chgtask.h"
#include "MyAlgLib.h"
#include "stdlib.h"
#include "ethernet.h"

static PARAINFO ParaInfo;
static CFGINFO CfgInfo;

static uint8_t Paracmd[2];
static uint8_t ParaResult[2];
static uint16_t CfgPwm[2];

static uint32_t RunHour;

/************************************************************************************************************
** �� �� �� : GetParaCmd
** �������� : ��ȡ��������
** ��    �� : port  A  Bǹ  
** ��    �� : ��
** ��    �� :	1  ��������PWM����  2  ���ͻ�ȡPWM����
*************************************************************************************************************
*/
uint8_t GetParaCmd(uint8_t port)
{
	if(Paracmd[port] == 1){
		Paracmd[port] = 0;
		return 1;
	}else if(Paracmd[port] == 2){
		Paracmd[port] = 0;
		return 2;
	}else{
		Paracmd[port] = 0;
		return 0;
	}

}

/************************************************************************************************************
** �� �� �� : GetCfgPwm
** �������� : ��ȡ���ù����·���PWM ����
** ��    �� : port  A  Bǹ  
** ��    �� : ��
** ��    �� :	���ù����·���PWM ����
*************************************************************************************************************
*/
uint16_t GetCfgPwm(uint8_t port)
{
	return CfgPwm[port];
}



/************************************************************************************************************
** �� �� �� : SetParaResult
** �������� : �������ý��
** ��    �� : port  A  Bǹ   Result    ���  0ʧ��    1�ɹ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void SetParaResult(uint8_t port, uint8_t Result)
{
	ParaResult[port] = Result;
}



/************************************************************************************************************
** �� �� �� : GetUpdata
** �������� : ��ȡ������־
** ��    �� : ��
** ��    �� : ��
** ��    �� :	0x5a ����   0  ������
*************************************************************************************************************
*/
uint32_t GetUpdata(void)
{
	EepromRead(0, EEPROMADDR_SAVEINFO, &CfgInfo, MODE_8_BIT, sizeof(CFGINFO));
	return CfgInfo.Updata;
}


/************************************************************************************************************
** �� �� �� : CfgSave
** �������� : �������
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
static void CfgSave(void)
{
	//Ӧ���ӻ������ź���
	EepromWrite(0, EEPROMADDR_SAVEINFO, &CfgInfo, MODE_8_BIT, sizeof(CFGINFO));
}

/************************************************************************************************************
** �� �� �� : ParaGetPwm
** �������� : ��ȡ PWM���β���
** ��    �� : port   Aǹ ��Bǹ  
** ��    �� : ��
** ��    �� :	PWM���β���
*************************************************************************************************************
*/
uint16_t ParaGetPwm(uint8_t port)
{
#if EXT_BOARD
	uint32_t TimerSend;
	Paracmd[port] = 0x02;
	TimerSend = TimerRead();
	ParaResult[port] = 0;
	while((ParaResult[port] == 0) && (TimerRead()- TimerSend) < T1S*4){
		OSTimeDlyHMSM (0, 0, 0, 10);
	}
	if(ParaResult[port]){
		return ReadPwm(port);
	}else{
		return 0;
	}	
#else
	return CfgInfo.Pwm;
#endif
}



/************************************************************************************************************
** �� �� �� : ParaSetPwm
** �������� : ���� PWM���β���
** ��    �� : port  Aǹ ��Bǹ     Pwm    PWMռ�ձ�
** ��    �� : ��
** ��    �� :	
*************************************************************************************************************
*/
static uint8_t ParaSetPwm(uint8_t port,uint16_t Pwm)
{
#if EXT_BOARD
	uint32_t TimerSend;
	CfgPwm[port] = Pwm;
	Paracmd[port] = 0x01;
	TimerSend = TimerRead();
	ParaResult[port] = 0;
	while((ParaResult[port] == 0) && (TimerRead()- TimerSend) < T1S*4){
		OSTimeDlyHMSM (0, 0, 0, 10);
	}
	if(ParaResult[port])
		return 1;
	else
		return 0;
#else
	CfgInfo.Pwm = Pwm;
	CfgSave();
	return 1;
#endif
	
}


/************************************************************************************************************
** �� �� �� : SetUpdataFlag
** �������� : ���ø��±�ʶ
** ��    �� : flag
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void SetUpdataFlag(uint32_t flag)
{

	CfgInfo.Updata = flag;
	CfgSave();
}
/************************************************************************************************************
** �� �� �� : SaveInfoDefault
** �������� : ������ʼ��
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void SaveInfoDefault(void)
{
	memset(&CfgInfo,0,sizeof(CFGINFO));
	CfgInfo.Updata = 0;
	CfgInfo.Pwm = 5400;
	memcpy(CfgInfo.ActPassword, "0000", 4);
	CfgInfo.LimitHour = 0;
	RunHour = 0;
}


/************************************************************************************************************
** �� �� �� : SaveInfoV2Def
** �������� : ��������������ʼ��
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void SaveInfoV2Def(void){
	memcpy(CfgInfo.ActPassword, "0000", 4);
	CfgInfo.LimitHour = 0;
	RunHour = 0;	
}



/************************************************************************************************************
** �� �� �� : ParaDefault
** �������� : ����Ĭ�ϲ���
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
static void ParaDefault(void)
{
	uint8_t port,i;
	
	memset(&ParaInfo,0,sizeof(PARAINFO));
	
	ParaInfo.version = 0x5a5a5a02;
	
	//��ǹ����
	ParaInfo.ChgGunNumber = 1;
	
	//����ͨ�� Ĭ��1��̫��
	ParaInfo.UpChannel = 1;
	
	//����IP
	ParaInfo.LocalIp[0] = 192;
	ParaInfo.LocalIp[1] = 168;
	ParaInfo.LocalIp[2] = 1;
	ParaInfo.LocalIp[3] = 10;
	
	//����
	ParaInfo.GateWay[0] = 192;
	ParaInfo.GateWay[1] = 168;
	ParaInfo.GateWay[2] = 1;
	ParaInfo.GateWay[3] = 1;
	
	//��������
	ParaInfo.SubMask[0] = 255;
	ParaInfo.SubMask[1] = 255;
	ParaInfo.SubMask[2] = 255;
	ParaInfo.SubMask[3] = 0;
	
	//������ַ
	ParaInfo.PhyMac[0] = 0x00;
	ParaInfo.PhyMac[1] = 0x00;
	ParaInfo.PhyMac[2] = 0x10;
	ParaInfo.PhyMac[3] = 0x00;
	ParaInfo.PhyMac[4] = 0x00;
	ParaInfo.PhyMac[5] = 0x00;
	
	//Ӳ���汾��
	memset(ParaInfo.HardVersion, 0, 16);
	
	//�ͻ����
	memset(ParaInfo.CustomerNumber, 0, 6);
	
	memset(ParaInfo.APN, 0, 16); //APN
	memset(ParaInfo.APNUser, 0, 32); //APN�û���
	memset(ParaInfo.APNPasswd, 0, 32); //APN����
	
	ParaInfo.ScreenOpenHour = 18; //���������ʱ�� Сʱ
	ParaInfo.ScreenOpenMin = 0; //���������ʱ�� ����
	ParaInfo.ScreenStopHour = 6; //������ر�ʱ�� Сʱ
	ParaInfo.ScreenStopMin = 0; //������ر�ʱ�� ����
	
	for(port = 0; port < 2; port++){
		if(port == 0){
			memcpy(ParaInfo.Port[port].PileNo, "0000000000000001", 16);
		}else{
			memcpy(ParaInfo.Port[port].PileNo, "0000000000000002", 16);
		}
		memset(ParaInfo.Port[port].ACMeterAddr, 0, 6);
		memset(ParaInfo.Port[port].DCMeterAddr, 0, 6);
		ParaInfo.Port[port].LoginTimeInterval = 30;
		ParaInfo.Port[port].NetHeartTime = 20;
		ParaInfo.Port[port].NetCommOverCnt = 3;
		ParaInfo.Port[port].ServerIp[0] = 39;
		ParaInfo.Port[port].ServerIp[1] = 108;
		ParaInfo.Port[port].ServerIp[2] = 62;
		ParaInfo.Port[port].ServerIp[3] = 139;
		ParaInfo.Port[port].ServerPort = 18090;
		for (i = 0; i < 12; i++){
			ParaInfo.Port[port].StartHour[i] = 0;
			ParaInfo.Port[port].StartMin[i] = 0; //�ƷѲ��� ��ʼ����
			ParaInfo.Port[port].StopHour[i] = 0; //�ƷѲ��� ����Сʱ
			ParaInfo.Port[port].StopMin[i] = 0; //�ƷѲ��� ��������
			ParaInfo.Port[port].Money[i] = 0; //�ƷѲ��� ���� ÿ�ȵ�ĵ�� ����5λС��
		}
		
		ParaInfo.Port[port].StateInfoReportPeriod = 15;
		//memcpy(ParaInfo.Port[port].QrData, ParaInfo.Port[port].PileNo, 32);
	}
	
	memset(ParaInfo.ManufactureDate, 0, 4); //��������
	memset(ParaInfo.ManufactureBase, 0, 16); //��������
	memset(ParaInfo.TestWorker, 0, 8); //�����
	memset(ParaInfo.LifeTime, 0, 5); //ʹ����
}

/************************************************************************************************************
** �� �� �� : ParaInit
** �������� : ������ʼ��
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaInit(void)
{
	uint32_t i;
	
	if (sizeof(PARAINFO) > 1024){
		print("para length fail\r\n");
		while(1);
	}
	EepromRead(0, EEPROMADDR_PARAINFO, &ParaInfo, MODE_8_BIT, 1024);
	EepromRead(0, EEPROMADDR_SAVEINFO, &CfgInfo, MODE_8_BIT, sizeof(CFGINFO));
	EepromRead(0, EEPROMADDR_SAVERUNHOUR, &RunHour, MODE_8_BIT, sizeof(RunHour));
	if (ParaInfo.version != 0x5a5a5a02){
		if (ParaInfo.version == 0x5a5a5a01){
			ParaInfo.version = 0x5a5a5a02;
			SaveInfoV2Def();
			EepromWrite(0, EEPROMADDR_PARAINFO, &ParaInfo, MODE_8_BIT, sizeof(PARAINFO));
			EepromErase(EEPROMADDR_SAVEINFO);
			EepromWrite(0, EEPROMADDR_SAVEINFO, &CfgInfo, MODE_8_BIT, sizeof(CFGINFO));
			EepromErase(EEPROMADDR_SAVERUNHOUR);
			EepromWrite(0, EEPROMADDR_SAVERUNHOUR, &RunHour, MODE_8_BIT, sizeof(RunHour));			
		}else{
			ParaDefault(); //Ĭ�ϲ���	
			for (i = 0; i< 16; i++){
				EepromErase(EEPROMADDR_PARAINFO + i);
			}		
			EepromWrite(0, EEPROMADDR_PARAINFO, &ParaInfo, MODE_8_BIT, sizeof(PARAINFO));		
			SaveInfoDefault();
			EepromErase(EEPROMADDR_SAVEINFO);
			EepromWrite(0, EEPROMADDR_SAVEINFO, &CfgInfo, MODE_8_BIT, sizeof(CFGINFO));
			EepromErase(EEPROMADDR_SAVERUNHOUR);
			EepromWrite(0, EEPROMADDR_SAVERUNHOUR, &RunHour, MODE_8_BIT, sizeof(RunHour));			
		}	
	}
	ParaInfo.Port[0].StateInfoReportPeriod = 15;	
}

/************************************************************************************************************
** �� �� �� : ParaSave
** �������� : �������
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSave(void)
{
	//Ӧ���ӻ������ź���
	EepromWrite(0, EEPROMADDR_PARAINFO, &ParaInfo, MODE_8_BIT, sizeof(PARAINFO));
}

/************************************************************************************************************
** �� �� �� : ParaSetChgGunNumber
** �������� : ���ó�ǹ����
** ��    �� : number ���� 1��2
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetChgGunNumber(uint8_t number)
{
	if ((number != 1) && (number != 2)){
		return;
	}
	
	ParaInfo.ChgGunNumber = number;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetChgGunNumber
** �������� : ��ȡ��ǹ����
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��ǹ���� 1��2
*************************************************************************************************************
*/
uint8_t ParaGetChgGunNumber(void)
{
	return ParaInfo.ChgGunNumber;
}

/************************************************************************************************************
** �� �� �� : ParaSetPileNo
** �������� : ���ó��׮��� 32���ֽ�
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......  data ���׮��� 32���ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetPileNo(uint8_t port, uint8_t *data)
{
	if (data == NULL){
		return;
	}
	memcpy(ParaInfo.Port[port].PileNo, data, 32);
	
	sprintf((char *)ParaInfo.Port[0].QrData, "http://chargepile.tomorn.cn/%sA", data);
	sprintf((char *)ParaInfo.Port[1].QrData, "http://chargepile.tomorn.cn/%sB", data);
	
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetPileNo
** �������� : ��ȡ���׮��� 32���ֽ�
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......  
** ��    �� : ���׮���
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetPileNo(uint8_t port, uint8_t *data)
{
	if (data == NULL){
		return;
	}
	memcpy(data, ParaInfo.Port[port].PileNo, 32);
}

/************************************************************************************************************
** �� �� �� : ParaSetLoginTimeInterval
** �������� : ����ǩ�����ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... TimeInterval ����
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetLoginTimeInterval(uint8_t port, uint16_t TimeInterval)
{
	ParaInfo.Port[port].LoginTimeInterval = TimeInterval;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetLoginTimeInterval
** �������� : ��ȡǩ�����ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	ʱ�� ����
*************************************************************************************************************
*/
uint16_t ParaGetLoginTimeInterval(uint8_t port)
{
	return ParaInfo.Port[port].LoginTimeInterval;
}

/************************************************************************************************************
** �� �� �� : ParaSetACMeterAddr
** �������� : ���ý��������ַ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   data �����ַ data[0]ΪA0
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetACMeterAddr(uint8_t port, uint8_t *data)
{
	memcpy(ParaInfo.Port[port].ACMeterAddr, data, 6);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetACMeterAddr
** �������� : ��ȡ���������ַ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   data �����ַ data[0]ΪA0
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetACMeterAddr(uint8_t port, uint8_t *data)
{
	memcpy(data, ParaInfo.Port[port].ACMeterAddr, 6);
}

/************************************************************************************************************
** �� �� �� : ParaSetDCMeterAddr
** �������� : ����ֱ�������ַ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   data �����ַ data[0]ΪA0
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetDCMeterAddr(uint8_t port, uint8_t *data)
{
	memcpy(ParaInfo.Port[port].DCMeterAddr, data, 6);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetDCMeterAddr
** �������� : ��ȡֱ�������ַ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   data �����ַ data[0]ΪA0
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetDCMeterAddr(uint8_t port, uint8_t *data)
{
	memcpy(data, ParaInfo.Port[port].DCMeterAddr, 6);
}

/************************************************************************************************************
** �� �� �� : ParaSetNetHeartTime
** �������� : ������������ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   time ��λ S
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetNetHeartTime(uint8_t port, uint16_t time)
{
	if (time == 0){
		return;
	}
	ParaInfo.Port[port].NetHeartTime=time;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetNetHeartTime
** �������� : ��ȡ��������ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   cnt �ط�����
** ��    �� : ��
** ��    �� :	����ʱ��  ��λ S
*************************************************************************************************************
*/
uint16_t ParaGetNetHeartTime(uint8_t port)
{
	return ParaInfo.Port[port].NetHeartTime;
}

/************************************************************************************************************
** �� �� �� : ParaSetNetCommOverCnt
** �������� : ��������ͨѶ��ʱ�ط����� 
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   cnt �ط�����
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetNetCommOverCnt(uint8_t port, uint8_t cnt)
{
	if (cnt == 0) {
		return;
	}
	ParaInfo.Port[port].NetCommOverCnt=cnt;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetNetCommOverCnt
** �������� : ��ȡ����ͨѶ��ʱ�ط�����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   cnt �ط�����
** ��    �� : ��
** ��    �� :	�ط�����
*************************************************************************************************************
*/
uint8_t ParaGetNetCommOverCnt(uint8_t port)
{
	return ParaInfo.Port[port].NetCommOverCnt;
}

/************************************************************************************************************
** �� �� �� : ParaSetUpChannel
** �������� : ��������ͨ��
** ��    �� : channel 0��̫�� 1GPRS
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetUpChannel(uint8_t channel)
{
	ParaInfo.UpChannel = channel;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetUpChannel
** �������� : ��ȡ����ͨ��
** ��    �� : ��
** ��    �� : ��
** ��    �� :	0��̫�� 1GPRS
*************************************************************************************************************
*/
uint8_t ParaGetUpChannel(void)
{
	return ParaInfo.UpChannel;
}

/************************************************************************************************************
** �� �� �� : ParaSetLocalIp
** �������� : ���ñ���IP 4���ֽ�
** ��    �� : IP
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetLocalIp(uint8_t *data)
{
	if (data == NULL){
		return;
	}

	memcpy(ParaInfo.LocalIp,data,4);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetLocalIp
** �������� : ��ȡ����IP 4���ֽ�
** ��    �� : IP
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetLocalIp(uint8_t *data)
{
	if (data == NULL){
		return;
	}
	
	memcpy(data, ParaInfo.LocalIp, 4);
}

/************************************************************************************************************
** �� �� �� : ParaSetSubMask
** �������� : ������������  4���ֽ�
** ��    �� : ��������
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetSubMask(uint8_t *data)
{
	if (data == NULL){
		return;
	}
	
	memcpy(ParaInfo.SubMask,data,4);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetSubMask
** �������� : ��ȡ�������� 4���ֽ�
** ��    �� : ����ָ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetSubMask(uint8_t *data)
{
	if (data == NULL){
		return;
	}
	
	memcpy(data, ParaInfo.SubMask, 4);
}

/************************************************************************************************************
** �� �� �� : ParaSetGateWay
** �������� : ��������  4���ֽ�
** ��    �� : ����ָ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetGateWay(uint8_t *data)
{
	if (data == NULL){
		return;
	}

	memcpy(ParaInfo.GateWay,data,4);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetGateWay
** �������� : ��ȡ���� 4���ֽ�
** ��    �� : ����ָ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetGateWay(uint8_t *data)
{
	if (data == NULL){
		return;
	}
	
	memcpy(data,ParaInfo.GateWay,4);
}

/************************************************************************************************************
** �� �� �� : ParaSetPhyMac
** �������� : ��������MAC  6���ֽ�
** ��    �� : ����MAC
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetPhyMac(uint8_t *data)
{
	if (data == NULL){
		return;
	}

	memcpy(ParaInfo.PhyMac,data,6);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetPhyMac
** �������� : ��ȡ����MAC 6���ֽ�
** ��    �� : ����ָ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetPhyMac(uint8_t *data)
{
	if (data == NULL){
		return;
	}
	
	memcpy(data, ParaInfo.PhyMac, 6);
}

/************************************************************************************************************
** �� �� �� : ParaSetServerIp
** �������� : ���÷�����IP
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   data ������IP
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetServerIp(uint8_t port, uint8_t *data)
{
	if (data == NULL){
		return;
	}
	memcpy(ParaInfo.Port[port].ServerIp, data, 4);
}

/************************************************************************************************************
** �� �� �� : ParaGetServerIp
** �������� : ��ȡ������IP
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   data ������IP
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetServerIp(uint8_t port, uint8_t *data)
{
	if (data == NULL){
		return;
	}
	memcpy(data, ParaInfo.Port[port].ServerIp, 4);
}

/************************************************************************************************************
** �� �� �� : ParaSetServerPort
** �������� : ���÷������˿�
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   port �������˿�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetServerPort(uint8_t port, uint16_t serverport)
{
	if (port >= 2){
		return;
	}
	ParaInfo.Port[port].ServerPort = serverport;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetServerPort
** �������� : ��ȡ�������˿�
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   
** ��    �� : ��
** ��    �� :	�������˿�
*************************************************************************************************************
*/
uint16_t ParaGetServerPort(uint8_t port)
{
	if (port >= 2){
		return 0;
	}
	return ParaInfo.Port[port].ServerPort;
}

/************************************************************************************************************
** �� �� �� : ParaSetFeilvTime
** �������� : ���üƷѲ��Ե�ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   
							period 12��ʱ�� 0-11
							starthour ��ʼСʱ 0-24
							startmin ��ʼ���� 0��30
							stophour ����Сʱ 0-24
							stopmin �������� 0��30
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetFeilvTime(uint8_t port, uint8_t period, uint8_t starthour, uint8_t startmin, uint8_t stophour, uint8_t stopmin)
{
	if ((port >= 2) || (period >= 12)){
		return;
	}
	ParaInfo.Port[port].StartHour[period] = starthour;
	ParaInfo.Port[port].StartMin[period] = startmin;
	ParaInfo.Port[port].StopHour[period] = stophour;
	ParaInfo.Port[port].StopMin[period] = stopmin;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetFeilvTime
** �������� : ��ȡ�ƷѲ��Ե�ʱ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......    period 12��ʱ�� 0-11
** ��    �� : starthour ��ʼСʱ 0-24
							startmin ��ʼ���� 0��30
							stophour ����Сʱ 0-24
							stopmin �������� 0��30
** ��    �� :	�������˿�
*************************************************************************************************************
*/
void ParaGetFeilvTime(uint8_t port, uint8_t period, uint8_t *starthour, uint8_t *startmin, uint8_t *stophour, uint8_t *stopmin)
{
	if ((port >= 2) || (period >= 12)){
		return;
	}
	*starthour = ParaInfo.Port[port].StartHour[period];
	*startmin = ParaInfo.Port[port].StartMin[period];
	*stophour = ParaInfo.Port[port].StopHour[period];
	*stopmin = ParaInfo.Port[port].StopMin[period];
}

/************************************************************************************************************
** �� �� �� : ParaSetFeilvMoney
** �������� : ���üƷѲ��Եĵ���
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   
							period 12��ʱ�� 0-11
							money ���� ����5λС��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetFeilvMoney(uint8_t port, uint8_t period, uint32_t money)
{
	if ((port >= 2) || (period >= 12)){
		return;
	}
	ParaInfo.Port[port].Money[period] = money;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetFeilvMoney
** �������� : ��ȡ�ƷѲ��Եĵ���
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......    period 12��ʱ�� 0-11
** ��    �� : ��
** ��    �� :	money ���� ����5λС��
*************************************************************************************************************
*/
uint32_t ParaGetFeilvMoney(uint8_t port, uint8_t period)
{
	if ((port >= 2) || (period >= 12)){
		return 0;
	}
	return ParaInfo.Port[port].Money[period];
}

/************************************************************************************************************
** �� �� �� : ParaSetStateInfoReportPeriod
** �������� : ����״̬��Ϣ�ϱ�����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   time ��λ S
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetStateInfoReportPeriod(uint8_t port, uint16_t time)
{
	if (time == 0){
		return;
	}
	ParaInfo.Port[port].StateInfoReportPeriod = time;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetStateInfoReportPeriod
** �������� : ��ȡ״̬��Ϣ�ϱ�����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......
** ��    �� : ��
** ��    �� :	����ʱ��  ��λ S
*************************************************************************************************************
*/
uint16_t ParaGetStateInfoReportPeriod(uint8_t port)
{
	return ParaInfo.Port[port].StateInfoReportPeriod;
}

/************************************************************************************************************
** �� �� �� : ParaSetQRCode
** �������� : ���ö�ά��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ......   qrcode ��ά�룬�256���ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetQRCode(uint8_t port, uint8_t *qrcode)
{
	memcpy(ParaInfo.Port[port].QrData, qrcode, 256);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetQRCode
** �������� : ��ȡ��ά��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... qrcode ��ά�룬�256���ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetQRCode(uint8_t port, uint8_t *qrcode)
{
	memcpy(qrcode, ParaInfo.Port[port].QrData, 256);
}

/************************************************************************************************************
** �� �� �� : ParaSetHardVersion
** �������� : ����Ӳ���汾��
** ��    �� : data �汾�� ASCII 16�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetHardVersion(uint8_t *data)
{
	memcpy(ParaInfo.HardVersion, data, 16);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetHardVersion
** �������� : ��ȡӲ���汾��
** ��    �� : data �汾�� ASCII 16�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetHardVersion(uint8_t *data)
{
	memcpy(data, ParaInfo.HardVersion, 16);
}

/************************************************************************************************************
** �� �� �� : ParaSetHardVersion
** �������� : ���ÿͻ����
** ��    �� : data �ͻ���� 6�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetCustomerNumber(uint8_t *data)
{
	memcpy(ParaInfo.CustomerNumber, data, 4);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetCustomerNumber
** �������� : ��ȡ�ͻ����
** ��    �� : data �ͻ���� 6�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetCustomerNumber(uint8_t *data)
{
	memcpy(data, ParaInfo.CustomerNumber, 4);
}

/************************************************************************************************************
** �� �� �� : ParaSetAPN
** �������� : ����APN
** ��    �� : data �ͻ���� 16�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetAPN(uint8_t *data)
{
	memcpy(ParaInfo.APN, data, 16);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetAPN
** �������� : ��ȡAPN
** ��    �� : data 6�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetAPN(uint8_t *data)
{
	memcpy(data, ParaInfo.APN, 16);
}

/************************************************************************************************************
** �� �� �� : ParaSetAPNUser
** �������� : ����APN�û���
** ��    �� : data 32�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetAPNUser(uint8_t *data)
{
	memcpy(ParaInfo.APNUser, data, 32);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetAPNUser
** �������� : ��ȡAPN�û���
** ��    �� : data 6�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetAPNUser(uint8_t *data)
{
	memcpy(data, ParaInfo.APNUser, 32);
}

/************************************************************************************************************
** �� �� �� : ParaSetAPNPasswd
** �������� : ����APN����
** ��    �� : data 32�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetAPNPasswd(uint8_t *data)
{
	memcpy(ParaInfo.APNPasswd, data, 32);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetAPNPasswd
** �������� : ��ȡAPN����
** ��    �� : data 6�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetAPNPasswd(uint8_t *data)
{
	memcpy(data, ParaInfo.APNPasswd, 32);
}

/************************************************************************************************************
** �� �� �� : ParaSetFeilvTime
** �������� : ���üƷѲ��Ե�ʱ��
** ��    �� : openhour ����Сʱ 0-24
							openmin �������� 0��30
							closehour �ر�Сʱ 0-24
							closemin �رշ��� 0��30
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetScreenTime(uint8_t openhour, uint8_t openmin, uint8_t closehour, uint8_t closemin)
{
	ParaInfo.ScreenOpenHour = openhour;
	ParaInfo.ScreenOpenMin = openmin;
	ParaInfo.ScreenStopHour = closehour;
	ParaInfo.ScreenStopMin = closemin;
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetScreenTime
** �������� : ��ȡ����������ر�ʱ��
** ��    �� : ��
** ��    �� : openhour ����Сʱ 0-24
							openmin �������� 0��30
							closehour �ر�Сʱ 0-24
							closemin �رշ��� 0��30
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetScreenTime(uint8_t *openhour, uint8_t *openmin, uint8_t *closehour, uint8_t *closemin)
{
	*openhour = ParaInfo.ScreenOpenHour;
	*openmin = ParaInfo.ScreenOpenMin;
	*closehour = ParaInfo.ScreenStopHour;
	*closemin = ParaInfo.ScreenStopMin;
}

/************************************************************************************************************
** �� �� �� : ParaSetManufactureDate
** �������� : ���ó�������
** ��    �� : Date ���� 4�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetManufactureDate(uint8_t *Date)
{
	memcpy(ParaInfo.ManufactureDate, Date, 4);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetManufactureDate
** �������� : ��ȡ��������
** ��    �� : ��
** ��    �� : Date ���� 4�ֽ�
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetManufactureDate(uint8_t *Date)
{
	memcpy(Date, ParaInfo.ManufactureDate, 4);
}

/************************************************************************************************************
** �� �� �� : ParaSetManufactureBase
** �������� : ������������
** ��    �� : Base ���� 16�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetManufactureBase(uint8_t *Base)
{
	memcpy(ParaInfo.ManufactureBase, Base, 16);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetManufactureBase
** �������� : ��ȡ��������
** ��    �� : ��
** ��    �� : Base ���� 16�ֽ�
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetManufactureBase(uint8_t *Base)
{
	memcpy(Base, ParaInfo.ManufactureBase, 16);
}

/************************************************************************************************************
** �� �� �� : ParaSetTestWorker
** �������� : ���ü�����
** ��    �� : TestWorker ������ 8�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetTestWorker(uint8_t *TestWorker)
{
	memcpy(ParaInfo.TestWorker, TestWorker, 8);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetManufactureBase
** �������� : ��ȡ������
** ��    �� : ��
** ��    �� : TestWorker ������ 8�ֽ�
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetTestWorker(uint8_t *TestWorker)
{
	memcpy(TestWorker, ParaInfo.TestWorker, 8);
}

/************************************************************************************************************
** �� �� �� : ParaSetLifeTime
** �������� : ����ʹ������
** ��    �� : LifeTime ʹ������ 5�ֽ�
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSetLifeTime(uint8_t *LifeTime)
{
	memcpy(ParaInfo.LifeTime, LifeTime, 5);
	ParaSave();
}

/************************************************************************************************************
** �� �� �� : ParaGetLifeTime
** �������� : ��ȡʹ������
** ��    �� : ��
** ��    �� : LifeTime ʹ������ 5�ֽ�
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetLifeTime(uint8_t *LifeTime)
{
	memcpy(LifeTime, ParaInfo.LifeTime, 5);
}



/************************************************************************************************************
** �� �� �� : ParaGetLimitHour
** �������� : ��ȡ��������ʹ�õ�ʱ������λ�� Сʱ
** ��    �� : ��
** ��    �� : ��
** ��    �� :	���õ�ʱ������λ��Сʱ
*************************************************************************************************************
*/
uint32_t ParaGetLimitHour(void)
{
	return CfgInfo.LimitHour;
}



/************************************************************************************************************
** �� �� �� : ParaSetActCode
** �������� : ���ü����룬��������ȷ���ɹ�д�뷵��0  ���򷵻�1
** ��    �� : buf
** ��    �� : ��
** ��    �� :	1 д�����   0 д����ȷ
*************************************************************************************************************
*/
uint8_t ParaSetActCode(uint8_t * data)
{
	uint8_t Getbuf[32],str[32], oldpass[4], newpass[4], daystr[5];
	uint8_t *pstr = NULL;
	
	memcpy(Getbuf, data, 32);	
	if(strlen((const char *)Getbuf) != 26)
		return 1;
	memset(str, 0, 32);	
	memset(daystr, '\0', sizeof(daystr));	
	Decryption(Getbuf, strlen((const char *)Getbuf), str);		//����
	pstr = str;
	memcpy(newpass, pstr, 4);
	memcpy(oldpass, pstr+4, 4);
	memcpy(daystr, pstr+8, 4);
	if(memcmp(oldpass, CfgInfo.ActPassword, 4))
		return 1;
	memcpy(CfgInfo.ActPassword, newpass, 4);// ��new  д��  old
	memcpy(CfgInfo.ActCode, data, 32); //д�뼤����
	CfgInfo.LimitHour =  atoi((const char *)daystr)* 24;	
	RunHour = 0;
	EepromWrite(0, EEPROMADDR_SAVERUNHOUR, &RunHour, MODE_8_BIT, sizeof(RunHour));
	EepromWrite(0, EEPROMADDR_SAVEINFO, &CfgInfo, MODE_8_BIT, sizeof(CFGINFO));
	return 0;
}


/************************************************************************************************************
** �� �� �� : ParaGetActCode
** �������� : ��ȡ������
** ��    �� : ��
** ��    �� : 32�ֽڼ�����
** ��    �� :	��
*************************************************************************************************************
*/
void ParaGetActCode(uint8_t *ActCode)
{
	memcpy(ActCode, CfgInfo.ActCode, 32);
}

/************************************************************************************************************
** �� �� �� : ParaGetRunHour
** �������� : ��ȡ�豸������ʹ�����������ڵ�����ʱ��  ��λ��Сʱ
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��ȷ����1 ���󷵻�0
*************************************************************************************************************
*/
uint32_t ParaGetRunHour(void)
{
	return RunHour;
}


/************************************************************************************************************
** �� �� �� : ParaSavaRunHour
** �������� : �洢����ʱ�����
** ��    �� : ����ʱ��
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ParaSavaRunHour(uint32_t  runhour)
{
	RunHour = runhour;
	EepromWrite(0, EEPROMADDR_SAVERUNHOUR, &RunHour, MODE_8_BIT, sizeof(RunHour));
}







/************************************************************************************************************
** �� �� �� : ConfigProtocolFrameCheck
** �������� : ����Э��֡��麯��
** ��    �� : buf���ݻ������׵�ַ  Channel Э��ͨ��
** ��    �� : ��
** ��    �� :	��ȷ����1 ���󷵻�0
*************************************************************************************************************
*/
uint8_t ConfigProtocolFrameCheck(uint8_t *buf, uint8_t Channel)
{
	uint8_t cs,i;
	uint8_t PileNumber[32]={0};
	if(Channel == PARACHANNEL_UART){
		
		if (buf[0] != 0x68){
			return 0;
		}
		if (buf[1] & 0x80){
			return 0;
		}
		cs = 0;
		for (i =0; i < buf[2] + 3; i++){
			cs += buf[i];
		}
		if (cs != buf[buf[2] + 3]){
			return 0;
		}
		if(buf[buf[2] + 4] != 0x16){
			return 0;
		}
		return 1;
		
	}else if(Channel == PARACHANNEL_UDP){
		if (buf[0] != 0x68){
			return 0;
		}
		memset(PileNumber, '0', 32);
		if( memcmp( &buf[1], PileNumber, 32) ){  //�ж�׮���
				ParaGetPileNo(0, PileNumber);
				if( memcmp( &buf[1], PileNumber, 32)){
					return 0;
				}
		}
		if (buf[33] != 0x68){
			return 0;
		}
		if (buf[1 + 33] & 0x80){
			return 0;
		}
		cs = 0;
		for (i =0; i < buf[2 + 33] + 3; i++){
			cs += buf[i+33];
		}
		if (cs != buf[33 + buf[2 + 33] + 3]){
			return 0;
		}
		if(buf[33 + buf[2 + 33] + 4] != 0x16){
			return 0;
		}
		return 1;
	}else{
		return 0;
	}

}

/************************************************************************************************************
** �� �� �� : DebugPlatformReadDataAnalyse
** �������� : ����ƽ̨�����ݷ���
** ��    �� : buf���ݻ������׵�ַ
** ��    �� : ��
** ��    �� :	��ȷ����1 ���󷵻�0
*************************************************************************************************************
*/
uint8_t DebugPlatformReadDataAnalyse(uint8_t *buf, uint8_t Channel)
{
	uint16_t DI;
	uint8_t ret = 0x02,databuf[257],datalen,ackbuffer[256],cs,i,*p,openhour,openmin,closehour,closemin;
	uint8_t data8;
	uint16_t data16;
	uint32_t data32;
	Rtc datetime;
	
	memset(databuf, 0, sizeof(databuf));
	datalen = 0;
	if(Channel == PARACHANNEL_UDP)
		buf +=33;
	
	DI = *(buf+3) | ( *(buf + 4) << 8);
	switch (DI){
		case 0x0001://���׮���
			databuf[0] = *(buf + 5);
			if (*(buf + 5) == 1){
				ParaGetPileNo(0, databuf + 1);
			} else if (*(buf + 5) == 2){
				ParaGetPileNo(1, databuf + 1);
			}
			datalen = 33;
			ret = 0;
			break;
			
		case 0x0002://�����汾��
			p = databuf;
			data8 = _VERSION_MAJOR;
			if (data8 >= 100){
				*p++ = (data8 / 100) | 0x30;
				data8 %= 100;
				*p++ = (data8 / 10) | 0x30;
				*p++ = (data8 % 10) | 0x30;
			} else if (data8 >= 10){
				*p++ = (data8 / 10) | 0x30;
				*p++ = (data8 % 10) | 0x30;
			} else {
				*p++ = data8 | 0x30;
			}
			*p++ = '.';
			data8 = _VERSION_MINOR;
			if (data8 >= 100){
				*p++ = (data8 / 100) | 0x30;
				data8 %= 100;
				*p++ = (data8 / 10) | 0x30;
				*p++ = (data8 % 10) | 0x30;
			} else if (data8 >= 10){
				*p++ = (data8 / 10) | 0x30;
				*p++ = (data8 % 10) | 0x30;
			} else {
				*p++ = data8 | 0x30;
			}
			*p++ = '.';
			data8 = _VERSION_PATCH;
			if (data8 >= 100){
				*p++ = (data8 / 100) | 0x30;
				data8 %= 100;
				*p++ = (data8 / 10) | 0x30;
				*p++ = (data8 % 10) | 0x30;
			} else if (data8 >= 10){
				*p++ = (data8 / 10) | 0x30;
				*p++ = (data8 % 10) | 0x30;
			} else {
				*p++ = data8 | 0x30;
			}
			*p++ = '\0';
			datalen = 16;
			ret = 0;
			break;
		
		case 0x0003://Ӳ���汾
			ParaGetHardVersion(databuf);
			datalen = 16;
			ret = 0;
			break;
		
		case 0x0004://���׮����
			databuf[0] = ParaGetChgGunNumber();
			datalen = 1;
			ret = 0;
			break;
		
		case 0x0007://���׮��ǰʱ��
			RtcRead(&datetime);
			databuf[6] = datetime.year >> 8;
			databuf[5] = datetime.year;
			databuf[4] = datetime.month;
			databuf[3] = datetime.day;
			databuf[2] = datetime.hour;
			databuf[1] = datetime.min;
			databuf[0] = datetime.sec;
			datalen = 7;
			ret = 0;
			break;
		
		case 0x0008://��������
			databuf[0] = ParaGetNetHeartTime(0);
			datalen = 1;
			ret = 0;
			break;
		
		case 0x0009://���������ʱ��
			ParaGetScreenTime(databuf + 1, databuf, &closehour, &closemin);
			datalen = 2;
			ret = 0;
			break;
		
		case 0x000A://������ر�ʱ��
			ParaGetScreenTime(&openhour, &openmin, databuf + 1, databuf);
			datalen = 2;
			ret = 0;
			break;
		
		case 0x000B://���������ַ
			if ((*(buf + 5) == 1) || (*(buf + 5) == 2)){
				databuf[0] = *(buf + 5);
				ParaGetACMeterAddr( *(buf + 5) - 1, databuf + 1);
				datalen = 7;
			}
			ret = 0;
			break;
			
		case 0x000C://ֱ�������ַ
			if ((*(buf + 5) == 1) || (*(buf + 5) == 2)){
				databuf[0] = *(buf + 5);
				ParaGetDCMeterAddr(*(buf + 5) - 1, databuf + 1);
				datalen = 7;
			}
			ret = 0;
			break;
			
		case 0x000D://�ͻ����
			ParaGetCustomerNumber(databuf);
			datalen = 4;
			ret = 0;
			break;
		
		case 0x000F://����������
			memcpy(databuf, "0000000000000001", 16);
			datalen = 16;
			ret = 0;
			break;
		
		case 0x0011://��ά��
			if ((*(buf + 5) == 1) || (*(buf + 5) == 2)){
				databuf[0] = *(buf + 5);
				ParaGetQRCode(*(buf + 5) - 1, databuf + 1);
				datalen = 129;
				ret = 0;
			}
			break;
		
		case 0x0101://����ͨ��
			databuf[0] = ParaGetUpChannel();
			datalen = 1;
			ret = 0;
			break;
		
		case 0x0102://������IP
			ParaGetServerIp(0, databuf);
			datalen = 4;
			ret = 0;
			break;
		
		case 0x0103://�������˿�
			data16 = ParaGetServerPort(0);
			memcpy(databuf, &data16, 2);
			datalen = 2;
			ret = 0;
			break;	
		
		case 0x0104://����IP
			ParaGetLocalIp(databuf);
			datalen = 4;
			ret = 0;
			break;
		
		case 0x0105://��������
			ParaGetSubMask(databuf);
			datalen = 4;
			ret = 0;
			break;
		
		case 0x0106://����
			ParaGetGateWay(databuf);
			datalen = 4;
			ret = 0;
			break;
		
		case 0x0107://������ַ
			ParaGetPhyMac(databuf);
			datalen = 6;
			ret = 0;
			break;
		
		case 0x0109://APN
			ParaGetAPN(databuf);
			datalen = 16;
			ret = 0;
			break;
		
		case 0x010A://APN User
			ParaGetAPNUser(databuf);
			datalen = 32;
			ret = 0;
			break;
		
		case 0x010B://APN Passwd
			ParaGetAPNPasswd(databuf);
			datalen = 32;
			ret = 0;
			break;
		
		case 0x0201://���ƽ�ȷ���
			for (i = 0; i < 12; i++){
				ParaGetFeilvTime(0, i, databuf + i * 8 + 1, databuf + i * 8 + 0, databuf + i * 8 + 3, databuf + i * 8 + 2);
				data32 = ParaGetFeilvMoney(0, i);
				memcpy(databuf + i * 8 + 4, &data32, 4);
			}
			datalen = 96;
			ret = 0;
			break;
			
		case 0x0012://��������
			ParaGetManufactureDate(databuf);
			datalen = 4;
			ret = 0;
			break;
		
		case 0x0013://��������
			ParaGetManufactureBase(databuf);
			datalen = 16;
			ret = 0;
			break;
		
		case 0x0014://�����
			ParaGetTestWorker(databuf);
			datalen = 8;
			ret = 0;
			break;
		
		case 0x0015://ʹ����
			ParaGetLifeTime(databuf);
			datalen = 5;
			ret = 0;
			break;
		case 0x0016://������
			ParaGetActCode(databuf);
			datalen = 32;
			ret = 0;
			break;
		case 0x00A1:	
			databuf[0] = *(buf + 5);
			if(databuf[0] > 2)
				break;
			data16 = ParaGetPwm(*(buf + 5) - 1);
			memcpy(databuf+1, &data16, 2);
			datalen = 3;
			ret = 0;
			break;
	}
	
	if (ret == 0){
		if(Channel == PARACHANNEL_UART){
			p = ackbuffer;	
			*p++ = 0x68;
			*p++ = 0x81;
			*p++ = datalen + 2;
			*p++ = *(buf + 3);
			*p++ = *(buf + 4);
			memcpy(p, databuf, datalen);
			p += datalen;
			cs = 0;
			for (i = 0; i < datalen + 5; i++){
				cs += ackbuffer[i];
			}
			*p++ = cs;
			*p++ = 0x16;
			UartWrite(DEBUGUARTNO, ackbuffer, p - ackbuffer);
		}else if(Channel == PARACHANNEL_UDP){		
			p = ackbuffer;	
			*p++ = 0x68;
			ParaGetPileNo(0, p);  //׮���
			p += 32;
			*p++ = 0x68;
			*p++ = 0x81;
			*p++ = datalen + 2;
			*p++ = *(buf + 3);
			*p++ = *(buf + 4);
			memcpy(p, databuf, datalen);
			p += datalen;
			cs = 0;
			for (i = 0; i < datalen + 5; i++){
				cs += ackbuffer[i+33];
			}
			*p++ = cs;
			*p++ = 0x16;
			UDPSend( ackbuffer, p - ackbuffer);
		}
	} else {
		if(Channel == PARACHANNEL_UART){
			p = ackbuffer;
			*p++ = 0x68;
			*p++ = 0xC1;
			*p++ = 0x01;
			*p++ = ret;
			cs = 0;
			for (i = 0; i < 4; i++){
				cs += ackbuffer[i];
			}
			*p++ = cs;
			*p++ = 0x16;
			UartWrite(DEBUGUARTNO, ackbuffer, p - ackbuffer);
		}else if(Channel == PARACHANNEL_UDP){
			p = ackbuffer;
			*p++ = 0x68;
			ParaGetPileNo(0, p);  //׮���
			p += 32;
			
			*p++ = 0x68;
			*p++ = 0xC1;
			*p++ = 0x01;
			*p++ = ret;
			cs = 0;
			for (i = 0; i < 4; i++){
				cs += ackbuffer[i+33];
			}
			*p++ = cs;
			*p++ = 0x16;
			UDPSend( ackbuffer, p - ackbuffer);
		}
	}
	
	return 1;
}

/************************************************************************************************************
** �� �� �� : DebugPlatformWriteDataAnalyse
** �������� : ����ƽ̨д���ݷ���
** ��    �� : buf���ݻ������׵�ַ
** ��    �� : ��
** ��    �� :	��ȷ����1 ���󷵻�0
*************************************************************************************************************
*/
uint8_t DebugPlatformWriteDataAnalyse(uint8_t *buf, uint8_t Channel)
{
	uint16_t DI;
	uint8_t ret = 0x02,databuf[128],ackbuffer[256],cs,i,*p,openhour,openmin,closehour,closemin;
	uint32_t data32;
	uint8_t LocaServerIp[4]={0};
	Rtc datetime;
	uint8_t PileNo[32];
	ParaGetPileNo(0, PileNo);
	memset(databuf, 0, sizeof(databuf));
	if(Channel == PARACHANNEL_UDP)
		buf +=33;
	
	DI = *(buf + 3) | (*(buf + 4) << 8);
	switch (DI){
		case 0x0001://���׮���
			if (buf[5] == 1){
				ParaSetPileNo(0, buf + 5 + 1);
				ParaSetPileNo(1, buf + 5 + 1);
				ret = 0;
			} else if (buf[5] == 2){
				ParaSetPileNo(1, buf + 5 + 1);
				ret = 0;
			}
			break;
			
		case 0x0003://Ӳ���汾
			ParaSetHardVersion(buf + 5);
			ret = 0;
			break;
		
		case 0x0004://���׮����
			if ((*(buf +5) == 1) || ( *(buf +5) == 2)){
				ParaSetChgGunNumber(*(buf +5));
				ret = 0;
			}
			break;
		
		case 0x0007://���׮��ǰʱ��
			datetime.year = *(buf + 10) | (*(buf + 11) << 8);
			datetime.month = *(buf + 9);
			datetime.day = *(buf + 8);
			datetime.hour = *(buf + 7);
			datetime.min = *(buf + 6);
			datetime.sec = *(buf + 5);
			RtcWrite(&datetime);
			ret = 0;
			break;
		
		case 0x0008://��������
			ParaSetNetHeartTime(0, *(buf + 5));
			ParaSetNetHeartTime(1, *(buf + 5));
			ret = 0;
			break;
		
		case 0x0009://���������ʱ��
			ParaGetScreenTime(&openhour, &openmin, &closehour, &closemin);
			ParaSetScreenTime(*(buf + 6), *(buf + 5), closehour, closemin);
			ret = 0;
			break;
		
		case 0x000A://������ر�ʱ��
			ParaGetScreenTime(&openhour, &openmin, &closehour, &closemin);
			ParaSetScreenTime(openhour, openmin, *(buf + 6), *(buf + 5));
			ret = 0;
			break;
		
		case 0x000B://���������ַ
			if ((*(buf + 5) == 1) || (*(buf + 5) == 2)){
				ParaSetACMeterAddr(*(buf + 5) - 1, buf + 5 + 1);
				ret = 0;
			}
			break;
			
		case 0x000C://ֱ�������ַ
			if ((*(buf + 5) == 1) || (*(buf + 5) == 2)){
				ParaSetDCMeterAddr(*(buf + 5) - 1, buf + 5 + 1);
				ret = 0;
			}
			break;
			
		case 0x000D://�ͻ����
			ParaSetCustomerNumber(buf + 5);
			ret = 0;
			break;
			
		case 0x000E://�ն˸�λ
			if ((*(buf + 5) == 0) || (*(buf + 5) == 1)){
				//��������λ
				DeleteAllRecord();
			}
			ret = 0;
			break;
		
		case 0x0010://ע����
			ret = 0;
			break;
		
		case 0x0011://��ά��
			if ((*(buf + 5) == 1) || (*(buf + 5) == 2)){
				ParaSetQRCode(*(buf + 5) - 1, buf + 5 + 1);
				ret = 0;
			}
			break;
		
		case 0x0101://����ͨ��
			ParaSetUpChannel(*(buf + 5));
			ret = 0;
			break;
		
		case 0x0102://������IP
			ParaGetServerIp(0, LocaServerIp);
			if(memcmp(LocaServerIp, buf + 5, 4 ) != 0){
				ParaSetServerIp(0, buf + 5);
				ParaSetServerIp(1, buf + 5);
				ParaSave();
				DeleteAllRecord();
			}
			ret = 0;
			break;
		
		case 0x0103://�������˿�
			ParaSetServerPort(0, *(buf + 5) | (*(buf + 6)  << 8));
			ParaSetServerPort(1, *(buf + 5) | (*(buf + 6) << 8));
			ret = 0;
			break;
		
		case 0x0104://����IP
			ParaSetLocalIp(buf + 5);
			ret = 0;
			break;
		
		case 0x0105://��������
			ParaSetSubMask(buf + 5);
			ret = 0;
			break;
		
		case 0x0106://����
			ParaSetGateWay(buf + 5);
			ret = 0;
			break;
		
		case 0x0107://������ַ
			ParaSetPhyMac(buf + 5);
			ret = 0;
			break;
		
		case 0x0109://APN
			ParaSetAPN(buf + 5);
			ret = 0;
			break;
		
		case 0x010A://APN User
			ParaSetAPNUser(buf + 5);
			ret = 0;
			break;
		
		case 0x010B://APN Passwd
			ParaSetAPNPasswd(buf + 5);
			ret = 0;
			break;
		
		case 0x0201://���ƽ�ȷ���
			for (i = 0; i < 12; i++){
				ParaSetFeilvTime(0, i,  *(buf + 5 + i * 8 + 1), *(buf + 5 + i * 8 + 0), *(buf + 5 + i * 8 + 3), *(buf + 5 + i * 8 + 2));
				memcpy(&data32, buf + 5 + i * 8 + 4, 4);
				ParaSetFeilvMoney(0, i, data32);
				ParaSetFeilvTime(1, i,  *(buf + 5 + i * 8 + 1), *(buf + 5 + i * 8 + 0), *(buf + 5 + i * 8 + 3), *(buf + 5 + i * 8 + 3));
				ParaSetFeilvMoney(1, i, data32);
			}
			ret = 0;
			break;
			
		case 0x0012://��������
			ParaSetManufactureDate(buf + 5);
			ret = 0;
			break;
		
		case 0x0013://��������
			ParaSetManufactureBase(buf + 5);
			ret = 0;
			break;
		
		case 0x0014://�����
			ParaSetTestWorker(buf + 5);
			ret = 0;
			break;
		
		case 0x0015://ʹ����
			ParaSetLifeTime(buf + 5);
			ret = 0;
			break;
		case 0x0016://������
			ret =ParaSetActCode(buf + 5);
			break;
		case 0x00A1://ռ�ձ�
			if(buf[5] > 2)
				break;
			if(ParaSetPwm(*(buf + 5) - 1 , *(buf + 6) | (*(buf + 7)  << 8))){
				ret = 0;
				break;
			}else{
				ret = 1;
				break;
			}

			
	}
	
	if (ret == 0){

		if(Channel == PARACHANNEL_UART){
			p = ackbuffer;
			*p++ = 0x68;
			*p++ = 0x91;
			*p++ = 0x00;
			*p++ = 0xF9;
			*p++ = 0x16;
			UartWrite(DEBUGUARTNO, ackbuffer, p - ackbuffer);
		}else if(Channel == PARACHANNEL_UDP){
			p = ackbuffer;
			*p++ = 0x68;
			memcpy(p, PileNo, 32);//׮���
			p += 32;
			*p++ = 0x68;
			*p++ = 0x91;
			*p++ = 0x00;
			*p++ = 0xF9;
			*p++ = 0x16;
			UDPSend( ackbuffer, p - ackbuffer);
		}
	} else {

		if(Channel == PARACHANNEL_UART){
			p = ackbuffer;
			*p++ = 0x68;
			*p++ = 0xD1;
			*p++ = 0x01;
			*p++ = ret;
			cs = 0;
			for (i = 0; i < 4; i++){
			cs += ackbuffer[i];
			}
			*p++ = cs;
			*p++ = 0x16;
			UartWrite(DEBUGUARTNO, ackbuffer, p - ackbuffer);
		}else if(Channel == PARACHANNEL_UDP){
			p = ackbuffer;
			*p++ = 0x68;
			memcpy(p, PileNo, 32);//׮���
			p += 32;
			*p++ = 0x68;
			*p++ = 0xD1;
			*p++ = 0x01;
			*p++ = ret;
			cs = 0;
			for (i = 0; i < 4; i++){
			cs += ackbuffer[i+33];
			}
			*p++ = cs;
			*p++ = 0x16;

			UDPSend( ackbuffer, p - ackbuffer);
		}
	}
	
	if (DI == 0x000E){ //�ն˸�λ
		OSTimeDlyHMSM (0, 0, 1, 0);
		NVIC_SystemReset();
	}
	
	return 1;
}




/****************************************************************************\
 Function:  AnalyseMsgFromDebugPlatform
 Descript:  �����ӵ���ƽ̨���յ�������
 Input:	    ��
 Output:    ��
 Return:    ��
 *****************************************************************************/
void AnalyseMsgFromDebugPlatform(void)
{
	static uint8_t buffer[512];
	static uint16_t len;
	uint16_t ret,i;
	
	ret = UartRead(DEBUGUARTNO, buffer + len, sizeof(buffer) - len);
	len += ret;
	if (len >= sizeof(buffer)){
		memset(buffer, 0, sizeof(buffer));
		len = 0;
	}
	if (len){
		for (i = 0; i < len; i++){
			if (ConfigProtocolFrameCheck(buffer + i, PARACHANNEL_UART) == 1){
				if ((buffer[i + 1 ] & 0x1f) ==0x01){
					DebugPlatformReadDataAnalyse(&buffer[i], PARACHANNEL_UART);
				} else if ((buffer[i + 1 ] & 0x1f) ==0x11){
					DebugPlatformWriteDataAnalyse(&buffer[i], PARACHANNEL_UART);
				}
				memset(buffer, 0, sizeof(buffer));
				len = 0;
				break;
			}
		}
	}
}
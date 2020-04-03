#ifndef _PARA_H
#define _PARA_H

//EEPROM中存储数据分布
//64个字节为一页，共64个页
#define EEPROMADDR_PARAINFO 0 //参数 第1页-第16页

#define EEPROMADDR_SAVEINFO 16 // 断电需要存储的数据 第16页
#define EEPROMADDR_SAVERUNHOUR 17// 断电需要存储的数据 第17页

//上行通道
#define CHANNEL_ETH		0X01	//以太网通道
#define CHANNEL_GPRS	0X02	//GPRS通道


#define PARACHANNEL_UART  0X01  // 串口配置通道
#define PARACHANNEL_UDP   0X02  // UDP网络配置通道

//参数
typedef struct{
	uint32_t version; //版本号 0x5A5A5A01
	
	uint8_t  ChgGunNumber; // 充枪数量 1或2
	
	uint8_t  UpChannel; // 上行通道
	uint8_t LocalIp[4]; //本机IP
	uint8_t SubMask[4]; //子网掩码
	uint8_t GateWay[4]; //网关
	uint8_t PhyMac[6]; //物理地址
	
	uint8_t HardVersion[16]; //硬件版本号 ASCII
	
	uint8_t CustomerNumber[6]; //客户编号
	
	uint8_t APN[16]; //APN
	uint8_t APNUser[32]; //APN用户名
	uint8_t APNPasswd[32]; //APN密码
	
	uint8_t ScreenOpenHour; //广告屏开启时间 小时
	uint8_t ScreenOpenMin; //广告屏开启时间 分钟
	uint8_t ScreenStopHour; //广告屏关闭时间 小时
	uint8_t ScreenStopMin; //广告屏关闭时间 分钟
	
	struct{
		uint8_t PileNo[32]; //充电桩编号 ASCII
		uint8_t ACMeterAddr[6]; //交流电表地址 MeterAddr[0]是A0
		uint8_t DCMeterAddr[6]; //直流电表地址 MeterAddr[0]是A0
		uint16_t LoginTimeInterval; //签到间隔时间 分钟
		uint16_t NetHeartTime; //网络心跳周期 单位S
		uint8_t NetCommOverCnt; //网络通讯超时重试次数
		uint8_t  ServerIp[4]; //服务器IP
		uint16_t ServerPort; //服务器端口
		uint8_t StartHour[12]; //计费策略 开始小时
		uint8_t StartMin[12]; //计费策略 开始分钟
		uint8_t StopHour[12]; //计费策略 结束小时
		uint8_t StopMin[12]; //计费策略 结束分钟
		uint32_t Money[12]; //计费策略 费率 每度电的电费 保留5位小数
		uint32_t StateInfoReportPeriod; //充电桩状态信息上报周期 单位秒
		uint8_t QrData[256]; //二维码内容
	}Port[2];
	
	uint8_t ManufactureDate[4]; //生产日期
	uint8_t ManufactureBase[16]; //生产基地
	uint8_t TestWorker[8]; //检测人
	uint8_t LifeTime[5]; //使用期
}PARAINFO;


typedef struct{
	uint32_t Updata;  //升级成功标志
	uint16_t Pwm;
	uint8_t ActPassword[4];
	uint32_t LimitHour;
	uint8_t ActCode[32];

}CFGINFO;

void ParaInit(void);
void 	ParaSetChgGunNumber(uint8_t number);
uint8_t ParaGetChgGunNumber(void);
void ParaSetPileNo(uint8_t port, uint8_t *data);
void ParaGetPileNo(uint8_t port, uint8_t *data);
void ParaSetLoginTimeInterval(uint8_t port, uint16_t TimeInterval);
uint16_t ParaGetLoginTimeInterval(uint8_t port);
void ParaSetACMeterAddr(uint8_t port, uint8_t *data);
void ParaGetACMeterAddr(uint8_t port, uint8_t *data);
void ParaSetDCMeterAddr(uint8_t port, uint8_t *data);
void ParaGetDCMeterAddr(uint8_t port, uint8_t *data);
void ParaSetNetHeartTime(uint8_t port, uint16_t time);
uint16_t ParaGetNetHeartTime(uint8_t port);
void ParaSetNetCommOverCnt(uint8_t port, uint8_t cnt);
uint8_t ParaGetNetCommOverCnt(uint8_t port);
void ParaSetUpChannel(uint8_t channel);
uint8_t ParaGetUpChannel(void);
void ParaSetLocalIp(uint8_t *data);
void ParaGetLocalIp(uint8_t *data);
void ParaSetSubMask(uint8_t *data);
void ParaGetSubMask(uint8_t *data);
void ParaSetGateWay(uint8_t *data);
void ParaGetGateWay(uint8_t *data);
void ParaSetPhyMac(uint8_t *data);
void ParaGetPhyMac(uint8_t *data);
void ParaSetServerIp(uint8_t port, uint8_t *data);
void ParaGetServerIp(uint8_t port, uint8_t *data);
void ParaSetServerPort(uint8_t port, uint16_t serverport);
uint16_t ParaGetServerPort(uint8_t port);
void ParaSetFeilvTime(uint8_t port, uint8_t period, uint8_t starthour, uint8_t startmin, uint8_t stophour, uint8_t stopmin);
void ParaGetFeilvTime(uint8_t port, uint8_t period, uint8_t *starthour, uint8_t *startmin, uint8_t *stophour, uint8_t *stopmin);
void ParaSetFeilvMoney(uint8_t port, uint8_t period, uint32_t money);
uint32_t ParaGetFeilvMoney(uint8_t port, uint8_t period);
void ParaSetStateInfoReportPeriod(uint8_t port, uint16_t time);
uint16_t ParaGetStateInfoReportPeriod(uint8_t port);
void ParaSetQRCode(uint8_t port, uint8_t *qrcode);
void ParaGetQRCode(uint8_t port, uint8_t *qrcode);
void ParaSetCustomerNumber(uint8_t *data);
void ParaGetCustomerNumber(uint8_t *data);
void ParaSetAPN(uint8_t *data);
void ParaGetAPN(uint8_t *data);
void ParaSetAPNUser(uint8_t *data);
void ParaGetAPNUser(uint8_t *data);
void ParaSetAPNPasswd(uint8_t *data);
void ParaGetAPNPasswd(uint8_t *data);
void ParaSetScreenTime(uint8_t openhour, uint8_t openmin, uint8_t closehour, uint8_t closemin);
void ParaGetScreenTime(uint8_t *openhour, uint8_t *openmin, uint8_t *closehour, uint8_t *closemin);
void ParaSetManufactureDate(uint8_t *Date);
void ParaGetManufactureDate(uint8_t *Date);
void ParaSetManufactureBase(uint8_t *Base);
void ParaGetManufactureBase(uint8_t *Base);
void ParaSetTestWorker(uint8_t *TestWorker);
void ParaGetTestWorker(uint8_t *TestWorker);
void ParaSetLifeTime(uint8_t *LifeTime);
void ParaGetLifeTime(uint8_t *LifeTime);
void AnalyseMsgFromDebugPlatform(void);

void SetUpdataFlag(uint32_t flag);
uint32_t GetUpdata(void);

uint8_t GetParaCmd(uint8_t port);
void SetParaResult(uint8_t port, uint8_t Result);
uint16_t ParaGetPwm(uint8_t port);
uint16_t GetCfgPwm(uint8_t port);

uint32_t ParaGetLimitHour(void);
uint8_t ParaSetActCode(uint8_t * data);
uint32_t ParaGetRunHour(void);
void ParaSavaRunHour(uint32_t  runhour);

uint8_t ConfigProtocolFrameCheck(uint8_t *buf, uint8_t Channel);
uint8_t DebugPlatformReadDataAnalyse(uint8_t *buf, uint8_t Channel);
uint8_t DebugPlatformWriteDataAnalyse(uint8_t *buf, uint8_t Channel);

#endif

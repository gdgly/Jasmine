#ifndef _MAINTASK_H
#define _MAINTASK_H

//停止充电原因
static const char *stopcausetab[] = {
	"正常停止",//0x00
	"等待插枪超时",//0x01
	"锁枪失败",//0x02
	"继电器粘连故障",//0x03
	"继电器拒动故障",//0x04
	"外侧电压异常",//0x05
	"充电模块输出故障",//0x06
	"绝缘检测异常",//0x07
	"泄放电路异常",//0x08
	"车辆参数不适合",//0x09
	"启动时电压异常",//0x0A
	"电池准备就绪变为未就绪",//0x0B
	"充电桩充电中暂停超时",//0x0C
	"充电枪断开连接",//0x0D
	"BMS通信超时",//0x0E
	"急停动作故障",//0x0F
	"门禁故障",//0x10
	"充电接口过温",//0x11
	"直流输出电压过压",//0x12
	"需求电压异常",//0x13
	"单体动力蓄电池电压过高",//0x14
	"单体动力蓄电池电压过低",//0x15
	"电池电量过高",//0x16
	"电池电量过低",//0x17
	"单体动力蓄电池电流过流",//0x18
	"动力蓄电池温度过高",//0x19
	"动力蓄电池绝缘状态异常",//0x1A
	"动力蓄电池组输出连接器连接状态异常",//0x1B
	"电池充满",//0x1C
	"电池充满",//0x1D
	"电池充满",//0x1E
	"绝缘故障",//0x1F
	"输出连接器过温故障",//0x20
	"元件或输出连接器过温",//0x21
	"充电连接器故障",//0x22
	"电池组温度过高",//0x23
	"高压继电器故障",//0x24
	"车辆接口未完全连接",//0x25
	"其他故障",//0x26
	"充电电流过大",//0x27
	"充电电压过大",//0x28
	"控制导引异常",//0x29
	"充电枪连接异常",//0x2A
	"充电资源申请失败",//0x2B
	"直流输出过流",//0x2C
	"输入电压过压",//0x2D
	"输入电压欠压",//0x2E
	"充电电流过流",//0x2F
	"充电电流持续过小，自动停止充电",//0x30
	"余额不足",//0x31
	"充电模块输入过压",//0x32
	"充电模块输入欠压",//0x33
	"电表故障",//0x34
	"充电中充电暂停次数过多",//0x35
	"充电电流持续过小，自动停止充电"// 0x36
};
#define PORTA     0
#define PORTB     1
#define LINENUM     5       //显示充电记录的行数
//单枪
#define A_IDLE		0x00 //A枪空闲
#define A_CHARGE	0x01 //A枪充电
#define A_STOP		0x02 //A枪停止

//双枪
#define A_IDLE_B_IDLE			0x00 //A枪空闲 B枪空闲
#define A_IDLE_B_CHARGE		0x01 //A枪空闲 B枪充电
#define A_IDLE_B_STOP			0x02 //A枪空闲 B枪停止
#define A_CHARGE_B_IDLE		0x10 //A枪充电 B枪空闲
#define A_CHARGE_B_CHARGE	0x11 //A枪充电 B枪充电
#define A_CHARGE_B_STOP		0x12 //A枪充电 B枪停止
#define A_STOP_B_IDLE			0x20 //A枪停止 B枪空闲
#define A_STOP_B_CHARGE		0x21 //A枪停止 B枪充电
#define A_STOP_B_STOP			0x22 //A枪停止 B枪停止

#define FAULTCODE_CHARGEBOARD_COMM	1 //与充电控制板通信故障
#define FAULTCODE_SCRAM							2 //急停故障
#define FAULTCODE_DOOR							3 //门禁故障
#define FAULTCODE_METER_COMM				4 //与电表通信故障
#define FAULTCODE_EXPIRE_COMM				5 //使用期限到期故障
#define FAULTCODE_CCBADDRCONFLICT		6 //充电控制板地址冲突故障   
#define FAULTCODE_TEMP_ERR          7 //充电枪座过温
#define FAULTCODE_HIGHVOL           8 //交流输入过压
#define FAULTCODE_LOWVOL            9 //交流输入欠压

#define UNLOCKCARDFAILCODE_WRITECARD	1 //写卡失败
#define UNLOCKCARDFAILCODE_NORECORD		2 //本桩没该锁卡记录

#define STARTCHARGETYPE_OFFLINECARD		0 //离线卡
#define STARTCHARGETYPE_ONLINECARD		3 //在线卡
#define STARTCHARGETYPE_PLATFORM			1 //平台

typedef struct{
	uint32_t StartTimer;//开始时定时器值
	uint32_t StopTimer;//结束时定时器值	
	uint32_t VoltageA;//A相电压值 精度0.1V
	uint32_t VoltageB;//B相电压值 精度0.1V
	uint32_t VoltageC;//C相电压值 精度0.1V
	uint32_t CurrentA;//A相电流值 精度0.01A
	uint32_t CurrentB;//B相电流值 精度0.01A
	uint32_t CurrentC;//C相电流值 精度0.01A
}GUNINFO;

typedef struct{
	uint8_t ChgPort; //充电接口标识 0代表A枪 1代表B枪
	
	uint16_t SerialNum;//序列号（用于锁卡程序判断）
	
	uint8_t StartType;//启动方式 1离线卡 2在线卡 3后台启动
	uint32_t StartCardID;//刷卡启动时的启动卡号
	uint8_t Mode;//模式 0充满 1按电量 2按时间 3按金额
	uint32_t Para;//参数 按充满时精度为0.01元 按电量时精度为0.01kWh 按时间时精度为秒 按金额时精度为0.01元
	
	uint32_t StartCardMoney;//充电前卡余额 精度0.01元
	
	uint32_t StartMeterEnergy;//开始电表电量示值 精度0.01度
	uint32_t StopMeterEnergy;//结束电表电量示值 精度0.01度
	uint32_t SumEnergy;//累计充电电量 精度0.01度
	
	uint32_t SumMoney;//累计充电金额 精度0.01元
	
	uint16_t PeriodEnergy[48]; //时段电量 精度0.01度
	
	uint32_t SumTime;//累计充电时长 精度1秒钟
	
	uint8_t StartDateTime[6];//启动日期 hex [0]year [1]month [2]day [3]hour [4]min [5]sec
	uint8_t StopDateTime[6];//停止日期 hex [0]year [1]month [2]day [3]hour [4]min [5]sec
	
	uint8_t StartSOC;//开始SOC 单位1%
	uint8_t StopSOC;//结束SOC 单位1%
	
	uint8_t VIN[17];//车辆VIN码
	
	uint8_t TradeSerialNumber[32]; //交易流水号
	
	uint8_t CardOrDiscernNumber[32]; //充电卡号/用户识别号
	
	uint8_t StopCause;//停止原因
	
	uint8_t UserId[2];//用户ID
}CHARGINGDATA;

typedef struct 
{
	GUNINFO GunInfo[2];
	
	CHARGINGDATA ChgDat[2]; //需考虑断电时也保存好
	
}MAININFO;

void  	MainTask(void *pdata);

#define MAIN_IDEL			0 //空闲
#define MAIN_CHARGE		1 //充电
#define MAIN_COMPLETE	2 //完成

#define SET_START_CHARGE	1
#define SET_STOP_CHARGE 	2

#define STATE_IDLE			0
#define STATE_CHARGE		1
#define STATE_COMPLETE	2

void SetUpdataSucFlag(void);
uint8_t GetLifeTime(void);
uint8_t GetStartingChg(uint8_t port);
uint8_t GetStartType(uint8_t port);
uint32_t GetStartCardID(uint8_t port);
uint8_t GetStartMode(uint8_t port);
uint32_t GetStartPara(uint8_t port);
void GetVoltage(uint8_t port, uint32_t *VolA, uint32_t *VolB, uint32_t *VolC);
void GetCurrent(uint8_t port, uint32_t *CurA, uint32_t *CurB, uint32_t *CurC);
uint8_t GetStopCause(uint8_t port);
uint32_t GetStartMeterEnergy(uint8_t port);
uint32_t GetStopMeterEnergy(uint8_t port);
uint32_t GetSumEnergy(uint8_t port);
uint32_t GetSumMoney(uint8_t port);
uint16_t GetPeriodEnergy(uint8_t port, uint8_t Period);
uint32_t GetSumTime(uint8_t port);
void GetStartDateTime(uint8_t port, uint8_t *datetime);
void GetStopDateTime(uint8_t port, uint8_t *datetime);
uint32_t GetStartCardMoney(uint8_t port);
void GetCardOrDiscernNumber(uint8_t port, uint8_t *CardOrDiscernNumber);
void SetMainCharge(uint8_t port, uint8_t setcmd, uint8_t mode, uint32_t para, uint8_t *CardOrDiscernNumber, uint8_t *userid);
uint8_t GetMainCharge(uint8_t port);
uint8_t GetChargeRecord(uint8_t port, CHARGINGDATA* ChgData);

#endif

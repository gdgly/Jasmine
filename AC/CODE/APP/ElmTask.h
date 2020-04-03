#ifndef _ELMTASK_H
#define _ELMTASK_H

// 状态
#define ELM_ST_INIT       	0     	// 初始化
#define ELM_ST_TEST       	1		// 检测
#define ELM_ST_READTIME   	2    	// 读时间
#define ELM_ST_READDATA     3	   	// 读数据	


// 类型
#define TYPE_ELECY		0X01 // 电量
#define TYPE_VOL			0X02 // 电压
#define TYPE_CURRENT	0X03 // 电流	
#define TYPE_DATE			0X04 // 日期
#define TYPE_TIME			0X05 // 时间



#define ELMBAUD         		2400   		// 电表波特率 

#define ELMMAXCNT     			2   		// 最大电表个数



#define ELM_CMD_READDATA   		0X11  		// 读数据
#define ELM_ACK_READDATA   		0X91	 	// 读数据应答
#define ELM_ERR_READDATA   		0XD1  		// 读数据错误


#define ELM_DI_READTOTALELC  	0X00010000  // 读总电量
#define ELM_DI_READVOL	 		0x0201ff00  // 读电压
#define ELM_DI_READCURRENT		0x0202ff00	// 读电流
#define ELM_DI_READDAY			0x04000101  // 读日期
#define ELM_DI_READTIME			0x04000102  // 读时间




#define ELMHEAD  0x68    // 帧头
#define ELMTAIL  0x16	 // 帧尾	


typedef struct 
{
  uint8_t  addr[6];		// 地址
	uint8_t  ctlcode; 	// 控制码
	uint8_t  buflen; 		// 数据长度
	uint8_t  buf[128];	// 数据
}ElmRcvMsg;


typedef struct
{
	uint8_t state;				// 状态

	uint32_t totalelc;   			// 当前总电量  1/100KWH

	uint32_t vol[3];			  		// 电压	1/10 v

	uint32_t current[3];		  		// 电流   1/1000 A

	uint32_t timeoutcnt;	  		// 超时次数
	
	uint8_t commstate; //通信状态 0正常 1通信异常
}ELMINFO;









void 	ElmTask(void *pdata);
uint8_t 	ElmIsOK(uint8_t id);
uint32_t 	ElmGetTotalElc(uint8_t id);
uint32_t 	ElmGetVolA(uint8_t id);
uint32_t 	ElmGetVolB(uint8_t id);
uint32_t 	ElmGetVolC(uint8_t id);
uint32_t 	ElmGetCurA(uint8_t id);
uint32_t 	ElmGetCurB(uint8_t id);
uint32_t 	ElmGetCurC(uint8_t id);
uint32_t 	ElmGetMaxVol(uint8_t id);
uint32_t 	ElmGetMaxCur(uint8_t id);

uint8_t ElmGetCommState(uint8_t id);


static void 	ElmProc(void);
void 	ElmInit(uint8_t id);
static uint8_t  	ElmTest(uint8_t id);
static uint8_t 	ElmCheckTime(uint8_t id);
static void 	ElmErrCheck(void);




#endif





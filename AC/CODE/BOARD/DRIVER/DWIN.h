#ifndef _DWIN_H
#define _DWIN_H

// 命令
#define CMD_WRREGIT  		0x80    //写寄存器
#define CMD_RDREGIT  		0x81	//读寄存器	
#define CMD_WRVAR    		0x82	//写变量	
#define CMD_RDVAR    		0x83	//读变量	

// 寄存器地址
#define REG_VER    			0X00   // 软件版本号  可读  1 BYTE
#define REG_BKLED  			0X01   // 背光亮度    可写  1 BYTE
#define REG_PAGE   			0X03   // 页面        可读可写	2 BYTE
#define REG_RTC    			0X20   // RTC时间     可读可写  16 BYTE


#define DWINHEAD1          	0X5A    // 帧头1
#define DWINHEAD2          	0XA5    // 帧头2

typedef struct{
	uint8_t  cmd;
	uint16_t addr;
	uint16_t buflen;
	uint8_t  buf[256];
}DwinRcvMsg;

void DwinInit(void);
void DwinShowPage(uint16_t no);
void DwinWriteData(uint8_t cmd,uint16_t addr,uint16_t datalen,uint8_t* data);
uint8_t DwinReadData(DwinRcvMsg *msg);

#endif

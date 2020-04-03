#ifndef _MODEM_H
#define _MODEM_H


#define GPRSMAXSOCKET       3       //最大SOCKET个数

#define MODEMBUFLEN 		1024    // 缓冲


typedef  struct
{
	uint16_t in;
	uint16_t out;
	uint8_t  buf[MODEMBUFLEN];
}MODEMRCVMSG;

// 发送AT命令的返回
#define ATOK    		1   //AT OK
#define ATERR   		2   //AT fail
#define ATTIMEOUT       3	//AT 超时


// 协议模式
#define MODEM_TCP      	0  
#define MODEM_UDP      	1




void 	ModemInit(void);
uint8_t 	ModemConfig(void );
uint8_t   ModemConnect(uint8_t sockid,uint8_t type,uint8_t *apn,uint8_t *dip,uint16_t dport,uint16_t port);
uint8_t 	ModemDisCon(uint8_t sockid);
uint8_t 	ModemGetConSt(uint8_t sockid);
uint8_t 	ModemSendData(uint8_t sockid,uint8_t *data,uint16_t datalen);
uint16_t  ModemRcvData(uint8_t sockid,uint8_t *outbuf);
static uint8_t ModemWrWaitRd(uint8_t *data,uint8_t* ack,uint16_t time,uint8_t *outbuf);
static void  ModemBufWrByte(uint8_t data);
static uint8_t ModemBufRdByte(uint8_t *data);







#endif


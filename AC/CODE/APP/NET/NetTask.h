#ifndef _NETTASK_H
#define _NETTASK_H

#define NETDATAMAXLEN     1024   // 网络数据最大长度
#define SNNUM             2

#define GPRS             2
#define ETH              1
typedef struct{
	uint8_t state;

	uint8_t step;			// 步骤
	uint8_t loginmark;   // 登录标志 1 登录成功  0 未登录
	uint8_t heartmark;	// 心跳标志 1 有心跳应答  0 无心跳应答
	
	uint8_t socketportusedflag[SNNUM];
	uint8_t serverip[SNNUM][4];
	uint16_t serverport[SNNUM];
	
	uint8_t LinkFlag; //以太网link状态 1代表link up
	
	uint8_t ConnCmd[SNNUM]; //连接服务器命令 0待命 1重连
	uint8_t ConnState[SNNUM]; //连接服务器状态 0未连接 1已连接
	
	uint8_t SendCmd[SNNUM]; //发送数据命令 0待命 1发送
	uint8_t SendState[SNNUM]; //发送数据状态 0未成功 1成功
	
	uint8_t SendBuf[SNNUM][NETDATAMAXLEN];
	uint16_t SendLen[SNNUM];
	
	uint8_t RecvBuf[SNNUM][NETDATAMAXLEN];
	uint16_t RecvWr[SNNUM];
	uint16_t RecvRr[SNNUM];
}NETINFO;


uint8_t NetSend(uint8_t sn, uint8_t *buffer, uint16_t len);
uint16_t NetRecv(uint8_t sn, uint8_t *buffer, uint16_t len);
uint8_t NetConn(uint8_t sn, uint8_t serverip[4], uint16_t serverport);
void NetTask(void *pdata);


#endif





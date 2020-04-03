/****************************************************************************\
 Module:           GPRS通信
 instructions:     增加获取信号强度函数，修改代码规范
 Version:          1.0.0
 Author:           YuL
 Datetime:         18-09-29
 *****************************************************************************/



#include <LPC177x_8x.H>
#include "UART.h"
#include "timer.h"
#include "includes.h"
#include "NetTask.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gprs.h"
#include "Para.h"
#include "Cfg.h"
#include "Board.h"
static NETINFO NetInfo4G = {0};
static GPRS4GINFO  GprsInfo = {0};
static uint8_t readbuf[1024] = {0};

static uint32_t TimerCALL;
static uint32_t TimerCSQ;
static uint32_t TimerCPIN;
static uint32_t TimerSTART;
/************************************************************************************************************
** 函 数 名 : mystrstr
** 功能描述 : 查找相同内存函数，在长度为 leng 的 str 内存中查找长度为 len 的 msg 内存
** 输    入 : str需要查找的内存指针  msg与之比较的字符指针  leng str长度   len msg内存长度
** 输    出 : 无
** 返    回 : 0 未查找到 否则返回需要偏移的字节数-1
*************************************************************************************************************
*/
static uint32_t mystrstr(uint8_t *str, char *msg, uint32_t leng, uint32_t len)
{
	char *p = msg;
	uint32_t len1 = len;
	uint32_t rlen = 0;
	uint32_t rlen1 = 1;
	
	while (leng)
	{
		if(*str == *msg)
		{
			rlen = rlen1;
			while (len)
			{
				if(leng == 0)
					return 0;
				if (*str == *p){
					rlen1++;
					str++;
					p++;
					leng--;
					len--;
					if(len == 0){
						return rlen;
					}
				}else{
					str++;
					rlen1++;
					leng--;
					len = len1;
					p = msg;
					break;
				}
			}	
		}else{
			str++;
			leng--;
			rlen1++;
			if (len == 0){
				return 0;
			}
		}
	}
	return 0;
}


/************************************************************************************************************
** 函 数 名 : GprsReadbuf
** 功能描述 : 读取GPRS socket接收到的数据
** 输    入 : data:存放接收到数据的指针， 需要读取的数据长度
** 输    出 : 无
** 返    回 : 0 未读到数据 否则返回读到的数据长度
*************************************************************************************************************
*/
static uint16_t GprsReadbuf(uint8_t *data, uint16_t datalen)
{
	uint16_t r_len = 0;
	
	if( (data == NULL) || (datalen == 0)){
		return 0;
	}
	while(GprsInfo.RecvRd != GprsInfo.RecvWr){
		*data = GprsInfo.ReBuf[GprsInfo.RecvRd];
		GprsInfo.RecvRd = (GprsInfo.RecvRd + 1) % ME4GDATAMAXLEN;
		data++;
		if(++r_len >= datalen){
			return r_len;
		}
	}
	return r_len;
}


/************************************************************************************************************
** 函 数 名 : Recoverbuf
** 功能描述 : 恢复GPRS串口传来的数据
** 输    入 : 需要恢复的数据长度
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
static void Recoverbuf(uint16_t len)
{
	if( GprsInfo.RecvRd >= len) {
		GprsInfo.RecvRd = GprsInfo.RecvRd - len;   
	}else{
		GprsInfo.RecvRd = ME4GDATAMAXLEN - (len - GprsInfo.RecvRd );
	}
}


/************************************************************************************************************
** 函 数 名 : GprsReadSocket
** 功能描述 : 从GPRS串口接收的数据中解析服务器发送的数据并存储
** 输    入 : 无
** 输    出 : 无
** 返    回 : 0 没有解析的服务器传来的数据， 1 服务器有数据发送
*************************************************************************************************************
*/
static uint8_t GprsReadSocket(void)   
{

	uint16_t len = 0;
	char str[30] = {0};
	uint8_t serverip[4];
	uint16_t sport,i;
	uint16_t datalen = 0;
	uint8_t* pdata = NULL;
	uint8_t* pdata1 = NULL;
	uint8_t* pdata2 = NULL;
	uint16_t offset = 0;
	char sn = '1';
	
	memset(readbuf,0,1024);
	ParaGetServerIp(0, serverip);
	sport = ParaGetServerPort(0);
	sprintf(str, ",%d.%d.%d.%d,%d,", (uint16_t)serverip[0], (uint16_t)serverip[1], (uint16_t)serverip[2], (uint16_t)serverip[3], sport);
	len = UartRead(GPRSUART, (uint8_t *)readbuf, 1024);						
	if(len > 0){
		for(i = 0; i < len; i++){
			GprsInfo.ReBuf[GprsInfo.RecvWr] = readbuf[i];
			GprsInfo.RecvWr = (GprsInfo.RecvWr + 1) % ME4GDATAMAXLEN;
			if(GprsInfo.RecvWr == GprsInfo.RecvRd){
				GprsInfo.RecvRd = (GprsInfo.RecvRd + 1) % ME4GDATAMAXLEN;
			}	
		}
	}	
	len = GprsReadbuf((uint8_t *)readbuf, 1024);	
	pdata = readbuf;
	if( len ){
		offset = mystrstr(readbuf, "+", len, 1);  //ECV:
		if(offset){
			if((len +1 -offset) < 11){
				Recoverbuf(len +1 -offset);	
				return 0;
			}else{
				offset = mystrstr(readbuf, "+ZIPRECV:", len, 9);  //ECV:	
				if( offset ){	
					pdata2 = readbuf + offset - 1;
					pdata = readbuf + offset - 1;
					offset = mystrstr(pdata, ",", (uint32_t)(pdata - len), 1);
					if( offset ){        //接收到了套接字
						pdata = pdata + offset - 1;
						sn = *(pdata-1) - '1';
						if(sn < SNNUM){
							pdata += 1;
							offset = mystrstr(pdata, ",", (uint32_t)(pdata - len), 1);
							if( offset ){ //接收到了服务器IP 地址
								pdata = pdata + offset - 1;
								pdata += 1;
								offset = mystrstr(pdata, ",", (uint32_t)(pdata - len), 1);
								if( offset ){		 //接收到了端口号
									pdata = pdata + offset - 1;							
									pdata1 = pdata + 1;
									pdata += 1;
									offset = mystrstr(pdata, ",", (uint32_t)(pdata - len), 1);
									if( offset ){ //接收到了数据长度
										pdata = pdata + offset - 1;			
										pdata += 1;
										if(mystrstr(readbuf+(pdata2-readbuf), str, len - (pdata2 - readbuf), strlen(str))){//判断IP地址和端口号
											datalen = atoi((const char*)pdata1);  //数据长度
											if((len - (pdata - readbuf)) >= datalen + 2 ){ //数据长度加上 "\r\n"
												pdata1 = pdata + datalen;
												if(*pdata1 ==  '\r' && *(pdata1 + 1 ) == '\n'){
													for(i = 0; i < datalen; i++){ //  读出服务器发送的数据
														NetInfo4G.RecvBuf[sn][NetInfo4G.RecvWr[sn]] = *(pdata + i);  
														NetInfo4G.RecvWr[sn] = (NetInfo4G.RecvWr[sn] + 1) % NETDATAMAXLEN;			
														if(NetInfo4G.RecvWr[sn] == NetInfo4G.RecvRr[sn]){
															NetInfo4G.RecvRr[sn]  = (NetInfo4G.RecvRr[sn]) % BUFFER_LEN;
														}	
													}
													Recoverbuf(len - (pdata1 - readbuf));								
													return 1; 
												}else{
													Recoverbuf(len - (pdata2 - readbuf) + 9);	//接收数据错误			
													return 0; 
												}
											}else{
												Recoverbuf(len - (pdata2 - readbuf));	//本次数据接收不完整			
												return 0; 
											}	
										}else{
											Recoverbuf(len - (pdata2 - readbuf) + 9); //丢弃非本桩设置的IP地址端口号发来的数据
										}								
									}
								}
							}
						}
					}
					Recoverbuf(len - (pdata2 - readbuf));				//本次数据接收不完整				
					return 0;//数据长度不够
				}
			}	
		}
//		print("丢包 len = %d  %s\r\n",len ,readbuf);
		return 0;  //没有接收到服务器数据
	}	
	return 0;  //没有读到数据
}



/************************************************************************************************************
** 函 数 名 : GPRS_SendCmd
** 功能描述 : GPRS发送AT命令并读取接收数据
** 输    入 : cmd发送的AT命令指针 cmdlen AT命令字符串长度  ack GPRS应答字符串指针  acklen应答字符串长度  waittime 响应时间
** 输    出 : 无 
** 返    回 : 1 得到了预期的响应字符串，0  失败 
*************************************************************************************************************
*/
static uint8_t GPRS_SendCmd(uint8_t *cmd, uint16_t cmdlen, char *ack, uint16_t acklen, uint16_t waittime)
{
	uint32_t TimerCnt = 0; 	
	uint16_t len = 0;
	uint16_t recvlen = 0;
	uint16_t i;
	uint32_t offset;			
	uint8_t* pdata = NULL;
	uint16_t rssi = 0;
	
	UartWrite(GPRSUART, cmd, cmdlen);
	TimerCnt = TimerRead();
//	print("%s\r\n",cmd);
	if(ack && waittime){
		while(((TimerRead() - TimerCnt) * 10) <  waittime){
			OSTimeDlyHMSM(0,0,0,10);
			len = UartRead(GPRSUART, (uint8_t *)readbuf + recvlen, 1024);						
			if(len != 0){
				for(i = 0; i < len; i++){
					GprsInfo.ReBuf[GprsInfo.RecvWr] = readbuf[i + recvlen];
					GprsInfo.RecvWr = (GprsInfo.RecvWr + 1) % ME4GDATAMAXLEN;
					if(GprsInfo.RecvWr == GprsInfo.RecvRd){
						GprsInfo.RecvRd = (GprsInfo.RecvRd + 1) % ME4GDATAMAXLEN;
					}	
				}		
				recvlen += len;	
			}					
			if(recvlen){
				offset = mystrstr(readbuf, (char*)ack, recvlen, acklen);
				if( offset ){
					pdata = readbuf + offset - 1;
					if(memcmp(ack, "+CSQ:", 5) == 0){
						offset = mystrstr(pdata, ",", recvlen - offset + 1, 1);
						if( offset ){
							rssi = atoi((char*)pdata + 5);
							GprsInfo.Signal = rssi;
							if(rssi == 99){
								print("Error:CSQ:%d,No signal\r\n",rssi);
								OSTimeDlyHMSM(0,0,1,0);
							}
						}
					}
					GprsReadSocket();   
					return 1;		
				}	
			}				
		}
		return 0; 
	}
	return 1;
} 


/************************************************************************************************************
** 函 数 名 : GetSignal
** 功能描述 : 获取信号强度，  99 无信号   0-31  信号越好数值越大
** 输    入 : 无
** 输    出 : 无 
** 返    回 : 信号强度
*************************************************************************************************************
*/
uint8_t GetSignal(void)
{
	return GprsInfo.Signal;
}


/************************************************************************************************************
** 函 数 名 : GprsCon_
** 功能描述 : GPRS连接服务器函数
** 输    入 : sn socket值0-1 serverip服务器IP serverport服务器端口
** 输    出 : 无 
** 返    回 : 1成功 否则失败 
*************************************************************************************************************
*/

static uint8_t GprsCon_(uint8_t sn, uint8_t serverip[4], uint16_t port) //给外部的 API
{
	char Sendbuf[100] = {0};
	char ATBuf[30] = {0};
	static uint8_t ConErr[2];
	
	if(sn >= SNNUM){
		return 0;
	}
	if(NetInfo4G.LinkFlag != 1)  //如果没有连接
		return 0;

	sprintf(Sendbuf, "AT+ZIPSTAT=%d\r\n", sn + 1);
	sprintf(ATBuf, "+ZIPSTAT: %d,1", sn + 1);
	if(NetInfo4G.ConnState[sn] == 0){
		if(GPRS_SendCmd((uint8_t*)Sendbuf, strlen(Sendbuf), ATBuf, strlen(ATBuf), 1000)){
			NetInfo4G.ConnState[sn] = 1;
			ConErr[port] = 0;
			return 1;
		}else{	
			NetInfo4G.ConnState[sn] = 0;
		}	
	}
	
	if(TimerRead() - TimerSTART > T1S * 30 ){
		TimerSTART = TimerRead();
		if(GPRS_SendCmd((uint8_t*)Sendbuf, strlen(Sendbuf), ATBuf, strlen(ATBuf), 1000)){
			NetInfo4G.ConnState[sn] = 1;
			ConErr[port] = 0;
			return 1;
		}else{	
		NetInfo4G.ConnState[sn] = 0;
		}	
	}

	if(NetInfo4G.ConnState[sn] == 1)  //如果没有连接
		return 1;
	
	
	if(NetInfo4G.ConnState[sn] == 0){
		sprintf(ATBuf, "+ZIPSTAT: %d,1", sn + 1);
		sprintf(Sendbuf, "AT+ZIPOPEN=%d,0,%d.%d.%d.%d,%d\r\n", sn + 1, serverip[0], serverip[1], serverip[2], serverip[3], port);
		if(GPRS_SendCmd((uint8_t*)Sendbuf, strlen(Sendbuf), ATBuf, strlen(ATBuf), 3000 )){
			NetInfo4G.ConnState[sn] = 1;
			ConErr[port] = 0;
			return 1;
		}
		ConErr[port]++;
		if(ConErr[port] >= 10){
			ConErr[0] = 0;
			ConErr[1] = 0;
			print("PORT %d 连接%d.%d.%d.%d,%d %d 失败\r\n", port, serverip[0], serverip[1], serverip[2], serverip[3], port);
			print("模块重启\r\n");
			NetInfo4G.LinkFlag = 0;
		}
		
		return 0;
	}else{		
		ConErr[port] = 0;
		return 1;
	}
}


/************************************************************************************************************
** 函 数 名 : GPRSReset
** 功能描述 : GPRS初始化
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
static void GPRSReset(void)
{
	GPRS_RET_H();
	OSTimeDlyHMSM(0, 0, 3, 0);
	GPRS_RET_L();
	OSTimeDlyHMSM(0, 0, 4, 0);
}


/************************************************************************************************************
** 函 数 名 : GPRSInit
** 功能描述 : GPRS初始化
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void GPRSInit(void)
{
	GPRSReset();
	NetInfo4G.ConnState[0] = 0;
	NetInfo4G.ConnState[1] = 0;
	for(;;){
		UartFlush(GPRSUART); //清除串口缓存
		if( GPRS_SendCmd((uint8_t *)"AT\r\n", 4, "OK", 2, 2000) != 1){		
			NetInfo4G.LinkFlag = 0;
			print("Error:NO Module\r\n");
			OSTimeDlyHMSM (0,0,1,0);
			continue;
		}
		GPRS_SendCmd((uint8_t *)"ATE0\r\n", 6, "OK", 2, 200);//关闭回显 
		GPRS_SendCmd((uint8_t *)"AT+CSQ\r\n", 8, "+CSQ:", 5, 1000); //查询信号状态
		if(GPRS_SendCmd((uint8_t *)"AT+CPIN?\r\n", 10, "+CPIN: READY", 12, 1000) != 1){
			print("Error:NO SimCard\r\n");
			GPRSReset();
			continue;
		}
		GPRS_SendCmd((uint8_t *)"AT+COPS=0\r\n", 11, "OK\r\n", 4, 1000);//设置网络注册状态
		if(GPRS_SendCmd((uint8_t *)"AT+CREG?\r\n", 10, "+CREG: 0,1", 10, 1000) != 1){//查询网络注册状态 
			print("Error:Network registration failed\r\n");
			OSTimeDlyHMSM (0,0,0,100);
			continue;
		}	
		if(GPRS_SendCmd((uint8_t *)"AT+ZIPCALL?\r\n", 13, "+ZIPCALL: 1,", 12, 3000) != 1){
			OSTimeDlyHMSM (0,0,0,100);
			print("Error:Failed to get the link\r\n");
			if(GPRS_SendCmd((uint8_t *)"AT+ZIPCALL=1\r\n", 14, "+ZIPCALL: 1,", 12, 4000) == 1){
				OSTimeDlyHMSM (0,0,0,100);
				NetInfo4G.LinkFlag = 1;
				break;
			}
			continue;
		}
		break;
	}
	NetInfo4G.LinkFlag = 1;
}




/************************************************************************************************************
** 函 数 名 : GprsCon
** 功能描述 : 网络连接服务器
** 输    入 : sn socket值0到1  serverip服务器IP port服务器端口
** 输    出 : 无
** 返    回 : 1成功 否则失败
*************************************************************************************************************
*/
uint8_t GprsCon(uint8_t sn, uint8_t serverip[4], uint16_t port)
{
	if(NetInfo4G.LinkFlag != 1)  
		return 0;
	if(sn >= SNNUM)
		return 0;
	memcpy(NetInfo4G.serverip[sn], serverip, 4);
	NetInfo4G.serverport[sn] = port;
	NetInfo4G.socketportusedflag[sn] = 1;
	return NetInfo4G.ConnState[sn];	
}




/************************************************************************************************************
** 函 数 名 : GprsSend
** 功能描述 : 网络发送函数
** 输    入 : sn socket值0-7 buffer发送缓冲区 len发送数据字节长度
** 输    出 : 无
** 返    回 : 1成功 否则失败
*************************************************************************************************************
*/
uint8_t GprsSend(uint8_t sn, uint8_t *buffer, uint16_t len)
{
	if(sn >= SNNUM){
		return 0;
	}
	if(NetInfo4G.LinkFlag != 1){
		return 0;
	}
	if(NetInfo4G.socketportusedflag[sn] == 0){
		return 0;
	}
	if(NetInfo4G.ConnState[sn] != 1){
		return 0;
	}
	if(len > NETDATAMAXLEN){
		len = NETDATAMAXLEN;
	}
	memcpy(NetInfo4G.SendBuf[sn], buffer, len);
	NetInfo4G.SendLen[sn] = len;
	NetInfo4G.SendCmd[sn] = 1;
	while(NetInfo4G.SendCmd[sn] == 1){
		OSTimeDlyHMSM (0, 0, 1, 0);
	}
	if(NetInfo4G.SendState[sn] == 1){
		return 1;
	}
	return 0;
}




/************************************************************************************************************
** 函 数 名 : GprsRecv
** 功能描述 : 网络接收函数
** 输    入 : sn socket值0-7 buffer接收缓冲区 len接收数据字节长度
** 输    出 : 无
** 返    回 : 实际接收到的数据长度
*************************************************************************************************************
*/
uint16_t GprsRecv(uint8_t sn, uint8_t *buffer, uint16_t len) 
{
	uint16_t rlen = 0;
	
	if(sn >= SNNUM){
		return 0;
	}
	if(NetInfo4G.LinkFlag != 1){
		return 0;
	}
	if(NetInfo4G.socketportusedflag[sn] == 0){
		return 0;
	}
	if(NetInfo4G.ConnState[sn] != 1){
		return 0;
	}
	while (NetInfo4G.RecvRr[sn] != NetInfo4G.RecvWr[sn]){
		*buffer++ = NetInfo4G.RecvBuf[sn][NetInfo4G.RecvRr[sn]];
		NetInfo4G.RecvRr[sn] = (NetInfo4G.RecvRr[sn] + 1) % NETDATAMAXLEN;
		if (++rlen > len){
			return len;
		}
	}
	return rlen;
}




/************************************************************************************************************
** 函 数 名 : GprsProc
** 功能描述 : GPRS处理函数，任务调用
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/


void GprsProc(void)
{
	uint8_t sn;
	char Cmdbuf[40]={0};
	if(TimerRead() -TimerCALL > T1M){
		TimerCALL = TimerRead();
		if(GPRS_SendCmd((uint8_t *)"AT+ZIPCALL?\r\n", 13, "+ZIPCALL: 1,", 12, 3000) != 1){  // 查询网络注册状态	
		NetInfo4G.LinkFlag = 0;
		}
	}
	if(TimerRead() -TimerCPIN > (T1M *3)){
		TimerCPIN = TimerRead();
		if(GPRS_SendCmd((uint8_t *)"AT+CPIN?\r\n", 10, "+CPIN: READY", 12, 1000) != 1){ //检测电话卡
			NetInfo4G.LinkFlag = 0;
		}
	}
	if(NetInfo4G.LinkFlag == 0){			
		GPRSInit();
	}
	if(TimerRead() - TimerCSQ > T1M *3){
		TimerCSQ = TimerRead();
		GPRS_SendCmd((uint8_t *)"AT+CSQ\r\n", 8, "+CSQ:", 5, 1000); //查询信号状态
	}
	if( NetInfo4G.LinkFlag ){
		for(sn=0; sn < SNNUM; sn++){		
			if(NetInfo4G.socketportusedflag[sn] == 1){
				GprsCon_(sn, (uint8_t *)NetInfo4G.serverip[sn], NetInfo4G.serverport[sn]);
				if( NetInfo4G.SendCmd[sn] == 1 ){
					NetInfo4G.SendState[sn] = 0;
					if( NetInfo4G.ConnState[sn] ){
						sprintf(Cmdbuf, "AT+ZIPSENDRAW=%d,%d\r\n", sn + 1, NetInfo4G.SendLen[sn] );				
						if( GPRS_SendCmd((uint8_t *)Cmdbuf,strlen(Cmdbuf), ">", 1, 1000) ){//发送数据命令
							if( GPRS_SendCmd(NetInfo4G.SendBuf[sn], NetInfo4G.SendLen[sn], "IPSENDRAW:", 10, 2000)){
								NetInfo4G.SendState[sn] = 1;
							}
						}										
					}
				}
				NetInfo4G.SendCmd[sn] = 0;
			}
		}	
	}
	GprsReadSocket();   
}




/************************************************************************************************************
** 函 数 名 : GprsTask
** 功能描述 : GPRS任务函数
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void GprsTask(void* pdata)
{
	UartInit(GPRSUART,115200);   // 串口
	GPRSInit();
	while(1){	
		GprsProc();
		OSTimeDlyHMSM (0,0,0,10);		
	}
}



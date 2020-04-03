#include "includes.h"
#include <stdint.h>
#include <string.h>
#include "w5500.h"
#include "socket.h"
#include "NetTask.h"
#include "SPI.h"
#include "GPIO.h"
#include "Board.h"
#include "UART.h"
#include "includes.h"
#include "Para.h"
#include "timer.h"
static NETINFO EthInfo={0};
static uint16_t UdpPort = 0;      //接收的UDP端口号

#define UDPSOCKET  7

/************************************************************************************************************
** 函 数 名 : W5500_CS_Select
** 功能描述 : w5500选中函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void W5500_CS_Select(void)
{
	GpioSetLev(GPIO_SNETCS, LOW);
}

/************************************************************************************************************
** 函 数 名 : W5500_CS_Deselect
** 功能描述 : w5500不选中函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void W5500_CS_Deselect(void)
{
	GpioSetLev(GPIO_SNETCS, HIGH);
}

/************************************************************************************************************
** 函 数 名 : SPI_SendByte
** 功能描述 : SPI发送函数
** 输    入 : data 数据
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SPI_SendByte(uint8_t data)
{
	SpiWriteByte(ETHSPINO, data);
}

/************************************************************************************************************
** 函 数 名 : SPI_RecvByte
** 功能描述 : SPI接收函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	接收数据
*************************************************************************************************************
*/
static uint8_t SPI_RecvByte(void)
{
	uint8_t data;
	data = SpiReadByte(ETHSPINO);
	return data;
}


/************************************************************************************************************
** 函 数 名 : EthInit
** 功能描述 : 以太网初始化函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void EthInit(void)
{
	wiz_NetTimeout nettime,readbacknettime;
	wiz_NetInfo netinfo,readbacknetinfo;
	uint8_t arg;
	uint8_t txrxsize[_WIZCHIP_SOCK_NUM_*2] = {2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2};
	uint8_t data[6];
	
	SpiInit(ETHSPINO);   	// SPI 初始化
	GpioSetDir(GPIO_SNETRST,OUTPUT);  // 复位脚
	GpioSetLev(GPIO_SNETRST,LOW);
	OSTimeDlyHMSM (0, 0, 2, 0);
	GpioSetLev(GPIO_SNETRST,HIGH);
	OSTimeDlyHMSM (0, 0, 2, 0);
	GpioSetDir(GPIO_SNETCS,OUTPUT);  // 失能
	GpioSetLev(GPIO_SNETCS,HIGH);
	
	//注册接口
	reg_wizchip_cs_cbfunc(W5500_CS_Select, W5500_CS_Deselect);
	reg_wizchip_spi_cbfunc(SPI_RecvByte, SPI_SendByte);
	reg_wizchip_cris_cbfunc(OS_ENTER_CRITICAL, OS_EXIT_CRITICAL);
	//软件复位W5500
	ctlwizchip(CW_RESET_WIZCHIP,&arg);
	//设置网络参数
	ParaGetPhyMac(data);
	memcpy(netinfo.mac, data, 6);
	ParaGetLocalIp(data);
	memcpy(netinfo.ip, data, 4);
	ParaGetSubMask(data);
	memcpy(netinfo.sn, data, 4);
	ParaGetGateWay(data);
	memcpy(netinfo.gw, data, 4);
	memset(netinfo.dns,0,4);
	netinfo.dhcp=NETINFO_STATIC;
	ctlnetwork(CN_SET_NETINFO,&netinfo);
	ctlnetwork(CN_GET_NETINFO,&readbacknetinfo);
	//设置超时参数
	nettime.retry_cnt=3;
	nettime.time_100us=2000;
	ctlnetwork(CN_SET_TIMEOUT,&nettime);
	ctlnetwork(CN_GET_TIMEOUT,&readbacknettime);
	//设置发送接收缓冲区大小
	ctlwizchip(CW_INIT_WIZCHIP,txrxsize);
}

/************************************************************************************************************
** 函 数 名 : EthConn
** 功能描述 : 以太网连接函数
** 输    入 : sn socket值0-1 serverip服务器IP serverport服务器端口
** 输    出 : 无
** 返    回 :	1成功 否则失败
*************************************************************************************************************
*/
uint8_t EthConn(uint8_t sn, uint8_t serverip[4], uint16_t serverport)
{
	if(sn >= SNNUM){
		return 0;
	}
	if(EthInfo.LinkFlag != 1){
		return 0;
	}
	print("Connect to %d.%d.%d.%d,%d P%d:\r\n", serverip[0], serverip[1], serverip[2], serverip[3], serverport, sn);
	memcpy(EthInfo.serverip[sn], serverip, 4);
	EthInfo.serverport[sn] = serverport;
	EthInfo.socketportusedflag[sn] = 1;
	EthInfo.ConnCmd[sn] = 1;
	while(EthInfo.ConnCmd[sn] == 1){
		OSTimeDlyHMSM (0, 0, 0, 100);
	}
	if(EthInfo.ConnState[sn] == 1){
		print("Connect success P%d:\r\n", sn);
		return 1;
	} else {
		print("Connect fail P%d:\r\n", sn);
		return 0;
	}
}

/************************************************************************************************************
** 函 数 名 : EthSend
** 功能描述 : 以太网发送函数
** 输    入 : sn socket值0-1 buffer发送缓冲区 len发送数据字节长度
** 输    出 : 无
** 返    回 :	1成功 否则失败
*************************************************************************************************************
*/
uint8_t EthSend(uint8_t sn, uint8_t *buffer, uint16_t len)
{
	if(sn >= SNNUM){
		return 0;
	}
	if(EthInfo.LinkFlag != 1){
		return 0;
	}
	if(EthInfo.socketportusedflag[sn] == 0){
		return 0;
	}
	if(EthInfo.ConnState[sn] != 1){
		return 0;
	}
	if(len > NETDATAMAXLEN){
		len = NETDATAMAXLEN;
	}
	memcpy(EthInfo.SendBuf[sn], buffer, len);
	EthInfo.SendLen[sn] = len;
	EthInfo.SendCmd[sn] = 1;
	while(EthInfo.SendCmd[sn] == 1){
		OSTimeDlyHMSM (0, 0, 1, 0);
	}
	if(EthInfo.SendState[sn] == 1){
		return 1;
	}
	return 0;
}

/************************************************************************************************************
** 函 数 名 : EthRecv
** 功能描述 : 以太网接收函数
** 输    入 : sn socket值0-1 buffer接收缓冲区 len接收数据字节长度
** 输    出 : 无
** 返    回 :	实际接收到的数据长度
*************************************************************************************************************
*/
uint16_t EthRecv(uint8_t sn, uint8_t *buffer, uint16_t len)
{
	uint16_t rlen = 0;
	
	if(sn >= SNNUM){
		return 0;
	}
	if(EthInfo.LinkFlag != 1){
		return 0;
	}
	if(EthInfo.socketportusedflag[sn] == 0){
		return 0;
	}
	if(EthInfo.ConnState[sn] != 1){
		return 0;
	}
	while (EthInfo.RecvRr[sn] != EthInfo.RecvWr[sn]){
		*buffer++ = EthInfo.RecvBuf[sn][EthInfo.RecvRr[sn]];
		EthInfo.RecvRr[sn] = (EthInfo.RecvRr[sn] + 1) % NETDATAMAXLEN;
		if (++rlen > len){
			return len;
		}
	}
	return rlen;
}






/************************************************************************************************************
** 函 数 名 : EthHandle
** 功能描述 : 以太网处理函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void EthHandle(void)
{
	static uint32_t timer = 0;
	int32_t ret;
	uint8_t arg,sn;
	uint16_t port = 9000,i,sentsize,size;
	uint8_t buffer[NETDATAMAXLEN];
	
	//查询link状态
	ret = ctlwizchip(CW_GET_PHYLINK, &arg);
	if((ret != 0) || (arg != PHY_LINK_ON)){
		if(EthInfo.LinkFlag == 1){
			EthInfo.LinkFlag = 0;
			for(sn = 0; sn < 8; sn++){
				close(sn);
			}
		}
		if (TimerRead() - timer >= T1M){
			timer = TimerRead();
			EthInit();
			print("net fail\r\n");
		}
		return;
	} else {
		EthInfo.LinkFlag = 1;
		timer = TimerRead();
	}

	//数据处理
	for(sn = 0; sn < SNNUM; sn++){
		if(EthInfo.socketportusedflag[sn] == 1){
			//处理重连要求
			if(EthInfo.ConnCmd[sn] == 1){
				close(sn);
				EthInfo.ConnState[sn] = 0;
				if( (ret = socket(sn, Sn_MR_TCP, port + sn, 0x00)) == sn){
					if( (ret = connect(sn, EthInfo.serverip[sn], EthInfo.serverport[sn])) == SOCK_OK){
						EthInfo.ConnState[sn] = 1;
					}
				}
				memset(EthInfo.RecvBuf[sn], 0, NETDATAMAXLEN);
				EthInfo.RecvWr[sn] = 0;
				EthInfo.RecvRr[sn] = 0;
				EthInfo.ConnCmd[sn] = 0;
			}
			//处理发送数据要求
			if(EthInfo.SendCmd[sn] == 1){
				EthInfo.SendState[sn] = 0;
				ret = getSn_SR(sn);
				if (ret == SOCK_ESTABLISHED){
					sentsize = 0;
					while(sentsize != EthInfo.SendLen[sn]){
						ret = send(sn, EthInfo.SendBuf[sn] + sentsize, EthInfo.SendLen[sn] - sentsize);
						if(ret < 0) {
							break;
						}
						sentsize += ret;
					}
					if(sentsize == EthInfo.SendLen[sn]){
						EthInfo.SendState[sn] = 1;
					}
				}
				EthInfo.SendCmd[sn] = 0;
			}
			//处理接收数据
			ret = getSn_SR(sn);
			if (ret == SOCK_ESTABLISHED){
				if(getSn_IR(sn) & Sn_IR_CON){
					setSn_IR(sn, Sn_IR_CON);
				}
				if((size = getSn_RX_RSR(sn)) > 0){
					if(size > NETDATAMAXLEN){
						size = NETDATAMAXLEN;
					}
					ret = recv(sn, buffer, size);
					if(ret > 0){
						for(i = 0; i < ret; i++){
							EthInfo.RecvBuf[sn][EthInfo.RecvWr[sn]] = buffer[i];
							EthInfo.RecvWr[sn] = (EthInfo.RecvWr[sn] + 1) % NETDATAMAXLEN;
						}
					}
				}
			} else if (ret == SOCK_CLOSED){
				close(sn);
				EthInfo.ConnState[sn] = 0;
			}
		}
	}
}


/************************************************************************************************************
** 函 数 名 : UDPSend
** 功能描述 : UDP发送函数
** 输    入 : buf 发送的buff  , len   发送的长度
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void UDPSend(uint8_t* buf, uint16_t len)
{
	uint8_t ip[4]={255,255,255,255};
	sendto(UDPSOCKET, buf,len, ip, UdpPort);                       // 接收到数据后再回给远程上位机，完成数据回环
}





/************************************************************************************************************
** 函 数 名 : AnalyseMsgFromUdpPlatform
** 功能描述 : 处理UDP通道配置函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void AnalyseMsgFromUdpPlatform(void)
{
	uint16_t len, i;
	static uint8_t buffer[512];
	int32_t ret;
	uint8_t arg;
	static uint16_t readlen;
	uint8_t UdpIp[4]={255,255,255,255};
	ret = ctlwizchip(CW_GET_PHYLINK, &arg);
	if((ret != 0) || (arg != PHY_LINK_ON))
		return;
	switch(getSn_SR(UDPSOCKET))                                                         // 获取socket0的状态
	{
		case SOCK_UDP:          // Socket处于初始化完成(打开)状态
			if(getSn_IR(UDPSOCKET) & Sn_IR_RECV){
				setSn_IR(UDPSOCKET, Sn_IR_RECV);   // Sn_IR的RECV位置1
			}
			if((len = getSn_RX_RSR(UDPSOCKET))>0){
				if(readlen + len > sizeof(buffer)){
					readlen = 0; // 丢包
				}
				if(len > sizeof(buffer)){
					len = sizeof(buffer); // 丢包
				}
				recvfrom(UDPSOCKET, buffer + readlen, len, UdpIp, &UdpPort); // W5500接收来自远程上位机的数据，并通过SPI发送给MCU		
				readlen +=len;
				if (readlen){
					for (i = 0; i < readlen; i++){
						if (ConfigProtocolFrameCheck(buffer + i, PARACHANNEL_UDP) == 1){
							if ((buffer[i + 1 + 33] & 0x1f) ==0x01){
								DebugPlatformReadDataAnalyse(&buffer[i], PARACHANNEL_UDP);
							} else if ((buffer[i + 1 + 33] & 0x1f) ==0x11){
								DebugPlatformWriteDataAnalyse(&buffer[i], PARACHANNEL_UDP);
							}
							memset(buffer, 0, sizeof(buffer));
							readlen = 0;
							break;
						}
					}		
				}
			}          
			break;
		case SOCK_CLOSED:                                                       // Socket处于关闭状态
			socket(UDPSOCKET,Sn_MR_UDP,2408, 0);                                   // 打开Socket0，并配置为UDP模式，打开一个本地端口
			break;
		default:
				break;
	}
}








/************************************************************************************************************
** 函 数 名 : EthTask
** 功能描述 : 以太网任通信务
** 输    入 : pdata 未使用
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/

/*
void EthTask(void *pdata)
{
	print("net task start...\r\n");
  EthInit();
	
	while(1){
		EthHandle();
	
		OSTimeDlyHMSM (0, 0, 0, 15);
	}
}


*/




#include <stdint.h>
#include "includes.h"
#include "Protocol.h"
#include "Para.h"
#include "RTC.h"
#include "Cfg.h"
#include "Record.h"
#include "Timer.h"
#include "MyAlgLib.h"
#include "Board.h"
#include "NetTask.h"
#include <LPC177x_8x.H>
#include "UART.h"
#include "MainTask.h"
#include "ChgTask.h"
#include "MyAlgLib.h"
#include <stdio.h>
#include <math.h>
#include "ElmTask.h"

static COMMINFO CommInfo[2];

static struct{
	uint32_t ReportOnlineCardID;//上报在线卡卡号
	uint8_t ReportFlag;//上报204报文标志 0不需上报 1需上报 2上报已得到回复
	uint8_t ReportRet;//上报204报文收到的结果 0有效帐户 1非法帐户 2余额不足 3把黑卡恢复成正常卡 4把正常卡变黑卡 5非法卡号 6挂失 7销卡 8密码错误 9该卡已经在别的桩充电
	uint32_t OnlineCardIDBalance;//在线卡余额 单位0.01元
}ReportCMD204[2];

/************************************************************************************************************
** 函 数 名 : addChecksum
** 功能描述 : 校验和计算函数
** 输    入 : buffer 缓冲区 length 缓冲区长度
** 输    出 : 无
** 返    回 :	校验和
*************************************************************************************************************
*/
static uint8_t addChecksum(const uint8_t* buffer, uint32_t length)
{
	uint32_t sum=0;
	uint32_t i=0;
	
	if (!buffer){
		return 0;
	}
	for (i=0; i<length; i++){
		sum = sum + *buffer;
		buffer++;
	}
	return sum&0xff;
}

/************************************************************************************************************
** 函 数 名 : SendCMD102
** 功能描述 : 发送充电桩上传心跳包信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SendCMD102(uint8_t port)
{
	static uint16_t snum[2];
	uint8_t buffer[COMMBUFFERLEN], *pbuf,i;
	
	pbuf = buffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x2F; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x41; //序列号域
	*pbuf++ = 102; //命令代码
	*pbuf++ = 0;
	*pbuf++ = 0x00; //数据域
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	ParaGetPileNo(0, pbuf);
	for (i = 0; i < 32; i++){
		if (pbuf[i] == 0x00){
			pbuf[i] = port + 'A';
			break;
		}
	}
	pbuf += 32;
	*pbuf++ = snum[port];
	*pbuf++ = snum[port] >> 8;
	snum[port]++;
	*pbuf++ = addChecksum(buffer + 6, pbuf - buffer - 6);
	
	NetSend(port, buffer, pbuf - buffer);
	print("CMD102 发送充电桩上传心跳包信息P%d:", port);
	printx(buffer, pbuf - buffer);
}

/************************************************************************************************************
** 函 数 名 : RecvCMD1
** 功能描述 : 接收后台服务器下发充电桩整型工作参数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD1(uint8_t port, uint8_t *buffer)
{
	uint8_t ackbuffer[COMMBUFFERLEN], *pbuf, type,ret;
	uint32_t paraaddr,num,i,paranum,data32;
	
	paraaddr = buffer[13] | (buffer[14] << 8) | (buffer[15] << 16) | (buffer[16] << 24);
	type = buffer[12];
	if(type == 0x00){ //查询
		ret = 0;
		memset(ackbuffer, 0, sizeof(ackbuffer));
		paranum = buffer[17];
		for(i = 0; i < paranum; i++){
			switch(paraaddr++){
				case 1: //签到时间间隔
					data32 = ParaGetLoginTimeInterval(port);
					memcpy(&ackbuffer[51 + i * 4], &data32, 4);
					break;
				case 21: //心跳上报周期
					data32 = ParaGetNetHeartTime(port);
					memcpy(&ackbuffer[51 + i * 4], &data32, 4);
					break;
				case 22: //心跳包检测超时次数
					data32 = ParaGetNetCommOverCnt(port);
					memcpy(&ackbuffer[51 + i * 4], &data32, 4);
					break;
				case 23: //充电桩状态信息报上报周期
					data32 = ParaGetStateInfoReportPeriod(port);
					memcpy(&ackbuffer[51 + i * 4], &data32, 4);
					break;
				case 25: //中心服务器地址
					ParaGetServerIp(port, &ackbuffer[51 + i * 4]);
					break;
				case 26: //中心服务器端口
					data32 = ParaGetServerPort(port);
					memcpy(&ackbuffer[51 + i * 4], &data32, 4);
					break;
				default:
					ret = 1;
					break;
			}
		}
		if (ret != 0){
			num =0;
		} else {
			num = i;
		}
		pbuf = ackbuffer;
		*pbuf++ = STARTCODE1; //起始域
		*pbuf++ = STARTCODE2;
		*pbuf++ = 9 + 43 + num * 4; //长度域
		*pbuf++ = 0x00;
		*pbuf++ = MSGCODE; //信息域
		*pbuf++ = 0x00; //序列号域
		*pbuf++ = 2; //命令代码
		*pbuf++ = 0;
		*pbuf++ = 0x00; //数据域
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		ParaGetPileNo(0, pbuf);
		for (i = 0; i < 32; i++){
			if (pbuf[i] == 0x00){
				pbuf[i] = port + 'A';
				break;
			}
		}
		pbuf += 32;
		*pbuf++ = buffer[12];
		memcpy(pbuf, &buffer[13], 4);
		pbuf += 4;
		*pbuf++ = buffer[17];
		*pbuf++ = ret;
		pbuf += num * 4;
		*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
		
		NetSend(port, ackbuffer, pbuf - ackbuffer);
		print("CMD2 充电桩参数整形查询应答P%d:", port);
		printx(ackbuffer, pbuf - ackbuffer);
	}
	if(type == 0x01){ //设置
		ret = 0;
		paranum = buffer[17];
		num = buffer[18] | (buffer[19] << 8);
		for(i = 0; i < paranum; i++){
			switch(paraaddr++){
				case 1: //签到时间间隔
					data32 = buffer[20 + i * 4] | (buffer[21 + i * 4] << 8);
					ParaSetLoginTimeInterval(port, data32);
					break;
				case 21: //心跳上报周期
					data32 = buffer[20 + i * 4];
					ParaSetNetHeartTime(port, data32);
					break;
				case 22: //心跳包检测超时次数
					data32 = buffer[20 + i * 4];
					ParaSetNetCommOverCnt(port, data32);
					break;
				case 23: //充电桩状态信息报上报周期
					data32 = buffer[20 + i * 4] | (buffer[21 + i * 4] << 8);
					ParaSetStateInfoReportPeriod(port, data32);
					break;
				case 25: //中心服务器地址
					ParaSetServerIp(port, &buffer[20 + i * 4]);
					break;
				case 26: //中心服务器端口
					data32 = buffer[20 + i * 4] | (buffer[21 + i * 4] << 8);
					ParaSetServerPort(port, data32);
					break;
				default:
					ret = 1;
					break;
			}
		}
		pbuf = ackbuffer;
		*pbuf++ = STARTCODE1; //起始域
		*pbuf++ = STARTCODE2;
		*pbuf++ = 0x34; //长度域
		*pbuf++ = 0x00;
		*pbuf++ = MSGCODE; //信息域
		*pbuf++ = 0x00; //序列号域
		*pbuf++ = 2; //命令代码
		*pbuf++ = 0;
		*pbuf++ = 0x00; //数据域
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		ParaGetPileNo(0, pbuf);
		for (i = 0; i < 32; i++){
			if (pbuf[i] == 0x00){
				pbuf[i] = port + 'A';
				break;
			}
		}
		pbuf += 32;
		*pbuf++ = buffer[12];
		memcpy(pbuf, &buffer[13], 4);
		pbuf += 4;
		*pbuf++ = buffer[17];
		*pbuf++ = ret;
		*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
		
		NetSend(port, ackbuffer, pbuf - ackbuffer);
		print("CMD2 充电桩参数整形设置应答P%d:", port);
		printx(ackbuffer, pbuf - ackbuffer);
	}
}

/************************************************************************************************************
** 函 数 名 : RecvCMD3
** 功能描述 : 接收后台服务器下发充电桩字符型工作参数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD3(uint8_t port, uint8_t *buffer)
{
	uint8_t ackbuffer[COMMBUFFERLEN], *pbuf, type,meteraddr[6],qrcode[256],oldpileno[32],oldpilenoflag = 0;
	uint32_t paraaddr,num,i;
	Rtc datetime;
	
	paraaddr = buffer[13] | (buffer[14] << 8) | (buffer[15] << 16) | (buffer[16] << 24);
	type = buffer[12];
	if(type == 0x00){ //查询
		memset(ackbuffer, 0, sizeof(ackbuffer));
		switch(paraaddr){
			case 1: //充电桩编码
//				ParaGetPileNo(port, &ackbuffer[50]);
				num = 32;
				break;
			case 2: //标准时钟时间
				RtcRead(&datetime);
				ackbuffer[50 + 0] = Hex32ToBcd32(datetime.year / 100);
				ackbuffer[50 + 1] = Hex32ToBcd32(datetime.year % 100);
				ackbuffer[50 + 2] = Hex32ToBcd32(datetime.month);
				ackbuffer[50 + 3] = Hex32ToBcd32(datetime.day);
				ackbuffer[50 + 4] = Hex32ToBcd32(datetime.hour);
				ackbuffer[50 + 5] = Hex32ToBcd32(datetime.min);
				ackbuffer[50 + 6] = Hex32ToBcd32(datetime.sec);
				ackbuffer[50 + 7] = 0xFF;
				num = 8;
				break;
			case 3: //管理员密码
				memset(&ackbuffer[50], 0, 8);
				ackbuffer[50 + 0] = '\0';
				num = 8;
				break;
			case 4: //操作员密码
				memset(&ackbuffer[50], 0, 8);
				ackbuffer[50 + 0] = '\0';
				num = 8;
				break;
			case 5: //MAC地址 预留
				memset(&ackbuffer[50], 0, 6);
				ackbuffer[50 + 0] = '\0';
				num = 6;
				break;
			case 6: //预留
				memset(&ackbuffer[50], 0, 16);
				ackbuffer[50 + 0] = '\0';
				num = 16;
				break;
			case 7: //二维码
				ParaGetQRCode(port, qrcode);
				num = 0;
				for(i = 0; i < 256; i++){
					num ++;
					if(qrcode[i] == '\0')
						break;
					ackbuffer[50 + i] = qrcode[i];
				}
				ackbuffer[50 + i] = '\0';
				break;
			case 8: //客户服务热线1 预留
				ackbuffer[50 + 0] = '\0';
				num = 1;
				break;
			case 9: //客户服务热线2 预留
				ackbuffer[50 + 0] = '\0';
				num = 1;
				break;
			case 10: //用户支付二维码 预留
				ackbuffer[50 + 0] = '\0';
				num = 1;
				break;
			case 11: //桩主界面二维码前缀
				ackbuffer[50 + 0] = '\0';
				num = 1;
				break;
			case 12: //DLT645-2007
				ParaGetACMeterAddr(port, meteraddr);
				//ParaGetDCMeterAddr(port, meteraddr);
				ackbuffer[50] = (meteraddr[5] >> 4) | 0x30;
				ackbuffer[51] = (meteraddr[5] & 0x0f) | 0x30;
				ackbuffer[52] = (meteraddr[4] >> 4) | 0x30;
				ackbuffer[53] = (meteraddr[4] & 0x0f) | 0x30;
				ackbuffer[54] = (meteraddr[3] >> 4) | 0x30;
				ackbuffer[55] = (meteraddr[3] & 0x0f) | 0x30;
				ackbuffer[56] = (meteraddr[2] >> 4) | 0x30;
				ackbuffer[57] = (meteraddr[2] & 0x0f) | 0x30;
				ackbuffer[58] = (meteraddr[1] >> 4) | 0x30;
				ackbuffer[59] = (meteraddr[1] & 0x0f) | 0x30;
				ackbuffer[60] = (meteraddr[0] >> 4) | 0x30;
				ackbuffer[61] = (meteraddr[0] & 0x0f) | 0x30;
				num = 12;
				break;
			default:
				break;
		}
		pbuf = ackbuffer;
		*pbuf++ = STARTCODE1; //起始域
		*pbuf++ = STARTCODE2;
		*pbuf++ = 9 + 42 + num; //长度域
		*pbuf++ = 0x00;
		*pbuf++ = MSGCODE; //信息域
		*pbuf++ = 0x00; //序列号域
		*pbuf++ = 4; //命令代码
		*pbuf++ = 0;
		*pbuf++ = 0x00; //数据域
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		ParaGetPileNo(0, pbuf);
		for (i = 0; i < 32; i++){
			if (pbuf[i] == 0x00){
				pbuf[i] = port + 'A';
				break;
			}
		}
		pbuf += 32;
		*pbuf++ = buffer[12];
		memcpy(pbuf, &buffer[13], 4);
		pbuf += 4;
		*pbuf++ = 0;
		pbuf += num;
		*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
		
		NetSend(port, ackbuffer, pbuf - ackbuffer);
		print("CMD4 充电桩参数字符形查询应答P%d:", port);
		printx(ackbuffer, pbuf - ackbuffer);
	}
	if(type == 0x01){ //设置
		switch(paraaddr){
			case 1: //充电桩编码
				ParaGetPileNo(port, oldpileno);
				oldpilenoflag = 1;
				ParaSetPileNo(port, &buffer[19]);
				break;
			case 2: //标准时钟时间
				datetime.year = Bcd32ToHex32(buffer[19]) * 100 + Bcd32ToHex32(buffer[19 + 1]);
				datetime.month = Bcd32ToHex32(buffer[19 + 2]);
				datetime.day = Bcd32ToHex32(buffer[19 + 3]);
				datetime.hour = Bcd32ToHex32(buffer[19 + 4]);
				datetime.min = Bcd32ToHex32(buffer[19 + 5]);
				datetime.sec = Bcd32ToHex32(buffer[19 + 6]);
				RtcWrite(&datetime);
				break;
			case 3: //管理员密码
				break;
			case 4: //操作员密码
				break;
			case 5: //MAC地址 预留
				break;
			case 6: //预留
				break;
			case 7: //二维码 预留
				memset(qrcode, 0, 256);
				num = 0;
				for(i = 0; i < 256; i++){
					num ++;
					if(buffer[19 + i] == '\0')
						break;
				}
				memcpy(qrcode, &buffer[19], num);
				ParaSetQRCode(port, qrcode);
				break;
			case 8: //客户服务热线1 预留
				break;
			case 9: //客户服务热线2 预留
				break;
			case 10: //用户支付二维码 预留
				break;
			case 11: //桩主界面二维码前缀
				for(i = 0; i < 128; i++){
					num ++;
					if(buffer[19 + i] == '\0'){
						break;
					}
				}
				break;
			case 12: //DLT645-2007电表地址
				meteraddr[5] = ((buffer[19] & 0x0f) << 4) | (buffer[20] & 0x0f);
				meteraddr[4] = ((buffer[21] & 0x0f) << 4) | (buffer[22] & 0x0f);
				meteraddr[3] = ((buffer[23] & 0x0f) << 4) | (buffer[24] & 0x0f);
				meteraddr[2] = ((buffer[25] & 0x0f) << 4) | (buffer[26] & 0x0f);
				meteraddr[1] = ((buffer[27] & 0x0f) << 4) | (buffer[28] & 0x0f);
				meteraddr[0] = ((buffer[29] & 0x0f) << 4) | (buffer[30] & 0x0f);
				ParaSetACMeterAddr(port, meteraddr);
				//ParaSetDCMeterAddr(port, meteraddr);
				break;
			default:
				break;
		}
		pbuf = ackbuffer;
		*pbuf++ = STARTCODE1; //起始域
		*pbuf++ = STARTCODE2;
		*pbuf++ = 0x33; //长度域
		*pbuf++ = 0x00;
		*pbuf++ = MSGCODE; //信息域
		*pbuf++ = 0x00; //序列号域
		*pbuf++ = 4; //命令代码
		*pbuf++ = 0;
		*pbuf++ = 0x00; //数据域
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		*pbuf++ = 0x00;
		if (oldpilenoflag == 1){
			memcpy(pbuf, oldpileno, 32);
		} else {
			ParaGetPileNo(port, pbuf);
		}
		pbuf += 32;
		*pbuf++ = buffer[12];
		memcpy(pbuf, &buffer[13], 4);
		pbuf += 4;
		*pbuf++ = 0;
		*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
		
		NetSend(port, ackbuffer, pbuf - ackbuffer);
		print("CMD4 充电桩参数字符形设置应答P%d:", port);
		printx(ackbuffer, pbuf - ackbuffer);
	}
}

/************************************************************************************************************
** 函 数 名 : RecvCMD5
** 功能描述 : 接收后台服务器下发充电桩控制命令
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD5(uint8_t port, uint8_t *buffer)
{
	uint8_t ackbuffer[COMMBUFFERLEN], *pbuf,ret = 1,rebootflag = 0,i;
	uint32_t cmdaddr;//,cmdnum,cmdlen;
	
	cmdaddr = buffer[13] | (buffer[14] << 8) | (buffer[15] << 16) | (buffer[16] << 24);
//	cmdnum = buffer[17];
//	cmdlen = buffer[18] | (buffer[19] << 8);
	switch(cmdaddr){
		case 2: //停止充电
			if(buffer[20] == 0x55){
				//应增加停止充电函数
				CommInfo[port].stopflag = 1;
				ret = 0;
			}
			break;
		case 10: //取消预约
			ret = 1;
			break;
		case 11: //设备重启
			rebootflag = 1;
			break;
		default:
			ret = 0;
			break;
	}
	
	pbuf = ackbuffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x34; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x00; //序列号域
	*pbuf++ = 0x06; //命令代码
	*pbuf++ = 0x00;
	memset(pbuf, 0, 4);//数据域
	pbuf += 4;
	ParaGetPileNo(0, pbuf);
	for (i = 0; i < 32; i++){
		if (pbuf[i] == 0x00){
			pbuf[i] = port + 'A';
			break;
		}
	}
	pbuf += 32;
	*pbuf++ = buffer[12];
	memcpy(pbuf, &buffer[13], 4);
	pbuf += 4;
	*pbuf++ = buffer[17];
	*pbuf++ = ret;
	*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
	
	NetSend(port, ackbuffer, pbuf - ackbuffer);
	print("CMD6 充电桩对后台控制命令应答P%d:", port);
	printx(ackbuffer, pbuf - ackbuffer);
	
	if (rebootflag == 1){
		print("准备重启设备..................\r\n\n\n");
		OSTimeDlyHMSM (0, 0, 3, 0);
		//是否需要保存数据再重启
		NVIC_SystemReset();
	}
}

/************************************************************************************************************
** 函 数 名 : RecvCMD7
** 功能描述 : 接收后台服务器下发充电桩开启充电控制命令
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD7(uint8_t port, uint8_t *buffer)
{
	if(buffer[14] != 0x00){ //充电生效类型不支持
		return;
	}
	memcpy(&CommInfo[port].UserId, &buffer[8], 2);//用户ID
	CommInfo[port].chargestrategy = buffer[21]; //充电策略
	CommInfo[port].chargepara = buffer[25] | (buffer[26] << 8) | (buffer[27] << 16) | (buffer[28] << 24); //充电策略参数
	memcpy(CommInfo[port].ChargeCardNumber, &buffer[38], 32);//充电卡号/用户识别号
	CommInfo[port].startflag = 1;
}

/************************************************************************************************************
** 函 数 名 : SendCMD8
** 功能描述 : 发送充电桩对后台下发的充电桩开启充电控制应答
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... failcode 0代表成功
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SendCMD8(uint8_t port, uint32_t failcode)
{
	uint8_t buffer[COMMBUFFERLEN], *pbuf,i;
	
	pbuf = buffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x32; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x00; //序列号域
	*pbuf++ = 8; //命令代码
	*pbuf++ = 0;
	*pbuf++ = 0x00; //数据域
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	ParaGetPileNo(0, pbuf);
	for (i = 0; i < 32; i++){
		if (pbuf[i] == 0x00){
			pbuf[i] = port + 'A';
			break;
		}
	}
	pbuf += 32;
	if(ParaGetChgGunNumber() > 1){
		*pbuf++ = port + 1;
	} else {
		*pbuf++ = 0;
	}
	memcpy(pbuf, &failcode, 4);
	pbuf += 4;
	*pbuf++ = addChecksum(buffer + 6, pbuf - buffer - 6);
	
	NetSend(port, buffer, pbuf - buffer);
	print("CMD8 充电桩对后台下发的充电桩开启充电控制应答P%d:", port);
	printx(buffer, pbuf - buffer);
}

/************************************************************************************************************
** 函 数 名 : SendCMD104
** 功能描述 : 发送充电桩状态信息包上报
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SendCMD104(uint8_t port)
{
	uint8_t buffer[COMMBUFFERLEN], *pbuf,datetime[8],i;
	uint32_t data32,data1,data2,data3,data4,data5,data6;
	uint8_t MainState;
	CHARGEINFO info;
	uint32_t alarmcode;
	
	MainState = GetMainCharge(port);
	info = GetCCBInfo(port);
	pbuf = buffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0xB2; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x41; //序列号域
	*pbuf++ = 104; //命令代码
	*pbuf++ = 0;
	
	memcpy(pbuf, &CommInfo[port].UserId, 2);
	pbuf+= 2;
//	*pbuf++ = 0x00; //数据域
//	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	ParaGetPileNo(0, pbuf); //充电桩编码
	for (i = 0; i < 32; i++){
		if (pbuf[i] == 0x00){
			pbuf[i] = port + 'A';
			break;
		}
	}
	pbuf += 32;
	*pbuf++ = ParaGetChgGunNumber(); //充电枪数量 1/2
	if (ParaGetChgGunNumber() > 1){
		*pbuf++ = port + 1; //充电口号
	} else {
		*pbuf++ = 0;
	}
	*pbuf++ = 0x01; //充电枪类型 01直流 02交流
	if (MainState == 0){
		*pbuf++ = 0; //工作状态 空闲
	} else if (MainState == 1){
		*pbuf++ = 2; //工作状态 充电进行中
	} else if (MainState == 2){
		*pbuf++ = 3; //工作状态 充电结束
	} else {
		*pbuf++ = 0;
	}
	*pbuf++ = info.SOC; //当前SOC
	if (info.ScramState == 1)//急停故障
		alarmcode = 3;
	else if (info.DoorState == 1)//门禁故障
		alarmcode = 4;
	else if(ElmGetCommState(port) == 1)//电表通讯异常
		alarmcode = 1002;
	else
		alarmcode = 0;
	memcpy(pbuf, &alarmcode, 4);//当前最高告警编码
	pbuf += 4;
	if (info.GunSeatState == 0){
		*pbuf++ = 0; //车连接状态 断开
	} else {
		*pbuf++ = 2; //车连接状态 连接
	}
	data32 = GetSumMoney(port); //本次充电累计充电费用
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	memset(pbuf, 0x00, 8); //内部变量2,3
	pbuf += 8;
	GetVoltage(port, &data32, &data2, &data3);//直流充电电压
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	GetCurrent(port, &data32, &data2, &data3);//直流充电电流
	data32 /= 100;
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = info.BmsDemandVol; //BMS需求电压
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = (4000 - info.BmsDemandCur) * 10; //BMS需求电流
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	*pbuf++ = info.Mode; //BMS充电模式
	data32 = 0;//交流A相充电电压 需改----------------
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = 0;//交流B相充电电压 需改----------------
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = 0;//交流C相充电电压 需改----------------
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = 0;//交流A相充电电流 需改----------------
	data32 /= 100;
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = 0;//交流B相充电电流 需改----------------
	data32 /= 100;
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = 0;//交流C相充电电流 需改----------------
	data32 /= 100;
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = info.SurplusMinute; //剩余充电时间
	memcpy(pbuf, &data32, 2);
	pbuf += 2;
	data32 = GetSumTime(port);//充电时长
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	data32 = GetSumEnergy(port);//本次充电累计充电电量
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	data32 = GetStartMeterEnergy(port);//充电前电表读数
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	data32 = GetStopMeterEnergy(port);//当前电表读数
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	if (GetStartType(port) == STARTCHARGETYPE_PLATFORM)
		*pbuf++ = 0x01; //充电启动方式 后台启动
	else
		*pbuf++ = 0x00; //充电启动方式 本地刷卡启动
	*pbuf++ = GetStartMode(port); //充电策略
	data32 = GetStartPara(port);//充电策略参数
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	*pbuf++ = 0x00; //预约标志
	memset(pbuf, 0x00, 32); //充电卡号
	GetCardOrDiscernNumber(port, pbuf);
	pbuf += 32;
	*pbuf++ = 0x00; //预约超时时间
	GetStartDateTime(port, datetime);
	*pbuf++ = 0x20;//预约/开始充电开始时间
	*pbuf++ = Hex32ToBcd32(datetime[0]);
	*pbuf++ = Hex32ToBcd32(datetime[1]);
	*pbuf++ = Hex32ToBcd32(datetime[2]);
	*pbuf++ = Hex32ToBcd32(datetime[3]);
	*pbuf++ = Hex32ToBcd32(datetime[4]);
	*pbuf++ = Hex32ToBcd32(datetime[5]);
	*pbuf++ = 0xff;
	data32 = GetStartCardMoney(port); //充电前卡余额
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	memset(pbuf, 0x00, 4); //预留
	pbuf += 4;
	GetVoltage(port, &data1, &data2, &data3);
	GetCurrent(port, &data4, &data5, &data6);
	data32 = (data1 * data4 + data2 * data5 + data3 * data6) / 1000;
	memcpy(pbuf, &data32, 4);//充电功率
	pbuf += 4;
	memset(pbuf, 0x00, 12); //系统变量3,4,5
	pbuf += 12;
	*pbuf++ = addChecksum(buffer + 6, pbuf - buffer - 6);
	
	NetSend(port, buffer, pbuf - buffer);
	print("CMD104 充电桩状态信息包上报P%d:", port);
	printx(buffer, pbuf - buffer);
}

/************************************************************************************************************
** 函 数 名 : SendCMD202
** 功能描述 : 发送充电桩上报充电记录信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... data 从充电桩编码-充电流水号
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SendCMD202(uint8_t port, CHARGINGDATA *ChgDat)
{
	uint8_t buffer[COMMBUFFERLEN], *pbuf,i;
	uint32_t data32;
	
	pbuf = buffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x29;//0x09; //长度域
	*pbuf++ = 0x01;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x41; //序列号域
	*pbuf++ = 202; //命令代码
	*pbuf++ = 0;
	
	memcpy(pbuf, ChgDat->UserId, 2);
	pbuf += 2;
//	*pbuf++ = 0x00; //数据域
//	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	
	ParaGetPileNo(0, pbuf); //充电桩编码
	for (i = 0; i < 32; i++){
		if (pbuf[i] == 0x00){
			pbuf[i] = port + 'A';
			break;
		}
	}
	pbuf += 32;
	*pbuf++ = 1; //充电枪位置类型
	if(ParaGetChgGunNumber() > 1){
		*pbuf++ = port + 1; //充电枪口
	} else {
		*pbuf++ = 0;
	}
	memset(pbuf, 0, 32);
	memcpy(pbuf, ChgDat->CardOrDiscernNumber, 32);//GetCardOrDiscernNumber(port, pbuf);
	pbuf += 32;
	*pbuf++ = 0x20;//充电开始时间
	*pbuf++ = Hex32ToBcd32(ChgDat->StartDateTime[0]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StartDateTime[1]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StartDateTime[2]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StartDateTime[3]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StartDateTime[4]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StartDateTime[5]);
	*pbuf++ = 0xff;
	*pbuf++ = 0x20;//充电结束时间
	*pbuf++ = Hex32ToBcd32(ChgDat->StopDateTime[0]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StopDateTime[1]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StopDateTime[2]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StopDateTime[3]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StopDateTime[4]);
	*pbuf++ = Hex32ToBcd32(ChgDat->StopDateTime[5]);
	*pbuf++ = 0xff;
	data32 = ChgDat->SumTime;
	memcpy(pbuf, &data32, 4);//充电时间长度
	pbuf += 4;
	*pbuf++ = ChgDat->StartSOC;//开始SOC
	*pbuf++ = ChgDat->StopSOC;//结束SOC
	data32 = 0;
//	if (ChgDat->StopCause == CAUSE_OTHER)//其他原因
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_USER_NORMAL_STOP)//计费控制板发送停止充电命令
//		data32 = 200;//用户中止
//	else if (ChgDat->StopCause == CAUSE_WAIT_INSERTGUN_TIMEOUT)//等待插枪超时
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_LOCK_GUN_FAILED)//锁枪失败
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_K1K2_ADHESION_FAULT)//继电器粘连故障
//		data32 = 2007;//接触器异常
//	else if (ChgDat->StopCause == CAUSE_K1K2_CLOSE_FAULT)//继电器拒动故障
//		data32 = 2007;//接触器异常
//	else if (ChgDat->StopCause == CAUSE_K1K2_OUTSIDE_VOL_GREATER_THAN_10V)//绝缘检测前K1K2外侧电压大于10V
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_CHARGEMODULE_OUTPUT_FAULT)//充电模块输出故障
//		data32 = 2008;//模块故障
//	else if (ChgDat->StopCause == CAUSE_INSULATION_DETECT_FAULT)//绝缘检测异常
//		data32 = 304;//绝缘检测异常
//	else if (ChgDat->StopCause == CAUSE_BLEED_UNIT_FAULT)//泄放电路异常
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BMS_PARA_CANNOT_FIT)//车辆参数不适合
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_START_VOLTAGE_FAULT)//启动时电压异常
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BATTERY_READY_TO_NOREADY)//电池准备就绪变为未就绪
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_SUSPEND_TIMEOUT)//充电桩充电中暂停超时
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_CC1_FAULT)//CC1异常
//		data32 = 300;//CC1连接断开
//	else if (ChgDat->StopCause == CAUSE_BMS_COMMUNICATE_TIMEOUT)//BMS通信超时
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_SCRAMSTOP_FAULT)//急停动作故障
//		data32 = 302;//紧急停机
//	else if (ChgDat->StopCause == CAUSE_DOOR_OPEN_FAULT)//门禁故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_GUNPORT_OVERTEMP)//充电接口过温
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_OUPUTVOLTAGE_OVER_FAULT)//直流输出电压过压故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_DEMANDVOL_FAULT)//需求电压异常
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_SINGLEBATTERY_OVERVOL_FAULT)//BSM报文中单体动力蓄电池电压过高
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_SINGLEBATTERY_LESSVOL_FAULT)//BSM报文中单体动力蓄电池电压过低
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_SOC_OVER_FAULT)//BSM报文中SOC过高
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_SOC_LESS_FAULT)//BSM报文中SOC过低
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_SINGLEBATTERY_OVERCUR_FAULT)//BSM报文中单体动力蓄电池电流过流
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_TEMP_OVER_FAULT)//BSM报文中动力蓄电池温度过高
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_INSULATION_FAULT)//BSM报文中动力蓄电池绝缘状态异常
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BSM_OUTPUTCONNECTER_FAULT)//BSM报文中动力蓄电池组输出连接器连接状态异常
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_NORMAL_REACHSOC)//BST报文中达到所需求的SOC目标值
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_NORMAL_REACHTOTALVOL)//BST报文中达到总电压的设定值
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_NORMAL_REACHSINGLEVOL)//BST报文中达到单体电压的设定值
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_INSULATION)//BST报文中绝缘故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_OUTPUTCONNECTER_OVERTEMP)//BST报文中输出连接器过温故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_ELEMENT_OVERTEMP)//BST报文中BMS元件、输出连接器过温
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_OUTPUTCONNECTER)//BST报文中充电连接器故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_BATTERYOVERTEMP)//BST报文中电池组温度过高故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_RELAY)//BST报文中高压继电器故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_CHECKPOINT2)//BST报文中检测点2电压检测故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_FAULT_OTHER)//BST报文中其他故障
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_ERROR_CUR)//BST报文中电流过大
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_BST_ERROR_VOL)//BST报文中电压过大
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_CP_FAULT)//CP异常
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_CC_FAULT)//CC异常
//		data32 = 2000;//系统其他故障
//	else if (ChgDat->StopCause == CAUSE_CHARGE_RESOURCE_APPLY_FAIL)//充电资源申请失败
//		data32 = 2000;//系统其他故障
//	else
//		data32 = 2000;//系统其他故障
	memcpy(pbuf, &data32, 4);//充电结束原因
	pbuf += 4;
	memcpy(pbuf, &ChgDat->SumEnergy, 4); //本次充电电量
	pbuf += 4;
	memcpy(pbuf, &ChgDat->StartMeterEnergy, 4); //充电前电表读数
	pbuf += 4;
	memcpy(pbuf, &ChgDat->StopMeterEnergy, 4); //充电后电表读数
	pbuf += 4;
	memcpy(pbuf, &ChgDat->SumMoney, 4); //本次充电金额
	pbuf += 4;
	memset(pbuf, 0, 4); //预留
	pbuf += 4;
	memcpy(pbuf, &ChgDat->StartCardMoney, 4); //充电前卡余额
	pbuf += 4;
	memcpy(pbuf, &CommInfo[port].ChargeRecordIndexBak, 4);//当前充电记录索引
	pbuf += 4;
	memcpy(pbuf, &CommInfo[port].ChargeRecordNumberBak, 4);//总充电记录条目
	pbuf += 4;
	*pbuf++ = 0; //预留
	*pbuf++ = ChgDat->Mode; //充电策略
	memcpy(pbuf, &ChgDat->Para, 4); //充电策略参数
	pbuf += 4;
	memcpy(pbuf, ChgDat->VIN, 17); //车辆VIN
	pbuf += 17;
	memset(pbuf, 0, 8); //车牌号
	pbuf += 8;
	memcpy(pbuf, ChgDat->PeriodEnergy, 48 * 2); //时段电量
	pbuf += 48 * 2;
	if (ChgDat->StartType == STARTCHARGETYPE_PLATFORM)
		*pbuf++ = 0x01; //充电启动方式 后台启动
	else
		*pbuf++ = 0x00; //充电启动方式 本地刷卡启动	
	memcpy(pbuf, ChgDat->TradeSerialNumber, 32); //交易流水号
	pbuf += 32;
	
	*pbuf++ = addChecksum(buffer + 6, pbuf - buffer - 6);
	
	NetSend(port, buffer, pbuf - buffer);
	print("CMD202 充电桩上报充电记录信息P%d:", port);
	printx(buffer, pbuf - buffer);
}

/************************************************************************************************************
** 函 数 名 : SendCMD204
** 功能描述 : 发送充电桩充电上传用户帐户查询报文
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SendCMD204(uint8_t port)
{
	uint8_t buffer[COMMBUFFERLEN], *pbuf,i,strlen;
	uint32_t data32;
	
	pbuf = buffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0xA2; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x41; //序列号域
	*pbuf++ = 204; //命令代码
	*pbuf++ = 0;
	*pbuf++ = 0x00; //数据域
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	ParaGetPileNo(0, pbuf); //充电桩编码
	for (i = 0; i < 32; i++){
		if (pbuf[i] == 0x00){
			pbuf[i] = port + 'A';
			break;
		}
	}
	pbuf += 32;
	memset(pbuf, 0x00, 32); //充电卡号
	strlen = sprintf((char *)pbuf, "%u", ReportCMD204[port].ReportOnlineCardID);
	if (strlen < 10){
		memset(pbuf, '0', 10 - strlen);
		sprintf((char *)pbuf + (10 - strlen), "%d", ReportCMD204[port].ReportOnlineCardID);
	}
	pbuf += 32;
	data32 = 0; //充电卡余额
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	*pbuf++ = 0x00; //充电卡黑名单标志
	memset(pbuf, 0x00, 32); //用户充电卡密码
	pbuf += 32;
	memset(pbuf, 0x00, 48); //卡扇区数据随机数
	*pbuf = 0x74;
	*(pbuf+1) = 0x65;
	*(pbuf+2) = 0x73;
	*(pbuf+3) = 0x74;
	pbuf += 48;
	*pbuf++ = addChecksum(buffer + 6, pbuf - buffer - 6);
	
	NetSend(port, buffer, pbuf - buffer);
	print("CMD204 充电桩充电上传用户帐户查询报文P%d:", port);
	printx(buffer, pbuf - buffer);
}

/************************************************************************************************************
** 函 数 名 : SendCMD106
** 功能描述 : 发送充电桩签到信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void SendCMD106(uint8_t port)
{
	uint8_t buffer[COMMBUFFERLEN], *pbuf, i;
	Rtc datetime;
	uint32_t data32;
	
	pbuf = buffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x69; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x41; //序列号域
	*pbuf++ = 106; //命令代码
	*pbuf++ = 0;
	*pbuf++ = 0x00; //数据域
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	ParaGetPileNo(0, pbuf); //充电桩编码
	for (i = 0; i < 32; i++){
		if (pbuf[i] == 0x00){
			pbuf[i] = port + 'A';
			break;
		}
	}
	pbuf += 32;
	*pbuf++ = 0x00; //标志
	*pbuf++ = _VERSION_MAJOR; //充电桩软件版本
	*pbuf++ = _VERSION_MINOR;
	*pbuf++ = _VERSION_PATCH;
	*pbuf++ = 0x00;
	memset(pbuf ,0x00, 6); //充电桩项目类型 启动次数
	pbuf += 6;
	*pbuf++ = 0x02; //数据上传模式
	*pbuf++ = ParaGetLoginTimeInterval(port) & 0xff; //签到间隔时间
	*pbuf++ = (uint16_t)ParaGetLoginTimeInterval(port) >> 8;
	*pbuf++ = 0x00; //运行内部变量
	*pbuf++ = ParaGetChgGunNumber(); //充电枪个数
	*pbuf++ = ParaGetNetHeartTime(port); //心跳上报周期
	*pbuf++ = ParaGetNetCommOverCnt(port); //心跳包检测超时次数
	data32 = GetChargeRecordNumber(port); //充电记录数量
	memcpy(pbuf, &data32, 4);
	pbuf += 4;
	RtcRead(&datetime);
	*pbuf++ = Hex32ToBcd32(datetime.year / 100) & 0xff; //当前充电桩系统时间
	*pbuf++ = Hex32ToBcd32(datetime.year % 100) & 0xff;
	*pbuf++ = Hex32ToBcd32(datetime.month) & 0xff;
	*pbuf++ = Hex32ToBcd32(datetime.day) & 0xff;
	*pbuf++ = Hex32ToBcd32(datetime.hour) & 0xff;
	*pbuf++ = Hex32ToBcd32(datetime.min) & 0xff;
	*pbuf++ = Hex32ToBcd32(datetime.sec) & 0xff;
	*pbuf++ = 0xFF;
	memset(pbuf ,0x00, 28);
	pbuf += 28;
	*pbuf++ = 26; //桩后台通信协议版本号 本协议是V2.6
	*pbuf++ = 0;
	*pbuf++ = addChecksum(buffer + 6, pbuf - buffer - 6);
	
	NetSend(port, buffer, pbuf - buffer);
	print("CMD106 发送充电桩签到信息P%d:", port);
	printx(buffer, pbuf - buffer);
}

/************************************************************************************************************
** 函 数 名 : RecvCMD203
** 功能描述 : 接收服务器应答帐户查询信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD203(uint8_t port, uint8_t *buffer)
{
	ReportCMD204[port].ReportRet = buffer[12];
	ReportCMD204[port].OnlineCardIDBalance = buffer[16] | (buffer[17] << 8) | (buffer[18] << 16) | (buffer[19] << 24);
	ReportCMD204[port].ReportFlag = 2;
}

/************************************************************************************************************
** 函 数 名 : RecvCMD1011
** 功能描述 : 接收服务器下发重启指令
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD1011(uint8_t port, uint8_t *buffer)
{
	uint8_t ackbuffer[COMMBUFFERLEN], *pbuf;

	pbuf = ackbuffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x0D; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x00; //序列号域
	*pbuf++ = 0xF4; //命令代码
	*pbuf++ = 0x03;
	memset(pbuf, 0, 4);//数据域
	pbuf += 4;
	*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
	
	NetSend(port, ackbuffer, pbuf - ackbuffer);
	print("CMD1012 充电桩应答服务器下发重启命令P%d:", port);
	printx(ackbuffer, pbuf - ackbuffer);
	
	print("准备重启设备..................\r\n\n\n");
	OSTimeDlyHMSM (0, 0, 3, 0);
	//是否需要保存数据再重启
	NVIC_SystemReset();
}

/************************************************************************************************************
** 函 数 名 : RecvCMD1101
** 功能描述 : 接收后台服务器查询24时电费计价策略信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD1101(uint8_t port, uint8_t *buffer)
{
	uint8_t ackbuffer[COMMBUFFERLEN], *pbuf, *p;
	uint32_t i,data32;
	 
	p = &ackbuffer[8];
	for(i = 0; i < 12; i++){
		ParaGetFeilvTime(port, i, &p[0], &p[1], &p[2], &p[3]);
		data32 = ParaGetFeilvMoney(port, i) / 1000;
		memcpy(&p[4], &data32, 4);
		p += 8;
	}

	pbuf = ackbuffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x69; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x00; //序列号域
	*pbuf++ = 0x4E; //命令代码
	*pbuf++ = 0x04;
	pbuf += 96;//数据域
	*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
	
	NetSend(port, ackbuffer, pbuf - ackbuffer);
	print("CMD1102 充电桩应答后台服务器查询24小计价策略信息P%d:", port);
	printx(ackbuffer, pbuf - ackbuffer);
}

/************************************************************************************************************
** 函 数 名 : RecvCMD1103
** 功能描述 : 接收后台服务器设置24时电费计价策略信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void RecvCMD1103(uint8_t port, uint8_t *buffer)
{
	uint8_t ackbuffer[COMMBUFFERLEN], *pbuf,ret = 1,*p;
	uint32_t i,data32;
	 
	p = &buffer[8];
	for(i = 0; i < 12; i++){
		if((p[0] > 24) || (p[2] > 24)){
			break;
		}
		if(((p[1] != 0) && (p[1] != 30)) || ((p[3] != 0) && (p[3] != 30))){
			break;
		}
		if(((p[0] == p[2]) && (p[0] != 0)) && ((p[1] == p[3]) && (p[1] != 0))){
			break;
		}
		p += 8;
	}
	if(i == 12){
		ret = 0;
		p = &buffer[8];
		for(i = 0; i < 12; i++){
			ParaSetFeilvTime(port, i, p[0], p[1], p[2], p[3]);
			data32 = p[4] | (p[5] << 8) | (p[6] << 16) | (p[7] << 24);
			ParaSetFeilvMoney(port, i, data32 * 1000);
			p += 8;
		}
	}

	pbuf = ackbuffer;
	*pbuf++ = STARTCODE1; //起始域
	*pbuf++ = STARTCODE2;
	*pbuf++ = 0x0A; //长度域
	*pbuf++ = 0x00;
	*pbuf++ = MSGCODE; //信息域
	*pbuf++ = 0x00; //序列号域
	*pbuf++ = 0x50; //命令代码
	*pbuf++ = 0x04;
	*pbuf++ = ret; //数据域
	*pbuf++ = addChecksum(ackbuffer + 6, pbuf - ackbuffer - 6);
	
	NetSend(port, ackbuffer, pbuf - ackbuffer);
	print("CMD1104 充电桩应答后台服务器设置24小计价策略信息P%d:", port);
	printx(ackbuffer, pbuf - ackbuffer);
}

/************************************************************************************************************
** 函 数 名 : FrameFormatCheck
** 功能描述 : 帧格式检测函数
** 输    入 : buffer 缓冲区 length 缓冲区长度
** 输    出 : 无
** 返    回 :	检测成功返回1 否则返回0
*************************************************************************************************************
*/
static uint8_t FrameFormatCheck(const uint8_t* buffer)
{
	uint16_t len;
	uint8_t checksum;
	
	if(buffer[0] != STARTCODE1){
		return 0;
	}
	if(buffer[1] != STARTCODE2){
		return 0;
	}
	len = buffer[2] | (buffer[3] << 8);
	if(len < 9){
		return 0;
	}
	checksum = addChecksum(buffer + 6, len - 6 - 1);
	if(checksum != buffer[len - 1]){
		return 0;
	}
	return 1;
}

/************************************************************************************************************
** 函 数 名 : UnpackMsg
** 功能描述 : 解析数据包
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... buffer 数据包
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void UnpackMsg(uint8_t port,uint8_t *buffer)
{
	uint16_t cmdcode;
	
	cmdcode = buffer[6] + (buffer[7] << 8);
	switch(cmdcode){
		case 1: //后台服务器下发充电桩整型工作参数
			print("CMD1 后台服务器下发充电桩整型工作参数P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD1(port, buffer);
			break;
		case 3: //后台服务器下发充电桩字符型工作参数
			print("CMD3 后台服务器下发充电桩字符型工作参数P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD3(port, buffer);
			break;
		case 5: //后台服务器下发充电桩控制命令
			print("CMD5 后台服务器下发充电桩控制命令P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD5(port, buffer);
			break;
		case 7: //后台服务器下发充电桩开启充电控制命令
			print("CMD7 后台服务器下发充电桩开启充电控制命令P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD7(port, buffer);
			break;
		case 101: //服务器应答心跳包信息
			print("CMD101 服务器应答心跳包信息P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			CommInfo[port].heartbeatackflag = 1;
			break;
		case 103: //服务器应答充电桩状态信息包
			print("CMD103 服务器应答充电桩状态信息包P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			break;
		case 105: //服务器应答充电桩签到命令
			print("CMD105 服务器应答充电桩签到命令P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			CommInfo[port].loginackflag = 1;
			break;
		case 201: //服务器应答充电桩上报充电信息报文
			print("CMD201 服务器应答充电桩上报充电信息报文P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			CommInfo[port].AckCMD201Flag = 1;
			break;
		case 203: //服务器应答帐户查询信息
			print("CMD203 服务器应答帐户查询信息P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD203(port, buffer);
			break;
		case 1011: //服务器下发重启指令
			print("CMD1011 服务器下发重启指令P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD1011(port, buffer);
			break;
		case 1101: //后台服务器查询24时电费计价策略信息
			print("CMD1101 后台服务器查询24时电费计价策略信息P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD1101(port, buffer);
			break;
		case 1103: //后台服务器设置24时电费计价策略信息
			print("CMD1103 后台服务器设置24时电费计价策略信息P%d:", port);
			printx(buffer, buffer[2] + (buffer[3] << 8));
			RecvCMD1103(port, buffer);
			break;
		default:
			break;
	}
}

/************************************************************************************************************
** 函 数 名 : AnalyseMsgFromMSA
** 功能描述 : 解析来自主站的消息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void AnalyseMsgFromMSA(uint8_t port)
{
	uint32_t i,j,len;
	uint16_t data16;
	
	do{
		len = NetRecv(port, &CommInfo[port].buffer[CommInfo[port].len], COMMBUFFERLEN - CommInfo[port].len);
		CommInfo[port].len += len;
		if(CommInfo[port].len >= COMMBUFFERLEN){
			CommInfo[port].len = 0;
			break;
		}
	}while(len);
	
	i = 0;
	do{
		if(FrameFormatCheck(&CommInfo[port].buffer[i]) == 1){
			UnpackMsg(port, &CommInfo[port].buffer[i]); //解析数据
			data16 = CommInfo[port].buffer[i + 2] + (CommInfo[port].buffer[i + 3] << 8);
			for(j = 0; j < CommInfo[port].len; j++){
				CommInfo[port].buffer[j] = CommInfo[port].buffer[i + j + data16];
			}
			CommInfo[port].len -= data16;
			i = 0;
		} else {
			i++;
		}
	}while(i < CommInfo[port].len);
}

/************************************************************************************************************
** 函 数 名 : AccountQueryReport
** 功能描述 : 充电桩充电上传用户帐户查询
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void AccountQueryReport(uint8_t port)
{
	static uint8_t test1;
	
	if(test1 == 1){
		test1 = 0;
		SendCMD204(port);
	}
}

///************************************************************************************************************
//** 函 数 名 : ReportChargeRecord
//** 功能描述 : 上报交易记录
//** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
//** 输    出 : 无
//** 返    回 :	无
//*************************************************************************************************************
//*/
//static void ReportChargeRecord(uint8_t port)
//{
//	CHARGINGDATA ChgDat;
//	uint16_t index;
//	
//	if (CommInfo[port].ChargeRecordNumberBak != GetChargeRecordNumber(port)){//新交易记录
//		CommInfo[port].ChargeRecordNumberBak = GetChargeRecordNumber(port);
//		index = GetChargeRecordIndex(port);
//		CommInfo[port].ChargeRecordIndexBak = index;
//		if (FindChargeRecord(index,  (uint8_t *)&ChgDat, sizeof(CHARGINGDATA)) == 1){//查询成功
//			SendCMD202(port, &ChgDat);
//		}
//	} else {
//		
//	}
//}

/************************************************************************************************************
** 函 数 名 : ReportChargeRecord
** 功能描述 : 上报交易记录
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void ReportChargeRecord(uint8_t port)
{
	static uint32_t Timer[2];
	CHARGINGDATA ChgDat;
	uint16_t index,i;
	
	if (CommInfo[port].AckCMD201Flag == 1){//后台应答充电记录
		CommInfo[port].AckCMD201Flag = 0;
		for (i = 0; i < 50 - 1; i++)
			CommInfo[port].ReportCtl[i] = CommInfo[port].ReportCtl[i + 1];
		CommInfo[port].NeedReportNum--;
		Timer[port] = 0;
	}
	if ((TimerRead() - Timer[port] > T1S * 10) || (CommInfo[port].ChargeRecordNumberBak != GetChargeRecordNumber(port))){
		if (CommInfo[port].ChargeRecordNumberBak != GetChargeRecordNumber(port)){//新交易记录
			CommInfo[port].ChargeRecordNumberBak = GetChargeRecordNumber(port);
			index = GetChargeRecordIndex(port);
			CommInfo[port].ChargeRecordIndexBak = index;
			if (FindChargeRecord(index,  (uint8_t *)&ChgDat, sizeof(CHARGINGDATA)) == 1){//查询成功
				if (ChgDat.ChgPort == port){
					for (i = 0; i < 50 - 1; i++)
						CommInfo[port].ReportCtl[50 - 1 - i] = CommInfo[port].ReportCtl[50 - 2 - i];
					CommInfo[port].ReportCtl[0].index = index;
					CommInfo[port].NeedReportNum++;
				}
			}
		}
		if (CommInfo[port].NeedReportNum > 0){
			FindChargeRecord(CommInfo[port].ReportCtl[0].index,  (uint8_t *)&ChgDat, sizeof(CHARGINGDATA));
			if (ChgDat.ChgPort == port){
				SendCMD202(port, &ChgDat);
			}
		}
//		if (ChgDat.ChgPort == port)
//			Timer[port] = TimerRead();
	}
}

/************************************************************************************************************
** 函 数 名 : ProtocolHandle
** 功能描述 : 通信协议处理函数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void ProtocolHandle(uint8_t port)
{
	static uint8_t state[2];
	static uint8_t logincount[2],heartbeatcount[2];
	static uint32_t timer[2],reportstatetimer[2],report204timer[2];
	static uint32_t reportstartchargeresulttimer[2];
	static uint8_t chgstate[2],mainstatebak[2];
	uint8_t sip[4];
	uint16_t sport,peroid;
	
	switch(state[port]){
		case 0: //连接服务器
			CommInfo[port].loginackflag = 0;
			ParaGetServerIp(port, sip);
			sport = ParaGetServerPort(port);
			if (NetConn(port, sip, sport) != 1)
				break;
			memset(CommInfo[port].buffer, 0, COMMBUFFERLEN);
			CommInfo[port].len = 0;
			logincount[port] = 0;
			state[port]++;
			
		case 1: //向服务器签到
			logincount[port]++;
			if (logincount[port] > 3){
				state[port]--;
				break;
			}
			SendCMD106(port);
			timer[port] = TimerRead();
			state[port]++;
		
		case 2: //等待服务器应答签到
			AnalyseMsgFromMSA(port);
			if(TimerRead() - timer[port] > T1S * 5){
				state[port]--;
				break;
			}
			if(CommInfo[port].loginackflag != 1)
				break;
			CommInfo[port].heartbeatackflag = 0;
			timer[port] = TimerRead();
			heartbeatcount[port] = 0;
			state[port]++;
		
		case 3: //定时心跳
			AnalyseMsgFromMSA(port);
			if ((peroid = ParaGetNetHeartTime(port)) < 3)
				peroid = 3;
			if (TimerRead() - timer[port] > T1S * peroid){
				timer[port] = TimerRead();
				if (CommInfo[port].heartbeatackflag == 1){
					heartbeatcount[port] = 0;
				}
				if (heartbeatcount[port]++ > ParaGetNetCommOverCnt(port)){
					state[port] = 0;
					break;
				} else {
					CommInfo[port].heartbeatackflag = 0;
					SendCMD102(port);
				}
			}
			
			//定时上报状态信息
			AnalyseMsgFromMSA(port);
			if ((peroid = ParaGetStateInfoReportPeriod(port)) < 3)
				peroid = 3;
			if(TimerRead() - reportstatetimer[port] > T1S * peroid){
				reportstatetimer[port] = TimerRead();
				SendCMD104(port);
			}
			//还应变化上报
			if (GetMainCharge(port) != mainstatebak[port]){
				mainstatebak[port] = GetMainCharge(port);
				SendCMD104(port);
			}
			
			//定时上报未应答的交易记录
			ReportChargeRecord(port);
			
			//上报在线卡查询信息
			if (ReportCMD204[port].ReportFlag == 1){
				if (TimerRead() - report204timer[port] > T1S * 3){
					report204timer[port] = TimerRead();
					SendCMD204(port);
				}
			}
		
			//其他数据交互
			AnalyseMsgFromMSA(port);
			AccountQueryReport(port);
			
			break;
		
		default:
			state[port] = 0;
			break;
	}
	
	
	//充电控制
	switch(chgstate[port]){
		case 0://待机
			if(CommInfo[port].startflag == 1){
				CommInfo[port].startflag = 0;
				if(CommInfo[port].chargestrategy == 1)//按时间
					SetMainCharge(port, SET_START_CHARGE, 2, CommInfo[port].chargepara, CommInfo[port].ChargeCardNumber, CommInfo[port].UserId);
				else if(CommInfo[port].chargestrategy == 2)//按金额
					SetMainCharge(port, SET_START_CHARGE, 3, CommInfo[port].chargepara, CommInfo[port].ChargeCardNumber, CommInfo[port].UserId);
				else if(CommInfo[port].chargestrategy == 3)//按电量
					SetMainCharge(port, SET_START_CHARGE, 1, CommInfo[port].chargepara, CommInfo[port].ChargeCardNumber, CommInfo[port].UserId);
				else
					SetMainCharge(port, SET_START_CHARGE, 0, CommInfo[port].chargepara, CommInfo[port].ChargeCardNumber, CommInfo[port].UserId);
				reportstartchargeresulttimer[port] = TimerRead();
				chgstate[port]++;
				CommInfo[port].stopflag = 0;
			}
			break;
			
		case 1://等待启动结果
			if (GetMainCharge(port) == STATE_CHARGE){//充电中
				SendCMD8(port, 0);
				chgstate[port]++;
			}
			if (GetMainCharge(port) == STATE_COMPLETE){//充电完成
				SetMainCharge(port, SET_STOP_CHARGE, 0 , 0, 0, 0);
				SendCMD8(port, 100001);
				chgstate[port] = 0;
				CommInfo[port].startflag = 0;
			}
			if (TimerRead() - reportstartchargeresulttimer[port] > T1S * 120){//超时
				SetMainCharge(port, SET_STOP_CHARGE, 0 , 0, 0, 0);
				SendCMD8(port, 100002);
				chgstate[port] = 0;
				CommInfo[port].startflag = 0;
			}
			break;
			
		case 2://充电中
			if (GetMainCharge(port) != STATE_CHARGE){//非充电中
				print("test1----\r\n");
				ReportChargeRecord(port);
				chgstate[port] = 0;
				CommInfo[port].startflag = 0;
			}
			if(CommInfo[port].stopflag == 1){//后台下发停止充电
				CommInfo[port].stopflag = 0;
				SetMainCharge(port, SET_STOP_CHARGE, 0 , 0, 0, 0);
//				ReportChargeRecord(port);
//				chgstate[port] = 0;
				CommInfo[port].startflag = 0;
			}
			break;
	}
}

/************************************************************************************************************
** 函 数 名 : GetOnlineState
** 功能描述 : 获取在线状态
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
uint8_t GetOnlineState(uint8_t port)
{
	if (CommInfo[port].loginackflag == 1){
		return 1;
	}
	return 0;
}

/************************************************************************************************************
** 函 数 名 : VerifyOnlineCardInfo
** 功能描述 : 核实在线卡信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......  CardID 卡号 
** 输    出 : Balance余额
** 返    回 :	0有效帐户 1非法帐户 2余额不足 3把黑卡恢复成正常卡 4把正常卡变黑卡 5非法卡号 6挂失 7销卡 8密码错误 9该卡已经在别的桩充电 
*************************************************************************************************************
*/
int8_t VerifyOnlineCardInfo(uint8_t port, uint32_t CardID, uint32_t *Balance)
{
	uint32_t Timer;
	
	*Balance = 0;
	ReportCMD204[port].ReportOnlineCardID = CardID;
	ReportCMD204[port].OnlineCardIDBalance = 0;
	ReportCMD204[port].ReportRet = 0;
	ReportCMD204[port].ReportFlag = 1;
	Timer = TimerRead();
	while(TimerRead() - Timer < T1S * 10){
		OSTimeDlyHMSM (0, 0, 0, 100);
		if (ReportCMD204[port].ReportFlag == 2){
			ReportCMD204[port].ReportFlag = 0;
			*Balance = ReportCMD204[port].OnlineCardIDBalance;
			return ReportCMD204[port].ReportRet;
		}
	}
	ReportCMD204[port].ReportOnlineCardID = 0;
	ReportCMD204[port].OnlineCardIDBalance = 0;
	ReportCMD204[port].ReportRet = 0;
	ReportCMD204[port].ReportFlag = 0;
	return -1;
}

/************************************************************************************************************
** 函 数 名 : ProtocolInit
** 功能描述 : 初始化函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void ProtocolInit(void)
{
	uint8_t port, PortNum = ParaGetChgGunNumber();
	uint16_t index,num,i;
	CHARGINGDATA ChgDat;
	
	memset(&CommInfo, 0, sizeof(COMMINFO));
	for(port = 0; port < PortNum; port++){
		index = GetChargeRecordIndex(port);
		CommInfo[port].ChargeRecordIndexBak = index;
		num = GetChargeRecordNumber(port) % 50;
		CommInfo[port].ChargeRecordNumberBak = num;
		for (i = 0; i < num; i++){
			if (FindChargeRecord(index,  (uint8_t *)&ChgDat, sizeof(CHARGINGDATA)) == 1)//查询成功
				CommInfo[port].ReportCtl[CommInfo[port].NeedReportNum++].index = index;
			if (index == 0)
				index = 1000;
			else
				index --;
			OSTimeDlyHMSM (0, 0, 0, 1);
		}
	}
}

/************************************************************************************************************
** 函 数 名 : TaskProtocolHandle
** 功能描述 : 通信协议处理任务
** 输    入 : pdata 未使用
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void TaskProtocolHandle(void *pdata)
{
	uint8_t port;
	uint8_t PortNum = ParaGetChgGunNumber();
	
	ProtocolInit();
	
	while(1){
		for(port = 0; port < PortNum; port++){
			ProtocolHandle(port);
			OSTimeDlyHMSM (0, 0, 0, 10);
		}
	}
}

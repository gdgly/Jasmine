#include <stdint.h>
#include "Board.h"
#include "UART.h"
#include "MT625.h"
#include "MyAlgLib.h"
#include "string.h"
#include "RTC.h"
#include "encrypt.h"

const uint8_t RegisterInfoPassWd[6] = {0x71, 0x3D, 0x0B, 0xF2, 0x3E, 0x8E};

/************************************************************************************************************
** 函 数 名 : MTMakeMsg
** 功能描述 : 组包
** 输    入 : 类型，命令，数据，长度
** 输    出 : 无
** 返    回 :	0 FAIL  1 OK
*************************************************************************************************************
*/
static uint16_t MTMakeMsg(uint8_t type,uint8_t cmd,uint8_t *data,uint16_t datalen,uint8_t *outdata)
{
	uint16_t outlen=0;

	outdata[outlen++] = MT_STX;     // BEGIN(1)
	
	outdata[outlen++] = (datalen+2)/256;   // LEN(2)
	outdata[outlen++] = (datalen+2)%256;
	
	outdata[outlen++] = type;  //类别(1)
	
	outdata[outlen++] = cmd;   //命令 (1)

	if ((data!=NULL) && (datalen!=0))   // 数据 (n)
	{
		memcpy(outdata+outlen,data,datalen);
		outlen += datalen;
	}	

	outdata[outlen++] = MT_ETX;   // END(1)
 
	outdata[outlen++]=MyXorSum(outdata,outlen);  // check(1)

	return outlen;
}

/************************************************************************************************************
** 函 数 名 : MTCheckMsg
** 功能描述 : 校验数据
** 输    入 : 数据，长度
** 输    出 : 无
** 返    回 :	0 FAIL  1 OK
*************************************************************************************************************
*/
static uint8_t MTCheckMsg(uint8_t *data,uint16_t datalen,MTRcvMsg *info)
{
	uint16_t lenth=0;
	uint8_t  xorsum=0;
	
	if ((data==NULL) || (datalen<8) || (datalen>128) || (info==NULL))//(datalen<9) 
	{
		return 0;
	}

	if ((data[0]!=MT_STX) || (data[datalen-2]!=MT_ETX))
	{
		return 0;
	}

	lenth=data[1]*256+data[2];
	if (lenth != datalen-5)
	{
		return 0;
	}

	xorsum=MyXorSum(data,datalen-1);
	if (xorsum != data[datalen-1])
	{
		return 0;
	}

	info->type = data[3];
	info->cmd = data[4];
	//info->ack = data[5];
	info->buflen=lenth-2;//info->buflen=lenth-3;
	if (info->buflen != 0)
	{
		memcpy(info->buf,&(data[5]),info->buflen);//memcpy(info->buf,&(data[6]),info->buflen);
	}


	return 1;
	
	
}

/************************************************************************************************************
** 函 数 名 : MTSenddRv
** 功能描述 : 需发送的数据和长度
** 输    入 : 接受到的数据
** 输    出 : 无
** 返    回 :	0 FAIL  1 OK
*************************************************************************************************************
*/
static uint8_t MTSenddRv(uint8_t *data,uint8_t datalen,MTRcvMsg *info)
{
	uint16_t i=0;
	uint8_t  tmpbuf[100]={0};
	uint16_t tmplen=0;
	uint8_t rcvbuf[128]={0};
	uint16_t rcvlen=0;
	
	UartFlush(MTUARTNO);          			// 清空串口缓存
	UartWrite(MTUARTNO,data,datalen);		// 发送数据
	
	for (i=0;i<2000;i++)				    // 接收数据
	{
		DelayMs(1);
		
		tmplen=UartRead(MTUARTNO,tmpbuf,100);
		if (tmplen != 0)
		{
			if (rcvlen+tmplen >= sizeof(rcvbuf))
			{
				return 0;
			}
			memcpy(rcvbuf+rcvlen,tmpbuf,tmplen);
			rcvlen += tmplen;
		
			if (MTCheckMsg(rcvbuf,rcvlen,info) == 1)
			{
				break;
			}

		}
	}

	if (i == 2000)
	{
		return 0;
	}
	else
	{
		return 1;
	}
	
}

/************************************************************************************************************
** 函 数 名 : MTInit
** 功能描述 : 卡初始化
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void MTInit(void)
{
	UartInit(MTUARTNO, 115200);
}

/************************************************************************************************************
** 函 数 名 : MTSearch
** 功能描述 : 寻卡函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	成功返回1 失败返回0
*************************************************************************************************************
*/
uint8_t MTSearch(uint32_t *UID)
{
	uint8_t  senddata[128];
	uint16_t sendlen=0;
	MTRcvMsg rcvinfo;
	
	sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_FIND, NULL, 0, senddata);//寻卡
	if (MTSenddRv(senddata, sendlen, &rcvinfo) == 1){
		if ((rcvinfo.type == MT_CMD_TYPE2) && (rcvinfo.cmd == MT_CMD_M1_FIND)){
			if(rcvinfo.buf[0] == MT_ACK_OK){
				return 1;
			}
		}
	}
	return 0;
}

/************************************************************************************************************
** 函 数 名 : MTRead
** 功能描述 : 读卡函数
** 输    入 : vifdata 卡数据
** 输    出 : 无
** 返    回 :	成功返回1 失败返回0
*************************************************************************************************************
*/
uint8_t MTRead(M1CARDVIF *vifdata)
{
	uint8_t  senddata[128],crc8;
	uint16_t sendlen=0;
	MTRcvMsg rcvinfo;
	uint8_t  tmpbuf[16];
	//uint8_t DataPassWd[6] = {0xfb, 0x8f, 0xa0, 0xef, 0x67, 0x36};
	
	sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_FIND, NULL, 0, senddata);//寻卡
	if (MTSenddRv(senddata, sendlen, &rcvinfo) == 1){
		if ((rcvinfo.type == MT_CMD_TYPE2) && (rcvinfo.cmd == MT_CMD_M1_FIND) && (rcvinfo.buf[0] == MT_ACK_OK)){
			sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_READID, NULL, 0, senddata);//获取卡号
			if (MTSenddRv(senddata, sendlen, &rcvinfo) == 1){
				if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_READID &&(rcvinfo.buf[0] == MT_ACK_OK)) ){
					vifdata->cardID  = (uint32_t)(rcvinfo.buf[4]<<24);
					vifdata->cardID |= (uint32_t)(rcvinfo.buf[3]<<16);
					vifdata->cardID |= (uint32_t)(rcvinfo.buf[2]<<8);
					vifdata->cardID |= (uint32_t)(rcvinfo.buf[1]);
					
					tmpbuf[0] = VIF_REG_SECTOR;
					memcpy(&tmpbuf[1], RegisterInfoPassWd, 6);
					sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_VIFKEYA, tmpbuf, 7, senddata);//验证KEY A 验证A密码 注册信息
					if ((MTSenddRv(senddata,sendlen,&rcvinfo) == 1)){
						if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_VIFKEYA) && (rcvinfo.buf[0] == VIF_REG_SECTOR) && (rcvinfo.buf[1] ==MT_ACK_OK)){
							tmpbuf[0] = VIF_REG_SECTOR;
							tmpbuf[1] = VIF_REG_BLOCK;
							sendlen=MTMakeMsg(MT_CMD_TYPE2,MT_CMD_M1_BLOCKRD,tmpbuf,2,senddata);//读扇区块内容 注册信息
							if (MTSenddRv(senddata,sendlen,&rcvinfo) == 1){
								if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_BLOCKRD) && (rcvinfo.buf[0] == tmpbuf[0]) && (rcvinfo.buf[1] == tmpbuf[1]) && (rcvinfo.buf[2] ==MT_ACK_OK)){
									Decrypt(&rcvinfo.buf[3], 0, tmpbuf, 16);//AES解密
									crc8 = MyCrc8(tmpbuf,15);//校验CRC8
									if(crc8 == tmpbuf[15]){
										vifdata->type =  tmpbuf[8];
										
										if (vifdata->type == 2){
											return 1;
										}
										
										tmpbuf[0] = VIF_DATA_SECTOR;
										//memcpy(&tmpbuf[1], DataPassWd, 6);
										memcpy(&tmpbuf[1], vifdata->KeyA, 6);
										sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_VIFKEYA, tmpbuf, 7, senddata);//验证KEY A 验证A密码 数据信息
										if ((MTSenddRv(senddata,sendlen,&rcvinfo) == 1)){
											if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_VIFKEYA) && (rcvinfo.buf[0] == VIF_DATA_SECTOR) && (rcvinfo.buf[1] ==MT_ACK_OK)){
												tmpbuf[0] = VIF_DATA_SECTOR;
												tmpbuf[1] = VIF_DATA_BLOCK;
												sendlen=MTMakeMsg(MT_CMD_TYPE2,MT_CMD_M1_BLOCKRD,tmpbuf,2,senddata);//读扇区块内容 注册信息
												if (MTSenddRv(senddata,sendlen,&rcvinfo) == 1){
													if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_BLOCKRD) && (rcvinfo.buf[0] == tmpbuf[0]) && (rcvinfo.buf[1] == tmpbuf[1]) && (rcvinfo.buf[2] ==MT_ACK_OK)){
														Decrypt(&rcvinfo.buf[3], 0, tmpbuf, 16);//AES解密
														crc8 = MyCrc8(tmpbuf,15);//校验CRC8
														if(crc8 == tmpbuf[15]){
															vifdata->lockst = tmpbuf[0];
															vifdata->Money = (tmpbuf[3] << 16) | (tmpbuf[2] << 8) | tmpbuf[1];
															vifdata->recdInx = tmpbuf[4];
															vifdata->rechargeInx = tmpbuf[5];
															memcpy(vifdata->PileNumber, &tmpbuf[6], 4);
															
															return 1;
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/************************************************************************************************************
** 函 数 名 : MTWrite
** 功能描述 : 写卡函数
** 输    入 : vifdata 卡数据
** 输    出 : 无
** 返    回 :	成功返回1 失败返回0
*************************************************************************************************************
*/
uint8_t MTWrite(M1CARDVIF *vifdata)
{
	uint8_t  senddata[128];
	uint16_t sendlen=0;
	uint32_t data32;
	MTRcvMsg rcvinfo;
	uint8_t  tmpbuf[16],i,tmpbuf2[16];
	//uint8_t DataPassWd[6] = {0xfb, 0x8f, 0xa0, 0xef, 0x67, 0x36};
	
	sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_FIND, NULL, 0, senddata);//寻卡
	if (MTSenddRv(senddata, sendlen, &rcvinfo) == 1){
		if ((rcvinfo.type == MT_CMD_TYPE2) && (rcvinfo.cmd == MT_CMD_M1_FIND) && (rcvinfo.buf[0] == MT_ACK_OK)){
			sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_READID, NULL, 0, senddata);//获取卡号
			if (MTSenddRv(senddata, sendlen, &rcvinfo) == 1){
				if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_READID &&(rcvinfo.buf[0] == MT_ACK_OK)) ){
					data32  = (uint32_t)(rcvinfo.buf[4]<<24);
					data32 |= (uint32_t)(rcvinfo.buf[3]<<16);
					data32 |= (uint32_t)(rcvinfo.buf[2]<<8);
					data32 |= (uint32_t)(rcvinfo.buf[1]);
					if (data32 == vifdata->cardID){
						
						tmpbuf[0] = VIF_DATA_SECTOR;
						//memcpy(&tmpbuf[1], DataPassWd, 6);
						memcpy(&tmpbuf[1], vifdata->KeyA, 6);
						sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_VIFKEYA, tmpbuf, 7, senddata);//验证KEY A 验证A密码 注册信息
						if ((MTSenddRv(senddata,sendlen,&rcvinfo) == 1)){
							if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_VIFKEYA) && (rcvinfo.buf[0] == VIF_DATA_SECTOR) && (rcvinfo.buf[1] ==MT_ACK_OK)){
								
								for (i = 0; i < 2; i++){
									tmpbuf2[0] = vifdata->lockst;
									tmpbuf2[1] = vifdata->Money;
									tmpbuf2[2] = (vifdata->Money >> 8) & 0xff;
									tmpbuf2[3] = (vifdata->Money >> 16) & 0xff;
									tmpbuf2[4] = vifdata->recdInx;
									tmpbuf2[5] = vifdata->rechargeInx;
									memcpy(&tmpbuf2[6], vifdata->PileNumber, 4);
									memset(&tmpbuf2[10], 0, 5);
									tmpbuf2[15] = MyCrc8(tmpbuf2, 15);
									Encrypt((const uint8_t *)&tmpbuf2, 0, &tmpbuf[2], 16);
									tmpbuf[0] = VIF_DATA_SECTOR;
									tmpbuf[1] = VIF_DATA_BLOCK + i;
									sendlen = MTMakeMsg(MT_CMD_TYPE2, MT_CMD_M1_BLOCKWR, tmpbuf, (2+16),senddata);
									if (MTSenddRv(senddata,sendlen,&rcvinfo) == 1){
										if ((rcvinfo.type==MT_CMD_TYPE2) && (rcvinfo.cmd==MT_CMD_M1_BLOCKWR) && (rcvinfo.buf[0] == tmpbuf[0]) && (rcvinfo.buf[1] == tmpbuf[1]) && (rcvinfo.buf[2] ==MT_ACK_OK)){
											//return 1;
										} else {
											break;
										}
									}
								}
								if (i == 2){
									return 1;
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}


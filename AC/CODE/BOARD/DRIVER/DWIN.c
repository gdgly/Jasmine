#include"Board.h"
#include "RTC.h"
#include "UART.h"
#include "DWIN.h"
#include <string.h>
#include "Timer.h"

void DwinInit(void)
{
	UartInit(DISPUARTNO,115200);
}

void DwinShowPage(uint16_t no)
{
	uint8_t  data[32]={0};
	uint16_t datalen=0;

	data[datalen++]=no/256;
	data[datalen++]=no%256;

	DwinWriteData(CMD_WRREGIT,REG_PAGE,datalen,data);
}

void DwinWriteData(uint8_t cmd,uint16_t addr,uint16_t datalen,uint8_t* data)
{
	uint8_t  sendbuf[256]={0};
	uint16_t sendlen=0;

	if (datalen > 200) {
		datalen = 200;
//		return;
	}

	sendbuf[sendlen++]=DWINHEAD1;  // 头
	sendbuf[sendlen++]=DWINHEAD2;

	sendbuf[sendlen++]=0;  // 长度

	sendbuf[sendlen++]=cmd;		   //命令	

	
	if (cmd == CMD_WRREGIT)   // 写寄存器
	{
		sendbuf[sendlen++]=(uint8_t)addr;
		if (data!=NULL && datalen!=0)
		{
			memcpy(sendbuf+sendlen,data,datalen);
			sendlen += datalen;
		}
	}
	if (cmd == CMD_RDREGIT)   // 读寄存器
	{
		sendbuf[sendlen++]=(uint8_t)addr;
		sendbuf[sendlen++]=datalen;  // 要读的长度
	}

	
	if (cmd == CMD_WRVAR)	 //  写变量	
	{
		sendbuf[sendlen++]=addr/256;
		sendbuf[sendlen++]=addr%256;
		if (data!=NULL && datalen!=0)
		{
			memcpy(sendbuf+sendlen,data,datalen);
			sendlen += datalen;
		}
	}
	
	if (cmd == CMD_RDVAR)	 // 读变量
	{
		sendbuf[sendlen++]=addr/256;
		sendbuf[sendlen++]=addr%256;
		sendbuf[sendlen++]=datalen;  // 要读的长度
	}

	sendbuf[2]=sendlen-3;    //长度

	UartWrite(DISPUARTNO,sendbuf,sendlen);
	
	
}

uint8_t DwinReadData(DwinRcvMsg *msg)
{
	uint8_t  dat8=0;
	static uint32_t time=0;
	static uint8_t step=0;
	static uint8_t len=0;
	static DwinRcvMsg info;
	static uint8_t i=0;

	if (msg == NULL) 
	{
		return 0;
	}

	if (TimerRead()-time>=T1S*2)
	{
		step=0;
	}
	
	while(UartRead(DISPUARTNO,&dat8,1))	 // 从串口缓冲区取数据
	{
		time = TimerRead();	 //记录时间
		
		switch(step)
	 	{
			case 0:
				if (dat8 == DWINHEAD1)				 // 头 1
				{
					memset((char *)&info,0,sizeof(DwinRcvMsg));
					step++;
				}
				else
				{
					step=0;
				}
				break;

			case 1:
				if (dat8 == DWINHEAD2)				 // 头 1
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				
			case 2:							  // 长度			
				len=dat8;
				if ((len<=1) || (len>200))
				{
					step=0;
				}
				else
				{
					step++;
				}
				break;
				 
			case 3:							// 类型
				info.cmd = dat8;
				if (info.cmd == CMD_RDREGIT) //一个地址字节
				{
					step++;
					step++;
				}
				else if (info.cmd == CMD_RDVAR) // 两个地址字节
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;

			case 4:
				info.addr = dat8*256;
				step++;
				break;

			case 5:
				info.addr += dat8;
				step++;
				break;

				 
			case 6:
				info.buflen += dat8*2;
				i=0;
			  	step++;
				break;
			
			case 7:							// 数据
				info.buf[i++]=dat8;
				if(i==info.buflen)
   				{  	
					step=0;	 
		  			memcpy(msg,&info,sizeof(info));			
					return 1;
	 			}
				break;
				
			default:
				break;
		}

	}

	return 0;

	
}


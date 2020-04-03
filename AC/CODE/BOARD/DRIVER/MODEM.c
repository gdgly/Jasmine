#include "Board.h"
#include "MODEM.h"
#include "GPIO.h"
#include <string.h>
#include "UART.h"
#include "includes.h"
#include "MyAlgLib.h"
#include <stdio.h>
#include "Timer.h"

static MODEMRCVMSG  ModemRcvMsg;      // 缓存

/************************************************************************************************************
** 函 数 名 : ModemInit
** 功能描述 : 无线模块初始化
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void ModemInit(void)
{
	UartInit(GPRSUARTNO, 115200); //串口

//	GpioSetDir(GPIO_GPRSRST, OUTPUT); //复位脚
//	GpioSetLev(GPIO_GPRSRST, LOW);

//	GpioSetDir(GPIO_GPRSPWR, OUTPUT); //电源
//	GpioSetLev(GPIO_GPRSPWR, HIGH);
//	OSTimeDlyHMSM (0, 0, 10, 0); //延时，让模块放电放干净
//	GpioSetLev(GPIO_GPRSPWR, LOW);
//	OSTimeDlyHMSM (0, 0, 10, 0); //延时，等待模块启动
	


	GpioSetDir(GPIO_GPRSPWR, OUTPUT); //电源
	GpioSetLev(GPIO_GPRSPWR, HIGH);
	OSTimeDlyHMSM (0, 0, 10, 0); //延时，让模块放电放干净
	GpioSetLev(GPIO_GPRSPWR, LOW);
	OSTimeDlyHMSM (0, 0, 3, 0); //延时，等待模块启动
	
	GpioSetDir(GPIO_GPRSRST, OUTPUT); //复位脚
	GpioSetLev(GPIO_GPRSRST, HIGH);
	OSTimeDlyHMSM (0, 0, 1, 0);
	GpioSetLev(GPIO_GPRSRST, LOW);
	OSTimeDlyHMSM (0, 0, 1, 0);
	
}

/************************************************************************************************************
** 函 数 名 : ModemConfig
** 功能描述 : 无线模块配置
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
uint8_t ModemConfig(void)
{
//	if (ModemWrWaitRd("ATE","OK",3,NULL) != ATOK)     		// AT 测试
//	{
//		return 0;
//	}
	if (ModemWrWaitRd("ATI","OK",1,NULL) != ATOK)     		// AT 测试
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+CGMR","OK",3,NULL) != ATOK)     	// 查版本号
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+CPIN?","OK",5,NULL) != ATOK)  	// 查卡
	{
		return 0;
	}

	if (ModemWrWaitRd("ATE0","OK",3,NULL) != ATOK)    		// 关闭回显
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+CFUN=1","OK",3,NULL) != ATOK) 	// 开启全功能
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+ZSNT=0,0,0","OK",3,NULL) != ATOK)  // 自动选择网络
	{
		return 0;
	}

	return 1;
}


/****************************************************************************\
 Function:  ModemConnect
 Descript:  无线模块连接服务器
 Input:	    SOCKID  类型(TCP/UDP)   APN,IP PORT	
 Output:    无 
 Return:    1 OK  0 fail
 Author:    quanhouwei
 Datetime:  17-09-07
 *****************************************************************************/
uint8_t ModemConnect(uint8_t sockid,uint8_t type,uint8_t *apn,uint8_t *dip,uint16_t dport,uint16_t port)
{
	uint8_t sendbuf[128]={0},rcvbuf[128]={0};
	uint8_t *p=NULL,csq=0,creg=0,cgreg=0;
	
	if ((sockid>=GPRSMAXSOCKET) || (apn==NULL) || (dip==NULL) || (dport==0) || (port==0) )
	{
		return 0;
	}

	if (ModemWrWaitRd("AT","OK",3,NULL) != ATOK)		// AT 测试
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+CPIN?","OK",5,NULL) != ATOK)  // 查卡
	{
		return 0;
	}

	ModemDisCon(sockid);          						// 断开连接

	
	if (ModemWrWaitRd("AT+CSQ","OK",3,rcvbuf) != ATOK) // 查信号
	{
		return 0;
	}
	p=(uint8_t *)strstr((char *)rcvbuf,"+CSQ: ");
    if(p == NULL)
	{
		return 0;
	}
	p+=strlen((char *)"+CSQ: ");
    csq=MyAtoi(p);
    if (csq <=5)
    {
       return 0;
    }


	if (ModemWrWaitRd("AT+CREG?","OK",3,rcvbuf) != ATOK) // 卡是否注册网络 1 登录本地网络  5 登录漫游网络
  	{
  		return 0;
	}
    p=(uint8_t *)strstr((char *)rcvbuf,"+CREG: ");
	if (p == NULL)
	{
		return 0;
	}
    p+=strlen((char *)"+CREG: ");
    p+=2;
    creg=*p-'0';
    if ((creg!=1) && (creg!=5))  
    {
        return 0;
    }


	if (ModemWrWaitRd("AT+CGREG?","OK",180,rcvbuf) != ATOK) // 是否附着网络
	{
		return 0;
	}
	p=(uint8_t *)strstr((char *)rcvbuf,"+CGREG: ");
	if (p == NULL)
	{
		return 0;
	}
	p+=strlen((char *)"+CGREG: ");
	p+=2;
	cgreg=*p-'0';
	if (cgreg != 1)
	{
		return 0;
	}


	sprintf((char *)sendbuf,"AT+ZIPCFG=%s",apn);     // APN
	if (ModemWrWaitRd(sendbuf,"OK",3,NULL) != ATOK)
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+ZIPCALL=1","OK",150,NULL) != ATOK)  // 打开网络
	{
		return 0;
	}


	sprintf((char *)sendbuf,"AT+ZIPOPEN=%d,%d,%s,%d,%d",sockid,type,dip,dport,port);	 // 连接
	if (ModemWrWaitRd(sendbuf,"OK",150,NULL) != ATOK)
	{
		return 0;
	}

	if (ModemGetConSt(sockid) == 1)    // 获取连接状态
	{
		return 1;
	}
	else
	{
		return 0;
	}

}



/****************************************************************************\
 Function:  ModemDisCon
 Descript:  断开连接
 Input:	    SOCKID 	
 Output:    无 
 Return:    1 OK  0 fail
 Author:    quanhouwei
 Datetime:  17-09-07
 *****************************************************************************/
uint8_t ModemDisCon(uint8_t sockid)
{
	uint8_t sendbuf[128]={0};

	if (sockid >= GPRSMAXSOCKET)
	{
		return 0;
	}
	
	sprintf((char *)sendbuf,"AT+ZIPCLOSE=%d",sockid);
	if (ModemWrWaitRd(sendbuf,"OK",3,NULL) != ATOK)
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+ZIPCALL=0","OK",3,NULL) != ATOK)
	{
		return 0;
	}

	return 1;
}


/****************************************************************************\
 Function:  ModemGetLinkSt
 Descript:  获取连接状态
 Input:	    SOCKID 	
 Output:    无 
 Return:    1 连接  0 未连接
 Author:    quanhouwei
 Datetime:  17-09-07
 *****************************************************************************/
uint8_t ModemGetConSt(uint8_t sockid)
{
	uint8_t *p=NULL,sendbuf[128]={0},dat8=0;

	if (sockid >= GPRSMAXSOCKET)
	{
		return 0;
	}
	
	sprintf((char *)sendbuf,"AT+ZIPSTAT=%d",sockid);
	if (ModemWrWaitRd(sendbuf,"OK",3,NULL) != ATOK)
	{
		return 0;
	}
	p=(uint8_t *)strstr((char *)p,"+ZIPSTAT: ");
    if(p == NULL)
	{
		return 0;
	}
	p+=strlen((char *)"+ZIPSTAT: ");
    dat8=*p-'0';
	if (dat8 != sockid)
	{
		return 0;
	}
	p+=2;
	dat8=*p-'0';
	if (dat8 == 1)
	{
		return 1;
	}

	return 0;
}



/****************************************************************************\
 Function:  ModemSendData
 Descript:  发送数据
 Input:	    SOCKID 	数据  数据长度
 Output:    无 
 Return:    1 OK  0 fail
 Author:    quanhouwei
 Datetime:  17-09-07
 *****************************************************************************/
uint8_t ModemSendData(uint8_t sockid,uint8_t *data,uint16_t datalen)
{
	uint8_t sendbuf[128]={0};

	if (ModemWrWaitRd("AT","OK",3,NULL) != ATOK)
	{
		return 0;
	}

	if (ModemWrWaitRd("AT+CPIN?","OK",3,NULL) != ATOK)
	{
		return 0;
	}

	if (ModemGetConSt(sockid) == 0)     // 获取连接状态
	{
		return 0;
	}
	
	sprintf((char *)sendbuf,"AT+ZIPSENDRAW=%d,%d,0",sockid,datalen);
	if (ModemWrWaitRd(sendbuf,">",3,NULL) == ATOK)
	{
		UartWrite(GPRSUARTNO,data,datalen);

		return 1;
	}
	else
	{
		return 0;
	}
}


/****************************************************************************\
 Function:  ModemRcvData
 Descript:  接收数据
 Input:	    SOCKID 	数据  
 Output:    无 
 Return:    数到数据的长度
 Author:    quanhouwei
 Datetime:  17-09-07
 *****************************************************************************/
uint16_t ModemRcvData(uint8_t sockid,uint8_t *outbuf)
{
	static  uint8_t	step=0;
	static  uint16_t len=0;
//	static  uint8_t	type=0;
	static  uint32_t time=0;
	static  uint8_t	rcvbuf[MODEMBUFLEN]={0};
	static  uint16_t rcvlen=0;
	uint8_t	data=0,dat8=0;
	

	while(UartRead(GPRSUARTNO,&dat8,1))        
  	{
       	ModemBufWrByte(dat8);        // 放到串口缓冲区
  	}
	
	if (TimerRead()-time >= T1S*3)	 // 3S内数据未收完就退出收数据	
	{
		step=0;
	}
	
	while(ModemBufRdByte(&data))		 // 从串口缓冲区取数据
	{
		time = TimerRead();	 //记录时间
		switch(step)
	 	{
			case 0:
				if (data == '+')				 // 头
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 1:
				if (data == 'Z')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 2:
				if (data == 'I')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 3:
				if (data == 'P')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 4:
				if (data == 'R')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 5:
				if (data == 'E')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 6:
				if (data == 'C')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 7:
				if (data == 'V')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;
				 
			case 8:
				if (data == ':')
				{
//					type=0;
					step++;
				}
				else
				{
					step=0;
				}
				break;
	
			case 9:
//				type=data-'0';
				break;
	
			case 10:
				if (data == ',')
				{
					step++;
				}
				else
				{
					step=0;
				}
				break;

			case 11:
				if (data == ',')
			 	{
			 		len=0;
					step++;
			 	}
				break;
				 
			  case 12:						 // 长度 ()
				 if ((data<='9') && (data>='0'))
				 {	
					 len=len*10+(data-'0');
				 }
				 else
				 {
					 if ((len==0) || (len>= MODEMBUFLEN))
					 {
						 step=0;
					 }
					 else
					 { 
						 step++;
					 }
				 }
				 break;
	
			   case 13:  
				 if (data == ',')	
				 {
					 rcvlen=0;
					 step++;
				 }
				 else
				 {
					 step=0;
				 }
				 break;
				 
	
	
			  case 14:			  //数据
				 rcvbuf[rcvlen++]=data;
				 if (rcvlen == len)
				 {
					 step=0;
					 memcpy(outbuf,rcvbuf,rcvlen);
					 return rcvlen;
					
				 }
				 break;
	
			  default:
				 step=0;
				 break;
				 
		  }
	 }
	
	 return 1;
	
//	return NODATA;
}



	























/************************************************************************************************************
** 函 数 名 : ModemWrWaitRd
** 功能描述 : 发送AT命令并等待返回值
** 输    入 : data 发送数据 ack 返回值 time 超时时间 s
** 输    出 : outbuf 返回的数据
** 返    回 :	无
*************************************************************************************************************
*/
static uint8_t ModemWrWaitRd(uint8_t *data,uint8_t* ack,uint16_t time,uint8_t *outbuf)
{
	uint16_t len=0,i=0;
	uint8_t  rcvbuf[MODEMBUFLEN]={0};
	uint8_t  dat8=0;

	//清空数据
	while (UartRead(GPRSUARTNO, &dat8, 1));

	UartWrite(GPRSUARTNO, data, strlen((char *)data));
	UartWrite(GPRSUARTNO, "\x0D", 1);
	UartWrite(GPRSUARTNO, "\x0A", 1);

	print("send:%s\r\n",data);

	for (i = 0; i < (time * 1000); i++){
		OSTimeDlyHMSM (0, 0, 0, 1);
		while (UartRead(GPRSUARTNO, &dat8, 1)){
			ModemBufWrByte(dat8);        // 放到串口缓冲区

			if (len <= MODEMBUFLEN){
				rcvbuf[len++] = dat8;
			}
			if(strstr((char *)rcvbuf,(char *)ack)){// 收到正确应答
				print("rcv:%s\r\n",rcvbuf);
				return ATOK;
			}
			if(strstr((char *)rcvbuf,"ERROR")){// 收到正确应答
				print("rcv:%s\r\n",rcvbuf);
				return ATERR;
			}
		}
	}

	return ATTIMEOUT;
}


/****************************************************************************\
 Function:  ModemBufWrByte
 Descript:  写一个字节数据到缓冲区
 Input:	    需写入的字节数据
 Output:    返回的数据
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-24
 *****************************************************************************/
static void ModemBufWrByte(uint8_t data)
{
	if ((ModemRcvMsg.in>=MODEMBUFLEN)  || (ModemRcvMsg.out>=MODEMBUFLEN))    // 出错
    {
        ModemRcvMsg.in = 0;
        ModemRcvMsg.out = 0;
    }

    ModemRcvMsg.buf[ModemRcvMsg.in] = data;

    if (++ModemRcvMsg.in >= MODEMBUFLEN)
    {
        ModemRcvMsg.in = 0;
    }

    if (ModemRcvMsg.in == ModemRcvMsg.out)
    {
        ModemRcvMsg.out++;
        if (ModemRcvMsg.out >= MODEMBUFLEN)
        {
            ModemRcvMsg.out = 0;
        } 
    } 
}



/****************************************************************************\
 Function:  ModemBufRdByte
 Descript: 从缓冲区读一个字节数据
 Input:	    无
 Output:   读出的数据
 Return:   ０　未读出数据　　１　读到数据
 Author:    quanhouwei
 Datetime:  17-08-24
 *****************************************************************************/
static uint8_t ModemBufRdByte(uint8_t *data)
{
	if ((ModemRcvMsg.in>=MODEMBUFLEN)  || (ModemRcvMsg.out>=MODEMBUFLEN))   // 出错
    {
        ModemRcvMsg.in = 0;
        ModemRcvMsg.out = 0;
        return 0;
    }  

    if (ModemRcvMsg.in == ModemRcvMsg.out)      // 没有数据
    {
        return 0;
    }


    *data = ModemRcvMsg.buf[ModemRcvMsg.out];

    ModemRcvMsg.out++;
    if (ModemRcvMsg.out >= MODEMBUFLEN)
    {
       ModemRcvMsg.out = 0;
    }

    return 1;
	
}


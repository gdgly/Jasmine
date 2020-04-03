#include <stdint.h>
#include "App.h"
#include "ElmTask.h"
#include "RTC.h"
#include "RS485.h"
#include "UART.h"
#include "Timer.h"
#include "Board.h"
#include "Para.h"
#include "Myalglib.h"

static ELMINFO  ElmInfo[ELMMAXCNT];

/************************************************************************************************************
** 函 数 名 : ElmCheckMsg
** 功能描述 : 校验数据
** 输    入 : 数据，长度 结构体数据
** 输    出 : 无
** 返    回 :	0 FAIL  1 OK
*************************************************************************************************************
*/
static uint8_t ElmCheckMsg(uint8_t *data,uint16_t datalen,ElmRcvMsg *msg)
{
	uint8_t  checksum=0,len=0;
	uint16_t i=0;
	
	if ((data==NULL) || (datalen<12) || (datalen>200) || (msg==NULL)){
		return 0;
	}
	
	for (i = 0; i < datalen; i++){
		if (data[i] != 0xfe)
			break;
	}

	data += i;

	if (data[len++] != ELMHEAD)					// 头(1)
	{
		return 0;
	}

	memcpy(msg->addr,data+len,6);	 			// 地址 (6)	
	len+=6;

	if (*(data+len) != ELMHEAD)					// 头 (1)
	{
		return 0;
	}
	len++;

	msg->ctlcode=*(data+len);					// 控制码 (1)
	len++;

	msg->buflen=*(data+len);					// 数据域长度 (1)
	if (msg->buflen >200)
	{
		return 0;
	}
	len++;

	if (msg->buflen != 0)						// 数据域
	{
		for (i=0;i<msg->buflen;i++)
		{
			msg->buf[i]=*(data+len+i)-0x33;
		}
		len+=msg->buflen;	
	}
		
	checksum=MyCum8Sum(data,len);			// 校验 (1)
	
	if (checksum != *(data+len))
	{
		return 0;
	}
	len++;

	if (*(data+len) != ELMTAIL)					// 尾(1)
	{
		return 0;
	}

	return 1;
}

/************************************************************************************************************
** 函 数 名 : ElmSenddRv
** 功能描述 : 发送数据并等待返回
** 输    入 : 需发送的数据和长度
** 输    出 : 无
** 返    回 :	0 FAIL  1 OK
*************************************************************************************************************
*/
static uint8_t ElmSenddRv(uint8_t id,uint8_t *data,uint8_t datalen,ElmRcvMsg *msg)
{
	uint8_t  tmpbuf[100]={0},rcvbuf[128]={0};
	uint16_t tmplen=0,rcvlen=0;
	uint32_t timer;
	
	if ((data==NULL) || (datalen==0))
	{
		return 0;
	}

	RS485Flush();          			// 清空串口缓存
	RS485Write(data,datalen);		// 发送数据

	timer = TimerRead();
	while (TimerRead() - timer < T1S * 3){
		DelayMs(1);
		
		tmplen=RS485Read(tmpbuf,100);
		if (tmplen != 0)
		{
			if (rcvlen+tmplen >= sizeof(rcvbuf))
			{
				return 0;
			}
			memcpy(rcvbuf+rcvlen,tmpbuf,tmplen);
			rcvlen += tmplen;
		
			if (ElmCheckMsg(rcvbuf,rcvlen,msg) == 1)
			{
				//break;
				return 1;
			}
		}
	}
	return 0;
}

/************************************************************************************************************
** 函 数 名 : ElmMakeMsg
** 功能描述 : 组包函数
** 输    入 : 地址，控制码，数据类型，数据和长度
** 输    出 : 无
** 返    回 :	组包的数据长度
*************************************************************************************************************
*/
static uint16_t ElmMakeMsg(uint8_t *addr,uint8_t ctlcode,uint32_t type,uint8_t *data,uint16_t datalen,uint8_t *outbuf)
{
	uint16_t outlen=0,i=0;

	if ((addr==NULL) || (outbuf==NULL) || (datalen>46))
	{
		return 0;
	}
	
	outbuf[outlen++] = 0xfe;
	outbuf[outlen++] = 0xfe;
	outbuf[outlen++] = 0xfe;
	outbuf[outlen++] = 0xfe;

	outbuf[outlen++]=ELMHEAD;  						// 头 (1)

	memcpy(outbuf+outlen,addr,6);							// 地址 (6)
	outlen+=6;

	outbuf[outlen++]=ELMHEAD;  						// 头 (1)

	outbuf[outlen++]=ctlcode;  						// 控制码 (1)

	outbuf[outlen++]=4+datalen;        				// 长度 (1)

    outbuf[outlen++]=(type%256)+0x33;  				//数据标识
	outbuf[outlen++]= (type>>8)%256+0x33;
	outbuf[outlen++]= (type>>16)%256+0x33;
	outbuf[outlen++]= (type>>24)%256+0x33;

	if ((data!=NULL) && (datalen!=0))
	{
		for (i=0;i<datalen;i++)
		{
			outbuf[outlen++]=*(data+i)+0x33;
		}
	}

	outbuf[outlen++]=MyCum8Sum(outbuf+4,outlen - 4);   // 校验 (1)  

	outbuf[outlen++]=ELMTAIL;						// 尾 (1)

    return outlen;

}

/************************************************************************************************************
** 函 数 名 : ElmRead
** 功能描述 : 读电表数据 电表ID,类型 数据指针
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
uint8_t ElmRead(uint8_t id,uint8_t readtype,uint32_t *data, uint8_t MeterAddr[6])
{
	uint8_t  senddata[128]={0};
	uint32_t type=0;
	uint16_t sendlen=0;
	ElmRcvMsg rcvmsg;
//	uint8_t addr[10]={0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};

	if (readtype == TYPE_ELECY)
	{
		sendlen=ElmMakeMsg(MeterAddr,ELM_CMD_READDATA,ELM_DI_READTOTALELC,NULL,0,senddata);
		if (ElmSenddRv(id,senddata,sendlen,&rcvmsg) != 1)
		{
			return 0;
		}
		if (rcvmsg.ctlcode == ELM_ACK_READDATA)
		{
			//printx(rcvmsg.buf,rcvmsg.buflen);
			memcpy(&type,rcvmsg.buf,4);
			if (type == ELM_DI_READTOTALELC)
			{
				*data=Mylbcdstr2hex(rcvmsg.buf+4,4);
				return 1;
			}
		}
	}

	

	if (readtype == TYPE_VOL)
	{
		sendlen=ElmMakeMsg(MeterAddr,ELM_CMD_READDATA,ELM_DI_READVOL,NULL,0,senddata);
		if (ElmSenddRv(id,senddata,sendlen,&rcvmsg) != 1)
		{
			return 0;
		}
		if (rcvmsg.ctlcode == ELM_ACK_READDATA)
		{
			memcpy(&type,rcvmsg.buf,4);
			if (type == ELM_DI_READVOL)
			{
				data[0]=Mylbcdstr2hex(rcvmsg.buf+4,2);
				data[1]=Mylbcdstr2hex(rcvmsg.buf+6,2);
				data[2]=Mylbcdstr2hex(rcvmsg.buf+8,2);
				return 1;
			}	
		}
	}


	if (readtype == TYPE_CURRENT)
	{
		sendlen=ElmMakeMsg(MeterAddr,ELM_CMD_READDATA,ELM_DI_READCURRENT,NULL,0,senddata);
		if (ElmSenddRv(id,senddata,sendlen,&rcvmsg) != 1)
		{
			return 0;
		}
		
		if (rcvmsg.ctlcode == ELM_ACK_READDATA)
		{
			memcpy(&type,rcvmsg.buf,4);
			if (type == ELM_DI_READCURRENT)
			{
				data[0]=Mylbcdstr2hex(rcvmsg.buf+4,3);
				data[1]=Mylbcdstr2hex(rcvmsg.buf+7,3);
				data[2]=Mylbcdstr2hex(rcvmsg.buf+10,3);
				return 1;
			}
		}
	}


	if (readtype == TYPE_DATE)
	{
		sendlen=ElmMakeMsg(MeterAddr,ELM_CMD_READDATA,ELM_DI_READDAY,NULL,0,senddata);
		if (ElmSenddRv(id,senddata,sendlen,&rcvmsg) != 1)
		{
			return 0;
		}
		if (rcvmsg.ctlcode == ELM_ACK_READDATA)
		{
			memcpy(&type,rcvmsg.buf,4);
			if (type == ELM_DI_READDAY)
			{
				data[0]=Mybcd2hex(rcvmsg.buf[7]);
				data[1]=Mybcd2hex(rcvmsg.buf[6]);
				data[2]=Mybcd2hex(rcvmsg.buf[5]);
				return 1;
				
			}
		}
	}


	if (readtype == TYPE_TIME)
	{
		if (ElmSenddRv(id,senddata,sendlen,&rcvmsg) != 1)
		{
			return 0;
		}
		if (rcvmsg.ctlcode == ELM_ACK_READDATA)
		{
			memcpy(&type,rcvmsg.buf,4);
			if (type == ELM_DI_READTIME)
			{
				data[0]=Mybcd2hex(rcvmsg.buf[6]);
				data[1]=Mybcd2hex(rcvmsg.buf[5]);
				data[2]=Mybcd2hex(rcvmsg.buf[4]);
				return 1;
				
			}
		}
	
	}

	return 0;
	
}

/************************************************************************************************************
** 函 数 名 : ElmReadData
** 功能描述 : 读电表数据 (总电量，电压，电流)
** 输    入 : MeterAddr 电表地址
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void ElmReadData(uint8_t id, uint8_t MeterAddr[6])
{
	uint8_t ret=0;

	ret = ElmRead(id,TYPE_ELECY,&(ElmInfo[id].totalelc), MeterAddr);
	if (ret == 1){
		ElmInfo[id].timeoutcnt=0;
	} else {
		print("warnning:read meter ELE timeout!");
		ElmInfo[id].timeoutcnt++;
	}
		
	OSTimeDlyHMSM (0, 0, 0, 200);

	ret = ElmRead(id,TYPE_VOL,&(ElmInfo[id].vol[0]), MeterAddr);
	if (ret == 1){
		ElmInfo[id].timeoutcnt=0;
	} else {
		print("warnning:read meter VOL timeout!");
		ElmInfo[id].timeoutcnt++;
	}
	
	OSTimeDlyHMSM (0, 0, 0, 200);

	ret = ElmRead(id,TYPE_CURRENT,&(ElmInfo[id].current[0]), MeterAddr);	
	if (ret == 1){
		ElmInfo[id].timeoutcnt=0;
	} else {
		print("warnning:read meter CUR timeout!");
		ElmInfo[id].timeoutcnt++;
	}
	
	OSTimeDlyHMSM (0, 0, 0, 200);
}

/************************************************************************************************************
** 函 数 名 : ElmGetTotalElc
** 功能描述 : 获取电表总电量
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电量 含2位小数
*************************************************************************************************************
*/
uint32_t ElmGetTotalElc(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	
	return ElmInfo[id].totalelc;
}

/************************************************************************************************************
** 函 数 名 : ElmGetVolA
** 功能描述 : 获取电表电压 A相
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电压 含1位小数
*************************************************************************************************************
*/
uint32_t ElmGetVolA(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	
	return ElmInfo[id].vol[0];
}

/************************************************************************************************************
** 函 数 名 : ElmGetVolB
** 功能描述 : 获取电表电压 B相
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电压 含1位小数
*************************************************************************************************************
*/
uint32_t ElmGetVolB(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	return ElmInfo[id].vol[1];
}

/************************************************************************************************************
** 函 数 名 : ElmGetVolB
** 功能描述 : 获取电表电压 C相
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电压 含1位小数
*************************************************************************************************************
*/
uint32_t ElmGetVolC(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	return ElmInfo[id].vol[2];
}

/************************************************************************************************************
** 函 数 名 : ElmGetCurA
** 功能描述 : 获取电表电流 A相
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电流 含3位小数
*************************************************************************************************************
*/
uint32_t ElmGetCurA(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	
	return ElmInfo[id].current[0];
}

/************************************************************************************************************
** 函 数 名 : ElmGetCurB
** 功能描述 : 获取电表电流 B相
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电流 含3位小数
*************************************************************************************************************
*/
uint32_t ElmGetCurB(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	return ElmInfo[id].current[1];
}

/************************************************************************************************************
** 函 数 名 : ElmGetCurC
** 功能描述 : 获取电表电流 C相
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电流 含3位小数
*************************************************************************************************************
*/
uint32_t ElmGetCurC(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	return ElmInfo[id].current[2];
}


/************************************************************************************************************
** 函 数 名 : ElmGetMaxCur
** 功能描述 : 获取电表三相电流中的最大电流值
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电流 含3位小数
*************************************************************************************************************
*/
uint32_t ElmGetMaxCur(uint8_t id)
{
	uint32_t Max;
	if (id >= ELMMAXCNT){
		return 0;
	}
	Max = ElmGetCurA(id) > ElmGetCurB(id) ? ElmGetCurA(id):ElmGetCurB(id);
	Max = ElmGetCurC(id) > Max ? ElmGetCurC(id):Max;
	return Max;
	
}



/************************************************************************************************************
** 函 数 名 : ElmGetMaxVol
** 功能描述 : 获取电表三相电压中的最大电压值
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 电压 含1位小数
*************************************************************************************************************
*/
uint32_t ElmGetMaxVol(uint8_t id)
{
	uint32_t Max;
	if (id >= ELMMAXCNT){
		return 0;
	}
	Max = ElmGetVolA(id) > ElmGetVolB(id) ? ElmGetVolA(id):ElmGetVolB(id);
	Max = ElmGetVolC(id) > Max ? ElmGetVolC(id):Max;
	return Max;
	
}

/************************************************************************************************************
** 函 数 名 : ElmGetCurC
** 功能描述 : 获取电表通信状态
** 输    入 : id 0/1
** 输    出 : 无
** 返    回 : 0通信正常 1通信异常
*************************************************************************************************************
*/
uint8_t ElmGetCommState(uint8_t id)
{
	if (id >= ELMMAXCNT)
	{
		return 0;
	}
	
	return ElmInfo[id].commstate;
}

/************************************************************************************************************
** 函 数 名 : ElmProc
** 功能描述 : 电表流程
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void ElmProc(void)
{
	static uint32_t timer,cnt;
	uint8_t MeterAddr[6];
	uint8_t PortNum = ParaGetChgGunNumber();
	
	if (TimerRead() - timer > T1S * 1){
		timer = TimerRead();
		if (PortNum == 1){//单枪
			memset(MeterAddr, 0xAA, 6);
			ElmReadData(0, MeterAddr);
		} else {//双枪
			if (cnt == 0){
				cnt = 1;
				ParaGetACMeterAddr(0, MeterAddr);
				ElmReadData(0, MeterAddr);
			} else {
				cnt = 0;
				ParaGetACMeterAddr(1, MeterAddr);
				ElmReadData(1, MeterAddr);
			}
		}
	}
	
	if (ElmInfo[0].timeoutcnt >= 5){
		ElmInfo[0].commstate = 1;
	} else {
		ElmInfo[0].commstate = 0;
	}
	if (ElmInfo[1].timeoutcnt >= 5){
		ElmInfo[1].commstate = 1;
	} else {
		ElmInfo[1].commstate = 0;
	}
}

/************************************************************************************************************
** 函 数 名 : ElmTask
** 功能描述 : 读电表任务
** 输    入 : pdata 不使用
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void ElmTask(void *pdata)
{
	RS485Init(ELMBAUD);
	print("Elm task start...\r\n");
	
	while(1){
		ElmProc();
		OSTimeDlyHMSM (0, 0, 0, 15);
	}
}


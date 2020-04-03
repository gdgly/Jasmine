#include "EXT_ChgTask.h"
static struct{
	CHARGEINFO info;
	uint8_t CCBState; //充电控制板状态
	uint8_t State; //状态
	uint8_t StartResult;//启动结果 0成功 1失败
	uint8_t StartFailCause;//启动失败原因
	uint8_t RecvCCB_PF40_Flag;
	uint8_t RecvCCB_PF41_Flag;
	uint8_t RecvCCB_PF42_Flag;
	uint8_t RecvCCB_PF43_Flag;
	uint8_t RecvCCB_PF44_Flag;
	uint8_t RecvCCB_PF45_Flag;
	uint8_t CCBStopCause; 
}ChgCtl[2];

static uint8_t StartFlag[2],StartRet[2];
static uint8_t StopFlag[2],StopRet[2];
static uint16_t Readpwm[2];

/************************************************************************************************************
** 函 数 名 : ReadPwm
** 功能描述 : 获取充电板发送的PWM
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	充电板发送的PWM
*************************************************************************************************************
*/
uint16_t ReadPwm(uint8_t port)
{
	return Readpwm[port];
}

/************************************************************************************************************
** 函 数 名 : SendDataToCCB
** 功能描述 : 发送数据到充电控制板函数
** 输    入 : canid：CAN的ID号
							pdata：要发送的数据缓冲区地址
							len：要发送的字节数
** 输    出 : 无
** 返    回 :	校验和
*************************************************************************************************************
*/
static void SendDataToCCB(uint32_t canid,uint8_t *pdata,uint8_t len)
{
	if (CANWrite(DEV_CAN1, canid, pdata, len) == 0){
		OSTimeDlyHMSM (0, 0, 0, 1);
		CANWrite(DEV_CAN1, canid, pdata, len);
	}
	OSTimeDlyHMSM (0, 0, 0, 1);
}

/*********************************************************************************************************
** Function name:			MultiFramePackAndSend
** Descriptions:			?????????
** Input parameters:	canid:CAN?ID?
											pdata:???????????
											len:???????
** Output parameters:	?
** Returned value:		?
*********************************************************************************************************/
static void MultiFramePackAndSend(uint32_t id, uint8_t *pdata, uint16_t len)
{
	uint8_t data[8]={0,0,0,0,0,0,0,0},*p;
	uint8_t packnum,num=2;
	uint16_t remainlen;
	
	if (len <= 8){
		SendDataToCCB(id, pdata, 8);
	} else {
		packnum = len / 7;
		if(len % 7){
			packnum++;
		}
		remainlen = len;
		p = pdata;
		for (num = 0; num < packnum; num++){
			memset(data, 0, 8);
			data[0] = num + 1;
			if(remainlen > 7){
				memcpy(&data[1], p, 7);
				remainlen -= 7;
				p += 7;
			} else {
				memcpy(&data[1], p, remainlen);
			}
			SendDataToCCB(id, data, 8);
		}
	}
}

/************************************************************************************************************
** 函 数 名 : SendPF40
** 功能描述 : 发送计费控制板启动充电帧
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SendPF40(uint8_t port)
{
	uint32_t id=0x104000a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	data[0] = 0;
	
	SendDataToCCB(id, data, 8);
}

/************************************************************************************************************
** 函 数 名 : SendPF41
** 功能描述 : 发送计费控制板应答充电控制板上报的启动充电完成帧
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SendPF41(uint8_t port)
{
	uint32_t id=0x104100a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	data[0] = 0;
	
	SendDataToCCB(id, data, 8);
}

/************************************************************************************************************
** 函 数 名 : SendPF42
** 功能描述 : 发送计费控制板停止充电帧
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SendPF42(uint8_t port)
{
	uint32_t id=0x104200a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	SendDataToCCB(id, data, 8);
}

/************************************************************************************************************
** 函 数 名 : SendPF43
** 功能描述 : 发送计费控制板应答充电控制板上报的停止充电完成帧
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SendPF43(uint8_t port)
{
	uint32_t id=0x104300a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	data[0] = 0;
	
	SendDataToCCB(id, data, 8);
}

/************************************************************************************************************
** 函 数 名 : SendPF46
** 功能描述 : 发送计费控制板遥测帧
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SendPF46(uint8_t port)
{
	uint32_t id=0x184600a0 | (port << 8);
	uint8_t data[16] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t data16;
	
	data16 = ElmGetVolA(port);
	data[0] = data16 & 0xff;
	data[1] = (data16 >> 8) & 0xff;
	data16 = ElmGetVolB(port);
	data[2] = data16 & 0xff;
	data[3] = (data16 >> 8) & 0xff;
	data16 = ElmGetVolC(port);
	data[4] = data16 & 0xff;
	data[5] = (data16 >> 8) & 0xff;
	data16 = ElmGetCurA(port);
	data[6] = data16 & 0xff;
	data[7] = (data16 >> 8) & 0xff;
	data16 = ElmGetCurB(port);
	data[8] = data16 & 0xff;
	data[9] = (data16 >> 8) & 0xff;
	data16 = ElmGetCurC(port);
	data[10] = data16 & 0xff;
	data[11] = (data16 >> 8) & 0xff;
	data16 = 0;//需改
	data[12] = data16 & 0xff;
	data[13] = (data16 >> 8) & 0xff;
	data16 = 0;//需改
	data[14] = data16 & 0xff;
	data[15] = (data16 >> 8) & 0xff;
	
	MultiFramePackAndSend(id, data, 16);
}

/************************************************************************************************************
** 函 数 名 : SendPF47
** 功能描述 : 发送计费控制板设置PWM 波
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SendPF47(uint8_t port)
{
	uint32_t id=0x104700a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t data16;
	
	data16 = GetCfgPwm(port);
	data[0] = data16 & 0xff;
	data[1] = (data16 >> 8) & 0xff;
	MultiFramePackAndSend(id, data, 8);
}


/************************************************************************************************************
** 函 数 名 : SendPF48
** 功能描述 : 发送计费控制板设置PWM 波
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void SendPF48(uint8_t port)
{
	uint32_t id=0x104800a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	MultiFramePackAndSend(id, data, 8);
}

/************************************************************************************************************
** 函 数 名 : AnalyseMsgFromCCB
** 功能描述 : 分析来自于充电控制板的数据
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void AnalyseMsgFromCCB(void)
{
	uint32_t id;
	uint8_t len,data[8],port = 0;
	uint8_t gunnum = ParaGetChgGunNumber();
	static uint8_t YaoCeBuffer[2][100];
	uint8_t *pdata;
	
	do{
		len = CANRead(DEV_CAN1, &id, data);
		if (len && (((id >> 8) & 0x000000ff) == 0xa0) && ((id & 0x000000ff) < 2)){
			port = id & 0x000000ff;
			switch(id >> 16){
				
				case 0x1040://启动充电应答帧
					ChgCtl[port].RecvCCB_PF40_Flag = 1;
					break;
				
				case 0x1041://启动完成帧
					ChgCtl[port].StartResult = data[0];
					ChgCtl[port].StartFailCause = data[1];
					SendPF41(port);
					ChgCtl[port].RecvCCB_PF41_Flag = 1;
					break;
				
				case 0x1042://停止充电应答帧
					ChgCtl[port].RecvCCB_PF42_Flag = 1;
					break;
				
				case 0x1043://停止完成帧
					SendPF43(port);
					ChgCtl[port].CCBStopCause = data[1];
					ChgCtl[port].RecvCCB_PF43_Flag = 1;
					break;
				
				case 0x1844://遥信
					if (data[0] & (1 << 0))
						ChgCtl[port].info.ScramState = 1;
					else
						ChgCtl[port].info.ScramState = 0;
					if (data[0] & (1 << 1))
						ChgCtl[port].info.DoorState = 1;
					else
						ChgCtl[port].info.DoorState = 0;
					if (data[0] & (1 << 2))
						ChgCtl[port].info.GunSeatState = 1;
					else
						ChgCtl[port].info.GunSeatState = 0;
					if (data[0] & (1 << 3))  //过压
						ChgCtl[port].info.HighVol = 1;
					else
						ChgCtl[port].info.HighVol = 0;
					if (data[0] & (1 << 4))  //欠压
						ChgCtl[port].info.LowVol = 1;
					else
						ChgCtl[port].info.LowVol = 0;
					ChgCtl[port].RecvCCB_PF44_Flag = 1;
					break;
					
				case 0x1845://遥测
					memcpy(&YaoCeBuffer[port][7 * (data[0] - 1)], &data[1], 7);
					pdata = YaoCeBuffer[port];
					ChgCtl[port].CCBState = *pdata++;
					memcpy(&ChgCtl[port].info.VolA, pdata, 2);
					pdata += 2;
					memcpy(&ChgCtl[port].info.VolB, pdata, 2);
					pdata += 2;
					memcpy(&ChgCtl[port].info.VolC, pdata, 2);
					pdata += 2;
					memcpy(&ChgCtl[port].info.CurA, pdata, 2);
					pdata += 2;
					memcpy(&ChgCtl[port].info.CurB, pdata, 2);
					pdata += 2;
					memcpy(&ChgCtl[port].info.CurC, pdata, 2);
					pdata += 2;
					memcpy(&ChgCtl[port].info.GuideVol, pdata, 2);
					pdata += 2;
					memcpy(&ChgCtl[port].info.PWMPuty, pdata, 2);
					pdata += 2;
					ChgCtl[port].info.TempL1 = *pdata++;
					ChgCtl[port].info.TempL2 = *pdata++;
					ChgCtl[port].info.TempL3 = *pdata++;
					ChgCtl[port].info.TempL4 = *pdata++;
					ChgCtl[port].RecvCCB_PF45_Flag = 1;
					break;
				case 0x1047://设置PWM回的数据		
					SetParaResult(port, 1);			
					break;	
				case 0x1048:
					Readpwm[port] =	(data[0] | (data[1]<<8));
					SetParaResult(port, 1);			
				default:
					break;
			}
		}
	}while(len);
}

/************************************************************************************************************
** 函 数 名 : ChgHandle
** 功能描述 : 充电处理函数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void ChgHandle(uint8_t port)
{
	static uint32_t Timer[2],StartTimer[2],StopTimer[2],YaoCeTimer[2],CommTimer[2];
	uint8_t Cmd = 0;
	Cmd = GetParaCmd(port);
	if(Cmd ==1){		
		SendPF47(port);//发送设置PWM报文
		OSTimeDlyHMSM (0, 0, 0, 10);
	}else if(Cmd == 2){
		SendPF48(port);	//发送获取PWM报文
		OSTimeDlyHMSM (0, 0, 0, 10);
	
	}
	switch (ChgCtl[port].State){
		case 0: //待机
			ChgCtl[port].info.ChargeState = 0;
		
			//当充电控制板的状态不是空闲时，定时向充电控制板发送停止充电命令，以保证状态一致
			if ((ChgCtl[port].CCBState != 0) && (TimerRead() - StopTimer[port] > T100MS * 5)){
				StopTimer[port] = TimerRead();
				SendPF42(port);
			}
			
			//接收到启动充电命令
			if (StartFlag[port] == 1){
				StopFlag[port] = 0;
				ChgCtl[port].State++;
				ChgCtl[port].RecvCCB_PF40_Flag = 0;
				ChgCtl[port].RecvCCB_PF41_Flag = 0;
				ChgCtl[port].RecvCCB_PF42_Flag = 0;
				ChgCtl[port].RecvCCB_PF43_Flag = 0;
				StartTimer[port] = TimerRead();
				Timer[port] = TimerRead();
				SendPF40(port);
			}
			break;
			
		case 1: //启动
			ChgCtl[port].info.ChargeState = 0;
			
			//未接收到充电控制板应答启动充电命令时，定时发送启动充电命令
			StartFlag[port] = 0;
			if ((ChgCtl[port].RecvCCB_PF40_Flag == 0) && (TimerRead() - Timer[port] > T100MS * 5)){
				Timer[port] = TimerRead();
				SendPF40(port);
			}
			
			//当启动过程累计时间超过60秒时，跳转到待机状态
			if (TimerRead() - StartTimer[port] > T1S * 60){
				SendPF42(port);
				StartRet[port] = 2;
				ChgCtl[port].State = 0;
			}
			
			//当接收到充电控制板上报的启动完成上报帧时，根据启动结果跳转
			if (ChgCtl[port].RecvCCB_PF41_Flag == 1){
				if (ChgCtl[port].StartResult == 0){//启动成功
					ChgCtl[port].State++;
					Timer[port] = TimerRead();
					print("c1----\r\n");
				} else {//启动失败
					SendPF42(port);
					StartRet[port] = 2;
					ChgCtl[port].State = 0;
					print("c2----\r\n");
				}
			}
			
			//启动过程中接收到停止命令时，立即停止
			if (StopFlag[port] == 1){
				ChgCtl[port].State = 3;
				StartRet[port] = 2;
				StopRet[port] = 1;
			}
			break;
			
		case 2: //充电
//			print("c3----\r\n");
			ChgCtl[port].info.ChargeState = 1;
			StartRet[port] = 1;
		
			//充电过程中接收到停止命令时，下发停止
			if (StopFlag[port] == 1){
				ChgCtl[port].State++;
				StopRet[port] = 1;
			}		
			//当充电时间超过5秒后收到充电控制板上报的状态不是充电中状态时，下发停止
			if((ChgCtl[port].CCBState != 4) && (TimerRead() - Timer[port] > T1S * 5)){
				SendPF42(port);
				Timer[port] = TimerRead();
				ChgCtl[port].State++;
			}
			break;
			
		case 3: //停止		
			//定时发送停止充电帧
			ChgCtl[port].info.ChargeState = 1;
			StopFlag[port] = 0;
			if ((ChgCtl[port].RecvCCB_PF42_Flag == 0) && (TimerRead() - StopTimer[port] > T100MS * 5)){
				StopTimer[port] = TimerRead();
				SendPF42(port);
			}
			
			//当收到充电控制板上报的停止充电应答帧时，回待机状态
			if ((ChgCtl[port].RecvCCB_PF43_Flag == 1) || ((TimerRead() - Timer[port] > T1S * 10))){
				ChgCtl[port].State = 0;
			}	
			break;
			
		default:
			print("ChgCtl[port].State ERROR!\r\n");
			break;
	}
	
	//定时发送遥测
	if (TimerRead() - YaoCeTimer[port] > T100MS * 10){
		YaoCeTimer[port] = TimerRead();
		SendPF46(port);
	}
	
	//判断与充电控制板通信超时程序段
	if (ChgCtl[port].RecvCCB_PF44_Flag == 0){
		if (TimerRead() - CommTimer[port] > T1S * 10){
			ChgCtl[port].info.CommunicateState = 1;
		}
	} else {
		ChgCtl[port].RecvCCB_PF44_Flag = 0;
		CommTimer[port] = TimerRead();
		ChgCtl[port].info.CommunicateState = 0;
	}
	
	//分析从充电控制板接收到的消息
	AnalyseMsgFromCCB();
}


/************************************************************************************************************
** 函 数 名 : GetStopCause
** 功能描述 : 获取充电停止原因
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
uint8_t GetEXT_StopCause(uint8_t port)
{
	return ChgCtl[port].CCBStopCause;
}


/************************************************************************************************************
** 函 数 名 : GetCCBInfo
** 功能描述 : 获取CCB信息
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
CHARGEINFO GetEXT_CCBInfo(uint8_t port)
{
	return ChgCtl[port].info;
}

/************************************************************************************************************
** 函 数 名 : StartCharge
** 功能描述 : 启动充电函数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	结果 0成功 1失败
*************************************************************************************************************
*/
uint8_t EXT_StartCharge(uint8_t port)
{
	StartRet[port] = 0;
	StartFlag[port] = 1;
	while (StartFlag[port] == 1)
		OSTimeDlyHMSM (0, 0, 0, 100);
	
	while (1){
		if (StartRet[port] == 1)
			return 0;
		if (StartRet[port] == 2)
			return 1;
		OSTimeDlyHMSM (0, 0, 0, 100);
	}
}

/************************************************************************************************************
** 函 数 名 : StopCharge
** 功能描述 : 停止充电函数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪...... 
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
uint8_t EXT_StopCharge(uint8_t port)
{
	uint32_t timer;
	
	StopRet[port] = 0;
	StopFlag[port] = 1;
	while (StopFlag[port] == 1)
		OSTimeDlyHMSM (0, 0, 0, 100);
	
	timer = TimerRead();
	while(TimerRead() - timer < T1S * 10){
		if (StopRet[port] == 1)
			break;
		OSTimeDlyHMSM (0, 0, 0, 100);
	}
	
	return 1;
}

///************************************************************************************************************
//** 函 数 名 : ChgTask
//** 功能描述 : 充电任务
//** 输    入 : pdata 未使用
//** 输    出 : 无
//** 返    回 :	无
//*************************************************************************************************************
//*/
//void ChgTask(void *pdata)
//{
//	uint8_t port, PortNum = ParaGetChgGunNumber();
//	
//	while(1) {
//		for (port = 0; port < PortNum; port++){
//			
//				SendPF47(port);
//				
//			ChgHandle(port);
//			OSTimeDlyHMSM (0, 0, 0, 10);
//		}
//	}
//}

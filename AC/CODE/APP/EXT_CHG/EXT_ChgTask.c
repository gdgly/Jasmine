#include "EXT_ChgTask.h"
static struct{
	CHARGEINFO info;
	uint8_t CCBState; //�����ư�״̬
	uint8_t State; //״̬
	uint8_t StartResult;//������� 0�ɹ� 1ʧ��
	uint8_t StartFailCause;//����ʧ��ԭ��
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
** �� �� �� : ReadPwm
** �������� : ��ȡ���巢�͵�PWM
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	���巢�͵�PWM
*************************************************************************************************************
*/
uint16_t ReadPwm(uint8_t port)
{
	return Readpwm[port];
}

/************************************************************************************************************
** �� �� �� : SendDataToCCB
** �������� : �������ݵ������ư庯��
** ��    �� : canid��CAN��ID��
							pdata��Ҫ���͵����ݻ�������ַ
							len��Ҫ���͵��ֽ���
** ��    �� : ��
** ��    �� :	У���
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
** �� �� �� : SendPF40
** �������� : ���ͼƷѿ��ư��������֡
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
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
** �� �� �� : SendPF41
** �������� : ���ͼƷѿ��ư�Ӧ������ư��ϱ�������������֡
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
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
** �� �� �� : SendPF42
** �������� : ���ͼƷѿ��ư�ֹͣ���֡
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void SendPF42(uint8_t port)
{
	uint32_t id=0x104200a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
	SendDataToCCB(id, data, 8);
}

/************************************************************************************************************
** �� �� �� : SendPF43
** �������� : ���ͼƷѿ��ư�Ӧ������ư��ϱ���ֹͣ������֡
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
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
** �� �� �� : SendPF46
** �������� : ���ͼƷѿ��ư�ң��֡
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
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
	data16 = 0;//���
	data[12] = data16 & 0xff;
	data[13] = (data16 >> 8) & 0xff;
	data16 = 0;//���
	data[14] = data16 & 0xff;
	data[15] = (data16 >> 8) & 0xff;
	
	MultiFramePackAndSend(id, data, 16);
}

/************************************************************************************************************
** �� �� �� : SendPF47
** �������� : ���ͼƷѿ��ư�����PWM ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
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
** �� �� �� : SendPF48
** �������� : ���ͼƷѿ��ư�����PWM ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void SendPF48(uint8_t port)
{
	uint32_t id=0x104800a0 | (port << 8);
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	MultiFramePackAndSend(id, data, 8);
}

/************************************************************************************************************
** �� �� �� : AnalyseMsgFromCCB
** �������� : ���������ڳ����ư������
** ��    �� : ��
** ��    �� : ��
** ��    �� :	��
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
				
				case 0x1040://�������Ӧ��֡
					ChgCtl[port].RecvCCB_PF40_Flag = 1;
					break;
				
				case 0x1041://�������֡
					ChgCtl[port].StartResult = data[0];
					ChgCtl[port].StartFailCause = data[1];
					SendPF41(port);
					ChgCtl[port].RecvCCB_PF41_Flag = 1;
					break;
				
				case 0x1042://ֹͣ���Ӧ��֡
					ChgCtl[port].RecvCCB_PF42_Flag = 1;
					break;
				
				case 0x1043://ֹͣ���֡
					SendPF43(port);
					ChgCtl[port].CCBStopCause = data[1];
					ChgCtl[port].RecvCCB_PF43_Flag = 1;
					break;
				
				case 0x1844://ң��
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
					if (data[0] & (1 << 3))  //��ѹ
						ChgCtl[port].info.HighVol = 1;
					else
						ChgCtl[port].info.HighVol = 0;
					if (data[0] & (1 << 4))  //Ƿѹ
						ChgCtl[port].info.LowVol = 1;
					else
						ChgCtl[port].info.LowVol = 0;
					ChgCtl[port].RecvCCB_PF44_Flag = 1;
					break;
					
				case 0x1845://ң��
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
				case 0x1047://����PWM�ص�����		
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
** �� �� �� : ChgHandle
** �������� : ��紦����
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ChgHandle(uint8_t port)
{
	static uint32_t Timer[2],StartTimer[2],StopTimer[2],YaoCeTimer[2],CommTimer[2];
	uint8_t Cmd = 0;
	Cmd = GetParaCmd(port);
	if(Cmd ==1){		
		SendPF47(port);//��������PWM����
		OSTimeDlyHMSM (0, 0, 0, 10);
	}else if(Cmd == 2){
		SendPF48(port);	//���ͻ�ȡPWM����
		OSTimeDlyHMSM (0, 0, 0, 10);
	
	}
	switch (ChgCtl[port].State){
		case 0: //����
			ChgCtl[port].info.ChargeState = 0;
		
			//�������ư��״̬���ǿ���ʱ����ʱ������ư巢��ֹͣ�������Ա�֤״̬һ��
			if ((ChgCtl[port].CCBState != 0) && (TimerRead() - StopTimer[port] > T100MS * 5)){
				StopTimer[port] = TimerRead();
				SendPF42(port);
			}
			
			//���յ������������
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
			
		case 1: //����
			ChgCtl[port].info.ChargeState = 0;
			
			//δ���յ������ư�Ӧ�������������ʱ����ʱ���������������
			StartFlag[port] = 0;
			if ((ChgCtl[port].RecvCCB_PF40_Flag == 0) && (TimerRead() - Timer[port] > T100MS * 5)){
				Timer[port] = TimerRead();
				SendPF40(port);
			}
			
			//�����������ۼ�ʱ�䳬��60��ʱ����ת������״̬
			if (TimerRead() - StartTimer[port] > T1S * 60){
				SendPF42(port);
				StartRet[port] = 2;
				ChgCtl[port].State = 0;
			}
			
			//�����յ������ư��ϱ�����������ϱ�֡ʱ���������������ת
			if (ChgCtl[port].RecvCCB_PF41_Flag == 1){
				if (ChgCtl[port].StartResult == 0){//�����ɹ�
					ChgCtl[port].State++;
					Timer[port] = TimerRead();
					print("c1----\r\n");
				} else {//����ʧ��
					SendPF42(port);
					StartRet[port] = 2;
					ChgCtl[port].State = 0;
					print("c2----\r\n");
				}
			}
			
			//���������н��յ�ֹͣ����ʱ������ֹͣ
			if (StopFlag[port] == 1){
				ChgCtl[port].State = 3;
				StartRet[port] = 2;
				StopRet[port] = 1;
			}
			break;
			
		case 2: //���
//			print("c3----\r\n");
			ChgCtl[port].info.ChargeState = 1;
			StartRet[port] = 1;
		
			//�������н��յ�ֹͣ����ʱ���·�ֹͣ
			if (StopFlag[port] == 1){
				ChgCtl[port].State++;
				StopRet[port] = 1;
			}		
			//�����ʱ�䳬��5����յ������ư��ϱ���״̬���ǳ����״̬ʱ���·�ֹͣ
			if((ChgCtl[port].CCBState != 4) && (TimerRead() - Timer[port] > T1S * 5)){
				SendPF42(port);
				Timer[port] = TimerRead();
				ChgCtl[port].State++;
			}
			break;
			
		case 3: //ֹͣ		
			//��ʱ����ֹͣ���֡
			ChgCtl[port].info.ChargeState = 1;
			StopFlag[port] = 0;
			if ((ChgCtl[port].RecvCCB_PF42_Flag == 0) && (TimerRead() - StopTimer[port] > T100MS * 5)){
				StopTimer[port] = TimerRead();
				SendPF42(port);
			}
			
			//���յ������ư��ϱ���ֹͣ���Ӧ��֡ʱ���ش���״̬
			if ((ChgCtl[port].RecvCCB_PF43_Flag == 1) || ((TimerRead() - Timer[port] > T1S * 10))){
				ChgCtl[port].State = 0;
			}	
			break;
			
		default:
			print("ChgCtl[port].State ERROR!\r\n");
			break;
	}
	
	//��ʱ����ң��
	if (TimerRead() - YaoCeTimer[port] > T100MS * 10){
		YaoCeTimer[port] = TimerRead();
		SendPF46(port);
	}
	
	//�ж�������ư�ͨ�ų�ʱ�����
	if (ChgCtl[port].RecvCCB_PF44_Flag == 0){
		if (TimerRead() - CommTimer[port] > T1S * 10){
			ChgCtl[port].info.CommunicateState = 1;
		}
	} else {
		ChgCtl[port].RecvCCB_PF44_Flag = 0;
		CommTimer[port] = TimerRead();
		ChgCtl[port].info.CommunicateState = 0;
	}
	
	//�����ӳ����ư���յ�����Ϣ
	AnalyseMsgFromCCB();
}


/************************************************************************************************************
** �� �� �� : GetStopCause
** �������� : ��ȡ���ֹͣԭ��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
uint8_t GetEXT_StopCause(uint8_t port)
{
	return ChgCtl[port].CCBStopCause;
}


/************************************************************************************************************
** �� �� �� : GetCCBInfo
** �������� : ��ȡCCB��Ϣ
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
CHARGEINFO GetEXT_CCBInfo(uint8_t port)
{
	return ChgCtl[port].info;
}

/************************************************************************************************************
** �� �� �� : StartCharge
** �������� : ������纯��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��� 0�ɹ� 1ʧ��
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
** �� �� �� : StopCharge
** �������� : ֹͣ��纯��
** ��    �� : port ���ӿ� =0����1��ǹ =1����2��ǹ...... 
** ��    �� : ��
** ��    �� :	��
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
//** �� �� �� : ChgTask
//** �������� : �������
//** ��    �� : pdata δʹ��
//** ��    �� : ��
//** ��    �� :	��
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

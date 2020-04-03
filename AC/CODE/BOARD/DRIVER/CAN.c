#include 	<LPC177x_8x.H>
#include "Board.h"
#include "CAN.h"

static CANBUF CanBuf[2];

/************************************************************************************************************
** 函 数 名 : CANInit
** 功能描述 : CAN初始化函数
** 输    入 : no AD号 clk 速率
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void CANInit(uint8_t no, uint32_t clk)
{
	uint16_t i;
	uint32_t result = 0;
	uint8_t NT, TSEG1, TSEG2;
	uint32_t BRP;
	
	if(no == DEV_CAN1){
		LPC_SC->PCONP   |= 0x01L<<13;
		LPC_IOCON->P0_21 &= ~0x07;
		LPC_IOCON->P0_21 |= 0x04;
		LPC_IOCON->P0_22 &= ~0x07;
		LPC_IOCON->P0_22 |= 0x04;
		LPC_CAN1->MOD = 1;
		LPC_CAN1->IER = 0;
		LPC_CAN1->GSR = 0;
		LPC_CAN1->CMR = (1 << 1) | (1 << 2) | (1 << 3);
		i = LPC_CAN1->ICR;
		LPC_CAN1->MOD = 0;
		LPC_CANAF->AFMR = 0x01;
		for (i = 0; i < 512; i++){
			LPC_CANAF_RAM->mask[i] = 0x00;
		}
		LPC_CANAF->SFF_sa = 0x00;
		LPC_CANAF->SFF_GRP_sa = 0x00;
		LPC_CANAF->EFF_sa = 0x00;
		LPC_CANAF->EFF_GRP_sa = 0x00;
		LPC_CANAF->ENDofTable = 0x00;
		LPC_CANAF->AFMR = 0x02;
		result = PeripheralClock / clk;
		for(NT = 24; NT > 0; NT = NT-2){
			if ((result%NT) == 0){
				BRP = result / NT - 1;
				NT--;
				TSEG2 = (NT/3) - 1;
				TSEG1 = NT -(NT/3) - 1;
				break;
			}
		}
		LPC_CAN1->MOD = 0x01;
		LPC_CAN1->BTR = (TSEG2 << 20) | (TSEG1 << 16) | (3 << 14) | BRP;
		LPC_CAN1->MOD = 0;
		LPC_CAN1->IER |= (1<<0);
		NVIC_EnableIRQ(CAN_IRQn);
	}
	
	if(no == DEV_CAN2){
		LPC_SC->PCONP   |= 0x01L<<14;
		LPC_IOCON->P2_7 &= ~0x07;
		LPC_IOCON->P2_7 |= 0x01;
		LPC_IOCON->P2_8 &= ~0x07;
		LPC_IOCON->P2_8 |= 0x01;
		LPC_CAN2->MOD = 1;
		LPC_CAN2->IER = 0;
		LPC_CAN2->GSR = 0;
		LPC_CAN2->CMR = (1 << 1) | (1 << 2) | (1 << 3);
		i = LPC_CAN2->ICR;
		LPC_CAN2->MOD = 0;
		LPC_CANAF->AFMR = 0x01;
		for (i = 0; i < 512; i++){
			LPC_CANAF_RAM->mask[i] = 0x00;
		}
		LPC_CANAF->SFF_sa = 0x00;
		LPC_CANAF->SFF_GRP_sa = 0x00;
		LPC_CANAF->EFF_sa = 0x00;
		LPC_CANAF->EFF_GRP_sa = 0x00;
		LPC_CANAF->ENDofTable = 0x00;
		LPC_CANAF->AFMR = 0x02;
		result = PeripheralClock / clk;
		for(NT = 24; NT > 0; NT = NT-2){
			if ((result%NT) == 0){
				BRP = result / NT - 1;
				NT--;
				TSEG2 = (NT/3) - 1;
				TSEG1 = NT -(NT/3) - 1;
				break;
			}
		}
		LPC_CAN2->MOD = 0x01;
		LPC_CAN2->BTR = (TSEG2 << 20) | (TSEG1 << 16) | (3 << 14) | BRP;
		LPC_CAN2->MOD = 0;
		LPC_CAN2->IER |= (1<<0) | (1<<7);
		NVIC_EnableIRQ(CAN_IRQn);
	}
}

/************************************************************************************************************
** 函 数 名 : CAN_IRQHandler
** 功能描述 : CAN中断服务函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void CAN_IRQHandler(void)
{
	uint8_t IntStatus;
	uint32_t data;
	CAN_MSG_Type msg;
	
	IntStatus=LPC_CAN1->ICR;
	if((IntStatus >> 0) & 0x01){
		if((LPC_CAN1->SR &0x00000001)){
			msg.format = (uint8_t)(((LPC_CAN1->RFS) & 0x80000000) >> 31);
			msg.type = (uint8_t)(((LPC_CAN1->RFS) & 0x40000000) >> 30);
			msg.len = (uint8_t)(((LPC_CAN1->RFS) & 0x000F0000) >> 16);
			msg.id = LPC_CAN1->RID;
			if (msg.type == DATA_FRAME){
				data = LPC_CAN1->RDA;
				*((uint8_t *) &msg.dataA[0])= data & 0x000000FF;
				*((uint8_t *) &msg.dataA[1])= (data & 0x0000FF00) >> 8;;
				*((uint8_t *) &msg.dataA[2])= (data & 0x00FF0000) >> 16;
				*((uint8_t *) &msg.dataA[3])= (data & 0xFF000000) >> 24;
				data = LPC_CAN1->RDB;
				*((uint8_t *) &msg.dataB[0])= data & 0x000000FF;
				*((uint8_t *) &msg.dataB[1])= (data & 0x0000FF00) >> 8;
				*((uint8_t *) &msg.dataB[2])= (data & 0x00FF0000) >> 16;
				*((uint8_t *) &msg.dataB[3])= (data & 0xFF000000) >> 24;
			}
			LPC_CAN1->CMR = 0x04;
			CanBuf[0].Msg[CanBuf[0].Wr]=msg;
			CanBuf[0].Wr=(CanBuf[0].Wr+1)%CAN_MSG_NUMBER;
		}
	}
//	IntStatus=LPC_CAN2->ICR;
//	if((IntStatus >> 0) & 0x01){
//		if((LPC_CAN2->SR &0x00000001)){
//			msg.format = (uint8_t)(((LPC_CAN2->RFS) & 0x80000000) >> 31);
//			msg.type = (uint8_t)(((LPC_CAN2->RFS) & 0x40000000) >> 30);
//			msg.len = (uint8_t)(((LPC_CAN2->RFS) & 0x000F0000) >> 16);
//			msg.id = LPC_CAN2->RID;
//			if (msg.type == DATA_FRAME){
//				data = LPC_CAN2->RDA;
//				*((uint8_t *) &msg.dataA[0])= data & 0x000000FF;
//				*((uint8_t *) &msg.dataA[1])= (data & 0x0000FF00) >> 8;;
//				*((uint8_t *) &msg.dataA[2])= (data & 0x00FF0000) >> 16;
//				*((uint8_t *) &msg.dataA[3])= (data & 0xFF000000) >> 24;
//				data = LPC_CAN2->RDB;
//				*((uint8_t *) &msg.dataB[0])= data & 0x000000FF;
//				*((uint8_t *) &msg.dataB[1])= (data & 0x0000FF00) >> 8;
//				*((uint8_t *) &msg.dataB[2])= (data & 0x00FF0000) >> 16;
//				*((uint8_t *) &msg.dataB[3])= (data & 0xFF000000) >> 24;
//			}
//			LPC_CAN2->CMR = 0x04;
//			CanBuf[1].Msg[CanBuf[1].Wr]=msg;
//			CanBuf[1].Wr=(CanBuf[1].Wr+1)%CAN_MSG_NUMBER;
//		}
//	}
}

/************************************************************************************************************
** 函 数 名 : CANWrite
** 功能描述 : CAN写函数
** 输    入 : no AD号 canmsg 消息
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
uint8_t CANWrite(unsigned char port,uint32_t id,uint8_t data[8],uint8_t len)
{
	LPC_CAN_TypeDef *pCan;
	uint32_t data32;
	CAN_MSG_Type *CAN_Msg,msg;
	uint8_t i;
	
	msg.id=id;
	msg.len=len;
	msg.type=DATA_FRAME;
	msg.format=EXT_ID_FORMAT;
	for(i=0;i<4;i++){
		if(len){
			len--;
			msg.dataA[i]=*data++;
		}else{
			msg.dataA[i]=0;
		}
	}
	for(i=0;i<4;i++){
		if(len){
			len--;
			msg.dataB[i]=*data++;
		}else{
			msg.dataB[i]=0;
		}
	}
	CAN_Msg = &msg;

	if(port == 0){
		pCan=LPC_CAN1;
	}else if(port == 1){
		pCan=LPC_CAN2;
	}else{
		return 0;
	}
	if (pCan->SR & (1 << 2)){
		pCan->TFI1 &= ~ 0x000F0000;
		pCan->TFI1 |= (CAN_Msg->len) << 16;
		if(CAN_Msg->type == REMOTE_FRAME){
			pCan->TFI1 |= (1 << 30);
		}else{
			pCan->TFI1 &= ~(1 << 30);
		}
		if(CAN_Msg->format == EXT_ID_FORMAT){
			pCan->TFI1 |= (((uint32_t)1) << 31);
		}else{
			pCan->TFI1 &= ~(((uint32_t)1) << 31);
		}
		pCan->TID1 = CAN_Msg->id;
		data32 = (CAN_Msg->dataA[0]) | (((CAN_Msg->dataA[1]))<< 8) | ((CAN_Msg->dataA[2]) << 16) | ((CAN_Msg->dataA[3]) << 24);
		pCan->TDA1 = data32;
		data32 = (CAN_Msg->dataB[0]) | (((CAN_Msg->dataB[1])) << 8)|((CAN_Msg->dataB[2]) << 16)|((CAN_Msg->dataB[3]) << 24);
		pCan->TDB1 = data32;
		pCan->CMR = 0x21;
		return CAN_Msg->len;
	}else if((pCan->SR) & (1 << 10)){
		pCan->TFI2 &= ~0x000F0000;
		pCan->TFI2 |= (CAN_Msg->len) << 16;
		if(CAN_Msg->type == REMOTE_FRAME){
			pCan->TFI2 |= (1 << 30);
		}else{
			pCan->TFI2 &= ~(1 << 30);
		}
		if(CAN_Msg->format == EXT_ID_FORMAT){
			pCan->TFI2 |= (((uint32_t)1) << 31);
		}else{
			pCan->TFI2 &= ~(((uint32_t)1) << 31);
		}
		pCan->TID2 = CAN_Msg->id;
		data32 = (CAN_Msg->dataA[0]) | (((CAN_Msg->dataA[1])) << 8) | ((CAN_Msg->dataA[2]) << 16)|((CAN_Msg->dataA[3]) << 24);
		pCan->TDA2 = data32;
		data32 = (CAN_Msg->dataB[0]) | (((CAN_Msg->dataB[1])) << 8) | ((CAN_Msg->dataB[2]) << 16) | ((CAN_Msg->dataB[3]) << 24);
		pCan->TDB2 = data32;
		pCan->CMR = 0x41;
		return CAN_Msg->len;
	}else if (pCan->SR & (1<<18)){
		pCan->TFI3 &= ~0x000F0000;
		pCan->TFI3 |= (CAN_Msg->len) << 16;
		if(CAN_Msg->type == REMOTE_FRAME){
			pCan->TFI3 |= (1 << 30);
		}else{
			pCan->TFI3 &= ~(1 << 30);
		}
		if(CAN_Msg->format == EXT_ID_FORMAT){
			pCan->TFI3 |= (((uint32_t)1) << 31);
		}else{
			pCan->TFI3 &= ~(((uint32_t)1) << 31);
		}
		pCan->TID3 = CAN_Msg->id;
		data32 = (CAN_Msg->dataA[0]) | (((CAN_Msg->dataA[1])) << 8) | ((CAN_Msg->dataA[2]) << 16) | ((CAN_Msg->dataA[3]) << 24);
		pCan->TDA3 = data32;
		data32 = (CAN_Msg->dataB[0]) | (((CAN_Msg->dataB[1])) << 8) | ((CAN_Msg->dataB[2]) << 16) | ((CAN_Msg->dataB[3]) << 24);
		pCan->TDB3 = data32;
		pCan->CMR = 0x81;
		return CAN_Msg->len;
	}else{
		return 0;
	}
}

/************************************************************************************************************
** 函 数 名 : CANRead
** 功能描述 : CAN读函数
** 输    入 : port 充电接口 =0代表1号枪 =1代表2号枪......
** 输    出 : id CAN数据的ID 扩展帧
							data CAN数据
** 返    回 :	CAN数据长度
*************************************************************************************************************
*/
uint8_t CANRead(unsigned char port,uint32_t *id,uint8_t data[8])
{
	CAN_MSG_Type msg;
	CANBUF *pcanbuf;
	uint8_t i;

	*id = 0;
	msg.len = 0;
	if(port==0){
		pcanbuf=&CanBuf[0];
	}else if(port==1){
		pcanbuf=&CanBuf[1];
	}else{
    return 0;
	}
	if(pcanbuf->Rd != pcanbuf->Wr){
		msg=pcanbuf->Msg[pcanbuf->Rd];
		pcanbuf->Rd=(pcanbuf->Rd+1)%CAN_MSG_NUMBER;
		*id = msg.id;
		for(i=0; i<msg.len; i++){
			if(i < 4){
				data[i] = msg.dataA[i];
			}else if(i < 8){
				data[i] = msg.dataB[i - 4];
			}else{
				break;
			}
		}
	}
	
	return msg.len;
}

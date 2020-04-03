/***********************************************************************************************************
* brief: Charge Interface
* create date: 2018-10-11
* create author: YuL
************************************************************************************************************
* modify date: xxxx-xx-xx
* modify author: xxxxxx
* modify note: xx
************************************************************************************************************
* modify date: xxxx-xx-xx
* modify author: xxxxxx
* modify note: xx
************************************************************************************************************/
#include "includes.h"
#include "ChgInterface.h"
#include "ChgOnebodyTask.h" 
#include "EXT_ChgTask.h"
#include "Cfg.h"
/************************************************************************************************************
** �� �� �� : StartCharge
** �������� : ������纯���ӿں���
** ��    �� : ��
** ��    �� : ��
** ��    �� : 1 ==�������ɹ�   0 == �������ʧ��
*************************************************************************************************************
*/
uint8_t StartCharge(uint8_t port)
{
#if EXT_BOARD
	return EXT_StartCharge(port);
#else
	return StartOnebodyChg(port);
#endif
	
}

/************************************************************************************************************
** �� �� �� : StopCharge
** �������� : ֹͣ���ӿں���
** ��    �� : ��
** ��    �� : ��
** ��    �� : 1 == ֹͣ���ɹ�   0 == ֹͣ���ʧ��
*************************************************************************************************************
*/
uint8_t StopCharge(uint8_t port)
{
#if EXT_BOARD
	return EXT_StopCharge(port);
#else
	return StopOnebodyChg(port);
#endif
	
	
	
}

/************************************************************************************************************
** �� �� �� : GetChgInfo
** �������� : ��ȡ�����Ϣ�ӿں���
** ��    �� : port 0 --- Aǹ  1 --- Bǹ
** ��    �� : ��
** ��    �� : CHARGEINFO �����Ϣ�ṹ��
*************************************************************************************************************
*/

CHARGEINFO GetChgInfo(uint8_t port)
{	
#if EXT_BOARD
	return GetEXT_CCBInfo(port);
#else
	return GetChgOnebodyInfo(port);
#endif
	
}



/************************************************************************************************************
** �� �� �� : GetStopCause
** �������� : ��ȡ���ֹͣԭ��ӿں���
** ��    �� : ��
** ��    �� : ��
** ��    �� : 1 ==�������ɹ�   0 == �������ʧ��
*************************************************************************************************************
*/
uint8_t GetStopCause(uint8_t port)
{
#if EXT_BOARD
	return GetEXT_StopCause(port);
#else
	return GetOnebodyStopCause(port);
#endif
}




/************************************************************************************************************
** �� �� �� : ChargeTask
** �������� : ��紦������
** ��    �� : ��
** ��    �� : ��
** ��    �� : ��
*************************************************************************************************************
*/
void ChargeTask(void * pdata)
{
	uint8_t port, PortNum; 
	PortNum = ParaGetChgGunNumber();	
#if EXT_BOARD
	//  EXT���Ӧ��ʼ��
#else
	ADCInit(ADC_CLK);
	Swich_ModeIn();
	E_Unlock();
	STOPIN();
	TEST_LOCKIN();
	TEST_CCIn();
	PwmInit(HIVCNT);
#endif
	
	while(1) {
		for (port = 0; port < PortNum; port++){		
#if EXT_BOARD
			ChgHandle(port);
#else
			ChgOnebodyPro(0);
#endif
			OSTimeDlyHMSM (0, 0, 0, 10);
		}
	}
}


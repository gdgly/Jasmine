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
** 函 数 名 : StartCharge
** 功能描述 : 启动充电函数接口函数
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1 ==启动充电成功   0 == 启动充电失败
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
** 函 数 名 : StopCharge
** 功能描述 : 停止充电接口函数
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1 == 停止充电成功   0 == 停止充电失败
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
** 函 数 名 : GetChgInfo
** 功能描述 : 获取充电信息接口函数
** 输    入 : port 0 --- A枪  1 --- B枪
** 输    出 : 无
** 返    回 : CHARGEINFO 充电信息结构体
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
** 函 数 名 : GetStopCause
** 功能描述 : 获取充电停止原因接口函数
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1 ==启动充电成功   0 == 启动充电失败
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
** 函 数 名 : ChargeTask
** 功能描述 : 充电处理任务
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void ChargeTask(void * pdata)
{
	uint8_t port, PortNum; 
	PortNum = ParaGetChgGunNumber();	
#if EXT_BOARD
	//  EXT相对应初始化
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


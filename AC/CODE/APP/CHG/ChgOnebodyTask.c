#include "includes.h"
#include "ChgInterface.h"
#include "ChgOnebodyTask.h"
#include "Timer.h"
#include "Board.h"
#include "ElmTask.h"
#include "UART.h"
#include "Para.h"
static uint8_t StopCause[2]; 
static CHARGEINFO ChargeInfo;
static uint16_t Pwm;
static uint32_t Timer_MinCur;


/************************************************************************************************************
** 函 数 名 : GetErrCur
** 功能描述 : 获取是否过流
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1   过流   0  正常
*************************************************************************************************************
*/
static uint8_t GetErrCur(void)
{
	static uint32_t LimitCur;
	uint16_t pwm = ParaGetPwm(0);
	
	if(LimitCur == 0){
		if( 1000 <= pwm && pwm<= 8500){
			LimitCur = pwm * 6;
		}else if(pwm >8500 && pwm <= 9000 ){
			LimitCur = (pwm - 6400) * 25;
			if(LimitCur > 63000)
				LimitCur = 63000;
		}
		
		if(LimitCur <= 20000)
			LimitCur = LimitCur + 2000;
		else
			LimitCur = LimitCur * 110 /100;
	}
	
	if(ElmGetMaxCur(0) > LimitCur)
		return 1;
	else
		return 0;
}


/************************************************************************************************************
** 函 数 名 : ChgOnebodyPro
** 功能描述 : 一体式充电处理函数
** 输    入 : port     0  ---- A枪      1 ---- B枪
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
void ChgOnebodyPro(uint8_t port)
{
	static uint32_t Timer_Cnt1 = 0;
	static uint32_t Timer_Cnt2 = 0;

	static uint8_t IsComplete = 0;
	static uint8_t Cnt9V = 0;
	uint32_t Timer_Cnt3 = 0;
	uint8_t LockFlag = 0;
	uint8_t ChargeState;
	ADCScan();
	ChargeState = ChargeInfo.ChargeState;
	if((Get_AdcStatus(port) == ADC9V || Get_AdcStatus(port) == ADC6V) && Test_CC(NONE) ){	
		ChargeInfo.GunSeatState = 1;
	}else{
		ChargeInfo.GunSeatState = 0;
	}	
	if(TestStop() == 0){   //急停故障
		ChargeInfo.ScramState = 0;
	}else{
		ChargeInfo.ScramState = 1;
		StopCause[port] = CAUSE_SCRAMSTOP_FAULT;
		ChargeInfo.ChargeState = 0;
	}
	if(Test_Door() == 0){   //门禁
		ChargeInfo.DoorState = 0;
	}else{
		ChargeInfo.DoorState = 1;
		StopCause[port] = CAUSE_DOOR_OPEN_FAULT; //门禁故障
		ChargeInfo.ChargeState = 0;
	}
	if( GetTemp(port) == 1){
		ChargeInfo.ErrTemp = 1;
		StopCause[port] = CAUSE_GUNPORT_OVERTEMP;  //充电接口过温
		ChargeInfo.ChargeState = 0;
	}else{
		ChargeInfo.ErrTemp = 0;
	}
	if(ElmGetMaxVol(0) > 2420){  //过压
		ChargeInfo.HighVol = 1;
		StopCause[port] = CAUSE_AC_OVERVOL_FAULT;
		ChargeInfo.ChargeState = 0;
	}else{
		ChargeInfo.HighVol = 0;
	}
	if(ElmGetMaxVol(0) < 1980){
		ChargeInfo.LowVol = 1;
		StopCause[port] = CAUSE_AC_LESSVOL_FAULT;
		ChargeInfo.ChargeState = 0;
	}else{
		ChargeInfo.LowVol = 0;
	}
	ChargeInfo.CurA = ElmGetCurA(0);
	ChargeInfo.VolA = ElmGetVolA(0);
	if( GetErrCur()){  //判断过流
		if(Timer_Cnt1 == 0)
			Timer_Cnt1 = TimerRead();
		if((TimerRead() - Timer_Cnt1) >= T1S *5){
			StopCause[port] = CAUSE_AC_OVERCUR_FAULT;    //过流
			ChargeInfo.ChargeState = 0;
		}
	}else{
		Timer_Cnt1 = 0;
	}

	if( ChargeInfo.ChargeState ){    //充电时判断
		if(ElmGetMaxCur(0) < 2000){
			if( Timer_MinCur == 0)
				Timer_MinCur = TimerRead();
			if((TimerRead() - Timer_MinCur) >= T1M *2){
				Timer_MinCur = 0;
				StopCause[0] = CAUSE_AC_LESSCUR_FAULT;    //欠流
				ChargeInfo.ChargeState = 0;
			}
		}else{
				Timer_MinCur = 0;
		}
	
		if( Test_CC(NONE) == 0 ){
			StopCause[port] = CAUSE_CC_FAULT;          //CC 异常
			ChargeInfo.ChargeState = 0;
		}else if( ElmGetCommState(0)){   
			StopCause[port] = CAUSE_ELM_FAULT;         //电表异常
			ChargeInfo.ChargeState = 0;
		}
		if( Get_AdcStatus(port) == ADC0V || Get_AdcStatus(port) == ADC12V){    // CP异常
			PwmInit(HIVCNT);	
			OSTimeDlyHMSM(0, 0, 0, 40);
			AC_OFF();
			StopCause[port] = CAUSE_CP_FAULT;
			ChargeInfo.ChargeState = 0;
		}else if(Get_AdcStatus(port) == ADC6V){		   //充电中CP跳到 6V
			if(IsComplete){
				IsComplete = 0;
				E_Lock();              //电子锁上锁
				Timer_Cnt3 = TimerRead();												
				while(Test_Lock() == 1){
					OSTimeDlyHMSM(0, 0, 0, 10);
					if(TimerRead() - Timer_Cnt3 >= 200){
						StopCause[port] = CAUSE_LOCK_GUN_FAILED;
						ChargeInfo.ChargeState = 0;
						LockFlag = 1;
						break;
					}
				}
				if(LockFlag != 1 ){
					OSTimeDlyHMSM(0, 0, 2, 0); //控制3S以内
					Timer_MinCur = 0;
					AC_ON();	
				}
			}
		}else if(Get_AdcStatus(port) == ADC9V){		   //充电中CP跳到 9V		
			Timer_MinCur = TimerRead();
			if(IsComplete == 0){
				IsComplete = 1;
				OSTimeDlyHMSM(0, 0, 0, 40);
				AC_OFF();
				Cnt9V++;
				if(Cnt9V >= 4){
					StopCause[port] = CAUSE_PAUSE_TIMES_FAULT;
					ChargeInfo.ChargeState = 0;
				}
				OSTimeDlyHMSM(0, 0, 0, 100);
				E_Unlock();
				Timer_Cnt2 = TimerRead(); 
			}else{
				if(TimerRead() - Timer_Cnt2 >= T1M ){
					Timer_Cnt2 = 0;
					ChargeInfo.ChargeState = 0;
					StopCause[port] = CAUSE_SUSPEND_TIMEOUT; //充电过程中跳变9V超过1分钟认为 充电桩充电中暂停超时 
				}
			}
		}
	}
	if(ChargeState &&(ChargeInfo.ChargeState == 0)){
		PwmInit(HIVCNT);
		OSTimeDlyHMSM(0, 0, 0, 25);		
		AC_OFF();
		OSTimeDlyHMSM(0, 0, 0, 100);
		E_Unlock();
		Cnt9V = 0;
	}
}
	

/************************************************************************************************************
** 函 数 名 : StartOnebodyChg
** 功能描述 : 一体板启动充电函数
** 输    入 : 无
** 输    出 : 无
** 返    回 : 0 ==启动充电成功   1 == 启动充电失败
*************************************************************************************************************
*/
uint8_t StartOnebodyChg(uint8_t port)
{
	uint32_t CntTimer;
	Pwm = 100 - (ParaGetPwm(port)/100);
	if(TestStop())
		return 1;
	if(Test_Door())
		return 1;
	if(GetTemp(port))
		return 1;
	if(Test_CC(NONE) == 0)
		return 1;	
	if((Get_AdcStatus(port) == ADC9V || Get_AdcStatus(port) == ADC6V) && Test_CC(NONE) ){	
		E_Lock();
		CntTimer = TimerRead();												
		while(Test_Lock() == 1){
			OSTimeDlyHMSM(0, 0, 0, 10);
			if(TimerRead() - CntTimer >= T1S * 5){
				E_Unlock();
				StopCause[port] = CAUSE_LOCK_GUN_FAILED;
				return 1;
			}
		}	
		PwmInit(Pwm); 
		CntTimer = TimerRead();			
		while( Get_AdcStatus(port) != ADC6V){
			OSTimeDlyHMSM(0, 0, 0, 10);
			if((TimerRead() - CntTimer >= T1S*30) || (Get_AdcStatus(port) == ADC12V) || (Get_AdcStatus(port) == ADC0V)){
				E_Unlock();
				StopCause[port] = CAUSE_CP_FAULT;   //汽车3s内未打开 S2 开关
				PwmInit(HIVCNT); 
				return 1;
			}
		}
		OSTimeDlyHMSM(0, 0, 2, 0); //控制3S以内
		AC_ON();
		Timer_MinCur = 0;
		ChargeInfo.ChargeState = 1;
		return 0;
	}else {
		return 1;
	}	

}


/************************************************************************************************************
** 函 数 名 : StopOnebodyChg
** 功能描述 : 一体板停止充电函数
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1 == 停止充电成功   0 == 停止充电失败
*************************************************************************************************************
*/
uint8_t StopOnebodyChg(uint8_t port)
{
	uint32_t CntTimer;
	PwmInit(HIVCNT); 
	CntTimer = TimerRead();	
	Timer_MinCur = 0;
	while(Get_AdcStatus(port) == ADC6V){
		OSTimeDlyHMSM(0, 0, 0, 10);	
		if(TimerRead() - CntTimer >= T1S*3 ){							
			break;
		}
	}
	AC_OFF();
	OSTimeDlyHMSM(0, 0, 0, 100);
	E_Unlock();
	ChargeInfo.ChargeState = 0;
	StopCause[port] = CAUSE_USER_NORMAL_STOP;
	return 0;	
}

/************************************************************************************************************
** 函 数 名 : GetOnebodyStopCause
** 功能描述 : 一体式获取充电停止原因
** 输    入 : 无
** 输    出 : 无
** 返    回 : 1 ==启动充电成功   0 == 启动充电失败
*************************************************************************************************************
*/
uint8_t GetOnebodyStopCause(uint8_t port)
{
	return StopCause[port];
}



/************************************************************************************************************
** 函 数 名 : GetChgOnebodyInfo
** 功能描述 : 一体式获取充电信息
** 输    入 : 无
** 输    出 : 无
** 返    回 : CHARGEINFO  充电信息结构体
*************************************************************************************************************
*/
CHARGEINFO GetChgOnebodyInfo(uint8_t port)
{
	return ChargeInfo;
}












/************************************************************************************************************
** 函 数 名 : ChgOnebodyTask
** 功能描述 : 一体式充电处理任务函数
** 输    入 : port     0  ---- A枪      1 ---- B枪
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
//void ChgOnebodyTask(void *pdata)
//{
//	ADCInit(ADC_CLK);
//	Swich_ModeIn();
//	PwmInit(HIVCNT); 
//	Pwm = 100 - (ParaGetPwm()/100);
//	while(1)
//	{
//		ChgOnebodyPro(0);
//		OSTimeDlyHMSM(0, 0, 0, 10);
//	}
//}








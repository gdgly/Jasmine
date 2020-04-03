#include "adc.h"
#include "App.h"
#include "io.h"
#include "UART.h"
#include "Timer.h"
#include "Board.h"
static uint8_t ADstatus;
static uint8_t lastADstatus;
static uint16_t adc_value[3][ADCVAL_NUM];
static uint8_t cnt0=0;  
static uint8_t cnt1=0;
static uint8_t cnt4=0;
static uint8_t cnt=0;
static volatile uint32_t ADCSum=0;
static uint32_t LastADCSum=0;
uint32_t buf;


/************************************************************************************************************
** 函 数 名 : ADCInit
** 功能描述 : ADC初始化函数
** 输    入 : ADC_Clk  ADC时钟速率
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void ADCInit( uint32_t ADC_Clk )
{

  LPC_SC->PCONP |= (0x1<<12);        /* 使能ADC功率控制模块                 */   
  buf = LPC_ADC->DR[0];            /* 读数据寄存器，清除状态位            */
  buf = LPC_ADC->DR[1];                     
  buf = LPC_ADC->DR[4];  	
  buf = LPC_ADC->STAT;               /* 读状态寄存器，清除状态位            */
  buf = LPC_ADC->GDR;                /* 读全局数据寄存器，清除overrun状态位 */
  LPC_IOCON->P0_23 &= ~0x9F;  			 // TEMP-B
  LPC_IOCON->P0_23 |= 0x01;          /* ADC IN0  选择引脚为AD0模式          */
  LPC_IOCON->P0_24 &= ~0x9F; 			   // TEMP-C
  LPC_IOCON->P0_24 |= 0x01;          /* ADC IN1                             */         
  LPC_IOCON->P1_30 &= ~0x9F;
  LPC_IOCON->P1_30 |= 0x03;          /* ADC IN4        CP-AD                */ 
  LPC_ADC->CR = buf;
	
  LPC_ADC->CR = ((PeripheralClock/(ADC_Clk*31 )-1)<<8)| 
								( 0 << 16 ) |                   /* BURST = 0, 软件控制转换               */
								( 1 << 21 ) |                   /* PDN = 1,  处与正常模式                */
								( 0 << 24 ) |                   /* START = 0  不启动AD转换               */
								( 0 << 27 );                    /* EDGE = 0  上升沿启动                  */ 
	 LPC_ADC->INTEN &= ~(1<<8);  //禁止全局DONE产生中断	
	 LPC_ADC->INTEN |= (1<<0)|(1<<1)|(1<<4);
	 LPC_ADC->CR |= (1<<0)|(1<<1)|(1<<4);//设置需要转换的通道
	 buf = LPC_ADC->STAT;               /* 读状态寄存器，清除状态位            */
	 buf = LPC_ADC->GDR;                /* 读全局数据寄存器，清除overrun状态位 */
	 LPC_ADC->CR |= (1<<16); 
	 NVIC_EnableIRQ(ADC_IRQn);  //使能ADC中断
	 buf = LPC_ADC->STAT;               /* 读状态寄存器，清除状态位            */
	 buf = LPC_ADC->GDR;                /* 读全局数据寄存器，清除overrun状态位 */
	 NVIC_EnableIRQ(ADC_IRQn);  //使能ADC中断
}



/************************************************************************************************************
** 函 数 名 : ADC_IRQHandler
** 功能描述 : ADC中断函数
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/

void ADC_IRQHandler(void)     //ADC中断函数
{
		
	buf = LPC_ADC->STAT;               /* 读状态寄存器，清除状态位            */
	buf = LPC_ADC->GDR;                /* 读全局数据寄存器，清除overrun状态位 */
  if(((LPC_ADC->DR[0])&((unsigned int)1<<31))){  //AD0[0]转换完成产生的中断
		adc_value[0][cnt0] = ((LPC_ADC->DR[0])>>4) & 0xFFF;//获取本次中断的转换值
		cnt0++;
		if(cnt0 == ADCVAL_NUM){
			cnt0 = 0;
		}
	}
	if(((LPC_ADC->DR[1])&((unsigned int)1<<31))){   //AD0[1]转换完成产生的中断
		adc_value[1][cnt1] = ((LPC_ADC->DR[1])>>4) & 0xFFF;//获取本次中断的转换值
		cnt1++;
		if(cnt1 == ADCVAL_NUM){
			cnt1 = 0;
		}
	}
	if(((LPC_ADC->DR[4])&((unsigned int)1<<31))){   //AD0[2]转换完成产生的中断
		adc_value[2][cnt4] = ((LPC_ADC->DR[4])>>4) & 0xFFF;//获取本次中断的转换值
		cnt4++;
		if(cnt4 == ADCVAL_NUM){
			cnt4 = 0;
		}
	}
}





/************************************************************************************************************
** 函 数 名 : bubbleSort1
** 功能描述 : 冒泡排序
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
static void bubbleSort1(uint16_t arr[], uint16_t n) {
	uint16_t i = 0;
	uint16_t j = 0;
	uint16_t tmp = 0;
	uint16_t flag = 0; 
	for (i = 0; i < n; ++i) {
		flag = 0;
		for (j = 0; j < n - 1 - i; ++j) {
			if (arr[j] < arr[j + 1]) {
				flag = 1;
				tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
			}
		}
		if (flag == 0) {
			break;
		}
	}	
}




/************************************************************************************************************
** 函 数 名 : Get_V
** 功能描述 : 获取采集到的数据的平均值
** 输    入 : n  通道
** 输    出 : 无
** 返    回 : 无
*************************************************************************************************************
*/
uint16_t Get_V(uint8_t n)
{
	uint16_t i;
	uint32_t sumval=0; 
	uint16_t sumval1 = 0;
	uint16_t value[ADCVAL_NUM]={0};
	memcpy(value, &adc_value[n], sizeof(uint16_t)*ADCVAL_NUM);
	bubbleSort1(value, ADCVAL_NUM);	
	for(i=10; i < (ADCVAL_NUM-10); i++){
		sumval += value[i];
	}
	sumval = sumval / (ADCVAL_NUM-20);
	sumval1 = sumval;
	return sumval1;
}


/************************************************************************************************************
** 函 数 名 : GetTemp
** 功能描述 : 检测抢座温度是否过高
** 输    入 : port  --- 预留  只支持 A枪
** 输    出 : 无
** 返    回 : 1= 抢座温度过高   0 = 正常
*************************************************************************************************************
*/
uint8_t GetTemp(uint8_t port)
{
	static uint8_t err_temp;
	static uint32_t Temp;
	uint16_t V1, V2;	
	if((LPC_GPIO2->PIN & SWITCH_) == 0){	
		if(Temp == 0)
			Temp = TimerRead();
		if(TimerRead() - Temp > T1S*3){
			Temp = 0;
			V1 = Get_V(0);
			V2 = Get_V(1);	
			if(V1 > TEMP85 || V2>  TEMP85){
				err_temp = 1;
			}else{
				if(err_temp == 1){
					if(V1 < TEMP50 && V2 < TEMP50){
						err_temp = 0;
					}
				}else{
					err_temp = 0;
				}		
			}
		}

	}else{
		err_temp=0;
	}
	return err_temp;
}

/************************************************************************************************************
** 函 数 名 : ADCScan
** 功能描述 : 获取CP线  6 9 12V 电压 
** 输    入 : 无
** 输    出 : 无
** 返    回 : 无 
*************************************************************************************************************
*/
void ADCScan(void)
{
	uint32_t Result;
	LastADCSum = ADCSum;
	ADCSum = (Get_V(2) * 3714)/1000 + 180;  //  ADCSum = Get_V(2) * 33 * 461 / 4096 +180   放大1000倍  毫伏 ，对12V无效
	Result = (uint32_t)ADCSum - (uint32_t)LastADCSum;
	if(ADCSum < MIN6V){
		ADstatus = lastADstatus;
		cnt++;
		if(cnt == 6)
		ADstatus = ADC0V;	
	}else if(ADCSum < MAX6V){
		ADstatus = ADC6V;	//工作	
		cnt = 0;
		lastADstatus = ADstatus;
	}else if(ADCSum < MIN9V){
		if(abs(Result) >= ADTRANSITION){
			ADstatus = lastADstatus;
		}else{
			ADstatus = ADC0V;					
		}
	}else if(ADCSum < MAX9V){
		if(abs(Result) >= ADTRANSITION){
			ADstatus = lastADstatus;
		}else{
			ADstatus = ADC9V;
			cnt = 0;		
			lastADstatus = ADstatus;			
		}				
	}else if(ADCSum < MIN12V){
		if(abs(Result) >= ADTRANSITION){
			ADstatus = lastADstatus;
		}else{
			ADstatus = ADC0V;						
		}															
	}else if(ADCSum < MAX12V){
		ADstatus = ADC12V;																						
		cnt = 0;		
		lastADstatus = ADstatus;
	}else{
		lastADstatus = ADstatus;
		cnt++;
		if(cnt == 6)	
			ADstatus = ADC0V;																							
	}
//		print("换算电压 = %d  %d\r\n",ADCSum , ADstatus);
}

/************************************************************************************************************
** 函 数 名 : Get_AdcStatus
** 功能描述 : 获取CP线  6 9 12V 电压 
** 输    入 : port ---预留   只支持A枪
** 输    出 : 无
** 返    回 : 4---12V   3 --- 9V  2 --- 6V   其他值为异常电压
*************************************************************************************************************
*/
uint8_t Get_AdcStatus(uint8_t port)
{
	return ADstatus;

}















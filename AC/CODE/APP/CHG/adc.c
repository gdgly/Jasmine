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
** �� �� �� : ADCInit
** �������� : ADC��ʼ������
** ��    �� : ADC_Clk  ADCʱ������
** ��    �� : ��
** ��    �� :	��
*************************************************************************************************************
*/
void ADCInit( uint32_t ADC_Clk )
{

  LPC_SC->PCONP |= (0x1<<12);        /* ʹ��ADC���ʿ���ģ��                 */   
  buf = LPC_ADC->DR[0];            /* �����ݼĴ��������״̬λ            */
  buf = LPC_ADC->DR[1];                     
  buf = LPC_ADC->DR[4];  	
  buf = LPC_ADC->STAT;               /* ��״̬�Ĵ��������״̬λ            */
  buf = LPC_ADC->GDR;                /* ��ȫ�����ݼĴ��������overrun״̬λ */
  LPC_IOCON->P0_23 &= ~0x9F;  			 // TEMP-B
  LPC_IOCON->P0_23 |= 0x01;          /* ADC IN0  ѡ������ΪAD0ģʽ          */
  LPC_IOCON->P0_24 &= ~0x9F; 			   // TEMP-C
  LPC_IOCON->P0_24 |= 0x01;          /* ADC IN1                             */         
  LPC_IOCON->P1_30 &= ~0x9F;
  LPC_IOCON->P1_30 |= 0x03;          /* ADC IN4        CP-AD                */ 
  LPC_ADC->CR = buf;
	
  LPC_ADC->CR = ((PeripheralClock/(ADC_Clk*31 )-1)<<8)| 
								( 0 << 16 ) |                   /* BURST = 0, �������ת��               */
								( 1 << 21 ) |                   /* PDN = 1,  ��������ģʽ                */
								( 0 << 24 ) |                   /* START = 0  ������ADת��               */
								( 0 << 27 );                    /* EDGE = 0  ����������                  */ 
	 LPC_ADC->INTEN &= ~(1<<8);  //��ֹȫ��DONE�����ж�	
	 LPC_ADC->INTEN |= (1<<0)|(1<<1)|(1<<4);
	 LPC_ADC->CR |= (1<<0)|(1<<1)|(1<<4);//������Ҫת����ͨ��
	 buf = LPC_ADC->STAT;               /* ��״̬�Ĵ��������״̬λ            */
	 buf = LPC_ADC->GDR;                /* ��ȫ�����ݼĴ��������overrun״̬λ */
	 LPC_ADC->CR |= (1<<16); 
	 NVIC_EnableIRQ(ADC_IRQn);  //ʹ��ADC�ж�
	 buf = LPC_ADC->STAT;               /* ��״̬�Ĵ��������״̬λ            */
	 buf = LPC_ADC->GDR;                /* ��ȫ�����ݼĴ��������overrun״̬λ */
	 NVIC_EnableIRQ(ADC_IRQn);  //ʹ��ADC�ж�
}



/************************************************************************************************************
** �� �� �� : ADC_IRQHandler
** �������� : ADC�жϺ���
** ��    �� : ��
** ��    �� : ��
** ��    �� : ��
*************************************************************************************************************
*/

void ADC_IRQHandler(void)     //ADC�жϺ���
{
		
	buf = LPC_ADC->STAT;               /* ��״̬�Ĵ��������״̬λ            */
	buf = LPC_ADC->GDR;                /* ��ȫ�����ݼĴ��������overrun״̬λ */
  if(((LPC_ADC->DR[0])&((unsigned int)1<<31))){  //AD0[0]ת����ɲ������ж�
		adc_value[0][cnt0] = ((LPC_ADC->DR[0])>>4) & 0xFFF;//��ȡ�����жϵ�ת��ֵ
		cnt0++;
		if(cnt0 == ADCVAL_NUM){
			cnt0 = 0;
		}
	}
	if(((LPC_ADC->DR[1])&((unsigned int)1<<31))){   //AD0[1]ת����ɲ������ж�
		adc_value[1][cnt1] = ((LPC_ADC->DR[1])>>4) & 0xFFF;//��ȡ�����жϵ�ת��ֵ
		cnt1++;
		if(cnt1 == ADCVAL_NUM){
			cnt1 = 0;
		}
	}
	if(((LPC_ADC->DR[4])&((unsigned int)1<<31))){   //AD0[2]ת����ɲ������ж�
		adc_value[2][cnt4] = ((LPC_ADC->DR[4])>>4) & 0xFFF;//��ȡ�����жϵ�ת��ֵ
		cnt4++;
		if(cnt4 == ADCVAL_NUM){
			cnt4 = 0;
		}
	}
}





/************************************************************************************************************
** �� �� �� : bubbleSort1
** �������� : ð������
** ��    �� : ��
** ��    �� : ��
** ��    �� : ��
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
** �� �� �� : Get_V
** �������� : ��ȡ�ɼ��������ݵ�ƽ��ֵ
** ��    �� : n  ͨ��
** ��    �� : ��
** ��    �� : ��
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
** �� �� �� : GetTemp
** �������� : ��������¶��Ƿ����
** ��    �� : port  --- Ԥ��  ֻ֧�� Aǹ
** ��    �� : ��
** ��    �� : 1= �����¶ȹ���   0 = ����
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
** �� �� �� : ADCScan
** �������� : ��ȡCP��  6 9 12V ��ѹ 
** ��    �� : ��
** ��    �� : ��
** ��    �� : �� 
*************************************************************************************************************
*/
void ADCScan(void)
{
	uint32_t Result;
	LastADCSum = ADCSum;
	ADCSum = (Get_V(2) * 3714)/1000 + 180;  //  ADCSum = Get_V(2) * 33 * 461 / 4096 +180   �Ŵ�1000��  ���� ����12V��Ч
	Result = (uint32_t)ADCSum - (uint32_t)LastADCSum;
	if(ADCSum < MIN6V){
		ADstatus = lastADstatus;
		cnt++;
		if(cnt == 6)
		ADstatus = ADC0V;	
	}else if(ADCSum < MAX6V){
		ADstatus = ADC6V;	//����	
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
//		print("�����ѹ = %d  %d\r\n",ADCSum , ADstatus);
}

/************************************************************************************************************
** �� �� �� : Get_AdcStatus
** �������� : ��ȡCP��  6 9 12V ��ѹ 
** ��    �� : port ---Ԥ��   ֻ֧��Aǹ
** ��    �� : ��
** ��    �� : 4---12V   3 --- 9V  2 --- 6V   ����ֵΪ�쳣��ѹ
*************************************************************************************************************
*/
uint8_t Get_AdcStatus(uint8_t port)
{
	return ADstatus;

}















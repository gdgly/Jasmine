#ifndef __IO_H
#define __IO_H


#define LED_PWR      			(1ul << 25)              //电源灯                                                  
#define LED_PWR_H()		  	LPC_GPIO4->SET |= LED_PWR;	LPC_GPIO4->DIR |= LED_PWR                    /*                       */
#define LED_PWR_L() 			LPC_GPIO4->CLR |= LED_PWR;	LPC_GPIO4->DIR |= LED_PWR  

#define LED_CS      			(1ul << 29)             //  CS                                                    
#define LED_CS_H()  			LPC_GPIO4->SET |= LED_CS;	LPC_GPIO4->DIR |= LED_CS                    /*                       */
#define LED_CS_L()  			LPC_GPIO4->CLR |= LED_CS;	LPC_GPIO4->DIR |= LED_CS  

#define LED_CHARGE      	(1ul << 15)           // 充电指示灯                                                     
#define LED_CHARGE_H()  	LPC_GPIO4->SET |= LED_CHARGE;	LPC_GPIO4->DIR |= LED_CHARGE                    /*                       */
#define LED_CHARGE_L() 	  LPC_GPIO4->CLR |= LED_CHARGE;	LPC_GPIO4->DIR |= LED_CHARGE  

#define LED_WARNING      	(1ul << 28)          //故障灯                                                 
#define LED_WARNING_H()   LPC_GPIO4->SET |= LED_WARNING;	LPC_GPIO4->DIR |= LED_WARNING                    /*                       */
#define LED_WARNING_L() 	LPC_GPIO4->CLR |= LED_WARNING;	LPC_GPIO4->DIR |= LED_WARNING  

#define LED_RUN         	(1ul << 12) 
#define LED_RUN_H()   		LPC_GPIO0->SET |= LED_RUN;	LPC_GPIO0->DIR |= LED_RUN                    /*                       */
#define LED_RUN_L() 	 	  LPC_GPIO0->CLR |= LED_RUN;	LPC_GPIO0->DIR |= LED_RUN  

#define WID             	(1ul << 27)               //看门狗  IO口----- P0_27
#define WID_H()   				LPC_GPIO0->SET |= WID;	LPC_GPIO0->DIR |= WID                    /*                       */
#define WID_L() 	  			LPC_GPIO0->CLR |= WID;	LPC_GPIO0->DIR |= WID

#define BUZZER            (1ul << 12)               //蜂鸣器 IO  口
#define LED_BUZZER_H()   	LPC_GPIO2->SET |= BUZZER;	LPC_GPIO2->DIR |= BUZZER                    /*                       */
#define LED_BUZZER_L() 	  LPC_GPIO2->CLR |= BUZZER;	LPC_GPIO2->DIR |= BUZZER

#define AC              	(1ul << 26)               //看门狗  IO  口

//电子锁端口配置
#define _LOCK        	    (1ul << 23)     //电子锁输出      
#define _LOCKIN           (1ul << 20)     //电子锁输入    
#define TEST_LOCKIN()    	(LPC_GPIO1->DIR &= (~_LOCKIN))         //输入配置  //                        CC  p1.28
//485控制管脚
#define RS485             (1ul<<3)
#define RS485_H()   	   	LPC_GPIO4->DIR |= RS485;		LPC_GPIO4->SET |= RS485   		 /*                       */
#define RS485_L()       	LPC_GPIO4->DIR |= RS485; 	 	LPC_GPIO4->CLR |= RS485 		 /*                         */

#define TEST_cc    		    (1ul << 28) 														//CC	 P1.28
#define TEST_CCIn()       (LPC_GPIO1->DIR &= (~TEST_cc))


#define SWITCH_           (1ul <<11)       // P2_11
//#define GunMode()          (LPC_GPIO2->PIN & SWITCH_)
#define GUNLINE             1  
#define GUNSEAT             0

#define STOP_    		(1ul << 25) 														//STOP 急停	 P3.25
#define STOPIN()  	(LPC_GPIO3->DIR &= (~STOP_))            //输入配置
//#define STOP()		  (LPC_GPIO3->PIN & STOP_)                //急停信号




#define NONE              0         //带枪

uint8_t Test_CC(uint8_t MODE); 
uint8_t Test_Lock(void);
void  AC_OFF(void);
void AC_ON(void);
void E_Lock(void);
void E_Unlock(void);
void Swich_ModeIn(void);
uint8_t Test_Stop(void);
uint8_t Test_Door(void);

uint8_t TestStop(void);

#endif

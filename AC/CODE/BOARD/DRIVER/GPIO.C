#include "Board.h"
#include 	<LPC177x_8x.H>
#include <stdio.h>
#include "GPIO.h"

/****************************************************************************\
 Function:  GpioSetDir
 Descript:  设置ＩＯ口方向
 Input:	    ＩＯ号，方向　(input 输入　　output输出)
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void GpioSetDir(uint16_t io, uint8_t dir)
{
	uint8_t io_index=0,io_pin=0;
    
	io_index = io/100;
	io_pin = (io%100);
    
	switch(io_index)
	{
		case 0:
			if(dir == OUTPUT)
			{
			    LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO0->DIR |= (0x01<<io_pin);
			}
			else
			{
			   	LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO0->DIR &= ~(0x01<<io_pin);		
			}
			break;
			
		case 1:
			if(dir == OUTPUT)
			{
			    LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO1->DIR |= (0x01<<io_pin);
			}
			else
			{
			   	LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO1->DIR &= ~(0x01<<io_pin);		
			}
			break;
			
		case 2:
			if(dir == OUTPUT)
			{
			    LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO2->DIR |= (0x01<<io_pin);
			}
			else
			{
			   	LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO2->DIR &= ~(0x01<<io_pin);		
			}
			break;
			
		case 3:
			if(dir == OUTPUT)
			{
			    LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO3->DIR |= (0x01<<io_pin);
			}
			else
			{
			   	LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO3->DIR &= ~(0x01<<io_pin);		
			}
			break;
			
		case 4:
			if(dir == OUTPUT)
			{
			    LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO4->DIR |= (0x01<<io_pin);
			}
			else
			{
			   	LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO4->DIR &= ~(0x01<<io_pin);		
			}
			break;

		case 5:
			if(dir == OUTPUT)
			{
			    LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO5->DIR |= (0x01<<io_pin);
			}
			else
			{
			   	LPC_SC->PCONP |= (0x01<<15);
			   	LPC_GPIO5->DIR &= ~(0x01<<io_pin);		
			}
			break;
        
		default:
		    break;
	}
}


/****************************************************************************\
 Function:  GpioSetLev
 Descript:  设置ＩＯ口电平
 Input:	    ＩＯ号，电平　(HIGH 高　LOW 低)
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
void GpioSetLev(uint16_t io,uint8_t lev)
{
	uint8_t io_index=0,io_pin=0;
	
	io_index = io/100;
	io_pin = (io%100);
    
	switch(io_index)
	{
		case 0:
			if(lev == LOW)
			{
		        LPC_GPIO0->CLR |= (0x01<<io_pin);		
			}
			else
			{
				LPC_GPIO0->SET |= (0x01<<io_pin);	
			}
			break;
			
		case 1:
			if(lev == LOW)
			{
		        LPC_GPIO1->CLR |= (0x01<<io_pin);		
			}
			else
			{
				LPC_GPIO1->SET |= (0x01<<io_pin);	
			}
			break;
			
		case 2:
			if(lev == LOW)
			{
		        LPC_GPIO2->CLR |= (0x01<<io_pin);		
			}
			else
			{
				LPC_GPIO2->SET |= (0x01<<io_pin);	
			}
			break;
			
		case 3:
			if(lev == LOW)
			{
		        LPC_GPIO3->CLR |= (0x01<<io_pin);		
			}
			else
			{
				LPC_GPIO3->SET |= (0x01<<io_pin);	
			}
			break;
			
		case 4:
			if(lev == LOW)
			{
		        LPC_GPIO4->CLR |= (0x01<<io_pin);		
			}
			else
			{
				LPC_GPIO4->SET |= (0x01<<io_pin);	
			}
			break;

		case 5:
			if(lev == LOW)
			{
		        LPC_GPIO5->CLR |= (0x01<<io_pin);		
			}
			else
			{
				LPC_GPIO5->SET |= (0x01<<io_pin);	
			}
			break;
			
		default:
		       break;
	}
}




/****************************************************************************\
 Function:  Gpio_GetLevel
 Descript:  获取ＩＯ口电平
 Input:	    ＩＯ号，
 Output:    无 
 Return:    电平　(HIGH 高　LOW 低)
 Author:    quanhouwei
 Datetime:  17-08-18
 *****************************************************************************/
uint8_t GpioGetLev(uint16_t io)
{
	uint8_t io_index=0,io_pin=0;
	uint8_t ret=0;
    
	io_index = io/100;
	io_pin =   io%100;
    
	switch(io_index)
	{
		case 0:
			if (LPC_GPIO0->PIN & (0x01<<io_pin))
            {
                ret=HIGH;
            }
            else 
            {
                ret=LOW;
            }
            break;
            
		case 1:
			if (LPC_GPIO1->PIN & (0x01<<io_pin))
            {
                ret=HIGH;
            }
            else 
            {
                ret=LOW;
            }
            break;
            
		case 2:
			if (LPC_GPIO2->PIN & (0x01<<io_pin))
            {
                ret=HIGH;
            }
            else 
            {
                ret=LOW;
            }
            break;
            
		case 3:
			if (LPC_GPIO3->PIN & (0x01<<io_pin))
            {
                ret=HIGH;
            }
            else 
            {
                ret=LOW;
            }
            break;
            
		case 4:
			if (LPC_GPIO4->PIN & (0x01<<io_pin))
            {
                ret=HIGH;
            }
            else 
            {
                ret=LOW;
            }
            break;

		case 5:
			if (LPC_GPIO5->PIN & (0x01<<io_pin))
            {
                ret=HIGH;
            }
            else 
            {
                ret=LOW;
            }
            break;
            
		default:
		    break;
	}
    
	return ret;//失败
}


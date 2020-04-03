#include "Board.h"
#include "FRAM.h"
#include "GPIO.h"
#include "SPI.h"
#include "includes.h"

/************************************************************************************************************
** 函 数 名 : FramEnable
** 功能描述 : 使能铁电存储器
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void FramEnable(void)
{
	GpioSetDir(GPIO_FRAMCS,OUTPUT);
	GpioSetLev(GPIO_FRAMCS,LOW);
}

/************************************************************************************************************
** 函 数 名 : FramDisable
** 功能描述 : 失能铁电存储器
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void FramDisable(void)
{
	GpioSetDir(GPIO_FRAMCS,OUTPUT);
	GpioSetLev(GPIO_FRAMCS,HIGH);
}

/************************************************************************************************************
** 函 数 名 : FramInit
** 功能描述 : 铁电存储器初始化函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void FramInit(void)
{
	SpiInit(FRAMSPINO);
	FramDisable();
}

/************************************************************************************************************
** 函 数 名 : tdelay
** 功能描述 : 延时函数
** 输    入 : 无
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
static void tdelay(uint16_t dely)
{
	uint16_t i=0, j=0;

	for(i = 0; i <= dely/10; i++){
		for(j = 0; j <= dely; j++){
		}
	}
}

/************************************************************************************************************
** 函 数 名 : FramInit
** 功能描述 : 铁电存储器写函数
** 输    入 : addr 地址 data 数据缓冲区 length 数据长度
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void FramWrite(uint16_t addr, uint8_t *data, uint16_t length)
{
	uint16_t  i=0;
	uint8_t   st=0;

	FramEnable();
	tdelay(5);
	SpiWriteByte(FRAMSPINO,COMD_RD_STATUS);
	st=SpiReadByte(FRAMSPINO);
	FramDisable();

	if ((st & 0x71) == 0){
		FramEnable();
		tdelay(5);
		SpiWriteByte(FRAMSPINO,COMD_WR_ENABLE);
		FramDisable();
		
		do {
			FramEnable();
			tdelay(5);
			SpiWriteByte(FRAMSPINO,COMD_RD_STATUS);
			st=SpiReadByte(FRAMSPINO);
			FramDisable();
		} while(st & 0x80);

		FramEnable();
		tdelay(5);
		SpiWriteByte(FRAMSPINO,COMD_WR_MEMORY);
		SpiWriteByte(FRAMSPINO,(addr >> 8) & 0xff);
		SpiWriteByte(FRAMSPINO,addr & 0xff);
		for (i = 0; i < length; i++){	
			SpiWriteByte(FRAMSPINO,*data);
			data	+= 1;
		}
		tdelay(5);
		FramDisable();
	}
}

/************************************************************************************************************
** 函 数 名 : FramRead
** 功能描述 : 铁电存储器读函数
** 输    入 : addr 地址 data 数据缓冲区 length 数据长度
** 输    出 : 无
** 返    回 :	无
*************************************************************************************************************
*/
void FramRead(uint16_t addr, uint8_t *data, uint16_t length)
{
	uint16_t i=0;

	FramEnable();
	tdelay(5);
	SpiWriteByte(FRAMSPINO,COMD_RD_MEMORY);
	SpiWriteByte(FRAMSPINO,(addr >> 8) & 0xff);
	SpiWriteByte(FRAMSPINO,addr & 0xff);
	for (i = 0; i < length; i++){
		*data	= SpiReadByte(FRAMSPINO);
		data++;
	}
	FramDisable();
	tdelay(5);
}

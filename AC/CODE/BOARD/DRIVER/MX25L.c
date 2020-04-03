#include"Board.h"
#include "GPIO.h"
#include <string.h>
#include "MX25L.h"
#include "SPI.h"

/****************************************************************************\
 Function:  Mx25Init
 Descript:  MX25初始化  (NORFLASH)
 Input:	    无
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
void Mx25Init(void)
{
	SpiInit(MX25SPINO);
	Mx25Disable();
	
}


/****************************************************************************\
 Function:  Mx25write
 Descript:  向mx25写数据
 Input:	    数据ＢＵＦ，地址，数据长度
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
void Mx25write(uint32_t addr,uint8_t *data,uint16_t len)
{
	uint8_t  temp=0;
	uint16_t i=0;

	if ((data==NULL) || (len==0))
	{
		return ;
	}
	
	Mx25Enable();														// 写使能
	SpiWriteByte(MX25SPINO,SPI_FLASH_INS_WREN); 
	Mx25Disable();

	while(!(Mx25GetSt()&0x02)); // 是否可写
	
	Mx25Enable();
	SpiWriteByte(MX25SPINO,SPI_FLASH_INS_PP);
	temp=addr>>16;
	SpiWriteByte(MX25SPINO,temp);
	temp=addr>>8;
	SpiWriteByte(MX25SPINO,temp);
	temp=addr&0xff;
	SpiWriteByte(MX25SPINO,temp);
	for(i=0;i<len;i++)
	{
		SpiWriteByte(MX25SPINO,*data++);
	}
	Mx25Disable();

	while(Mx25GetSt()&0x01);	// 是否写完	

	Mx25Enable();														// 写失能
	SpiWriteByte(MX25SPINO,SPI_FLASH_INS_WRDI); 
	Mx25Disable();
}



/****************************************************************************\
 Function:  Mx25Read
 Descript:  从mx25读数据
 Input:	    数据ＢＵＦ，地址，数据长度
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
void Mx25Read(uint32_t addr,uint8_t *data,uint16_t len)
{
	uint16_t i=0;
	uint8_t temp=0;

	if ((data==NULL) || (len==0))
	{
		return;
	}
	
	Mx25Enable();
	
    SpiWriteByte(MX25SPINO,SPI_FLASH_INS_READ);
	temp=addr>>16;
	SpiWriteByte(MX25SPINO,temp);
	temp=addr>>8;
	SpiWriteByte(MX25SPINO,temp);
	temp=addr&0xff;
	SpiWriteByte(MX25SPINO,temp);
	for(i=0;i<len;i++)
	{
		*data++= SpiReadByte(MX25SPINO);;
	}
	
	Mx25Disable();
}


/****************************************************************************\
 Function:  Mx25Erase
 Descript:  擦除mx25一个扇区的数据　(4k)
 Input:	    地址
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
void Mx25Erase(uint32_t addr)
{
	uint8_t temp=0;

	Mx25Enable();
	SpiWriteByte(MX25SPINO,SPI_FLASH_INS_WREN);			//写使能
	Mx25Disable();

	while(!(Mx25GetSt()&0x02));	// 是否可写
	
	Mx25Enable();
	SpiWriteByte(MX25SPINO,0x20);				//擦除扇区
	temp=addr>>16;								//扇区地址
	SpiWriteByte(MX25SPINO,temp);
	temp=addr>>8;
	SpiWriteByte(MX25SPINO,temp);
	temp=addr&0xff;
	SpiWriteByte(MX25SPINO,temp);
	Mx25Disable();
	
	while(Mx25GetSt()&0x01);  // 是否写完	

	Mx25Enable();
	SpiWriteByte(MX25SPINO,SPI_FLASH_INS_WRDI);			//写失能
	Mx25Disable();
}




/****************************************************************************\
 Function:  Mx25GetSt
 Descript: 　读取ＭＸ２５的状态寄存器
 Input:	    地址
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
static uint8_t Mx25GetSt(void)
{	
	uint8_t st=0;
	
	Mx25Enable();
	
	SpiWriteByte(MX25SPINO,0x05);
	st=SpiReadByte(MX25SPINO);
	
	Mx25Disable();

	return st;
}



/****************************************************************************\
 Function:  Mx25Enable
 Descript: 　使能ＭＸ２５
 Input:	    无
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
static void Mx25Enable(void)
{
	GpioSetDir(GPIO_NORCS,OUTPUT);
	GpioSetLev(GPIO_NORCS,LOW);
}



/****************************************************************************\
 Function:  Mx25Disable
 Descript: 　失能ＭＸ２５
 Input:	    无
 Output:    无 
 Return:    无
 Author:    quanhouwei
 Datetime:  17-08-21
 *****************************************************************************/
static void Mx25Disable(void)
{
	GpioSetDir(GPIO_NORCS,OUTPUT);
	GpioSetLev(GPIO_NORCS,HIGH);
}






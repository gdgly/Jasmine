#ifndef _GPIO_H_
#define _GPIO_H_


#define OUTPUT             1            //输出
#define INPUT              0            //输入

#define HIGH              1           // 高电平
#define LOW               0           // 低电平



void 	GpioSetDir(uint16_t io, uint8_t dir);
void 	GpioSetLev(uint16_t io,uint8_t lev);
uint8_t 	GpioGetLev(uint16_t io);




#endif


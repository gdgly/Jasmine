#ifndef _FRAM_H
#define _FRAM_H

#include <stdint.h>

#define  COMD_WR_ENABLE				0x06
#define  COMD_WR_DISABLE			0x04
#define  COMD_RD_STATUS				0x05
#define  COMD_WR_STATUS				0x01
#define  COMD_RD_MEMORY				0x03
#define  COMD_WR_MEMORY				0x02



void 	FramInit(void);
void    FramWrite(uint16_t addr,uint8_t *data,uint16_t length);
void	FramRead(uint16_t addr,uint8_t *data,uint16_t length);





#endif


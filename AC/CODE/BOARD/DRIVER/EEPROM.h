#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stdint.h>

/*********************************************************************//**
 * Macro defines for EEPROM command register
 **********************************************************************/
#define EEPROM_CMD_8_BIT_READ			(0)
#define EEPROM_CMD_16_BIT_READ			(1)
#define EEPROM_CMD_32_BIT_READ			(2)
#define EEPROM_CMD_8_BIT_WRITE			(3)
#define EEPROM_CMD_16_BIT_WRITE			(4)
#define EEPROM_CMD_32_BIT_WRITE			(5)
#define EEPROM_CMD_ERASE_PRG_PAGE		(6)

#define EEPROM_CMD_RDPREFETCH			(1<<3)

#define EEPROM_PAGE_SIZE				64
#define EEPROM_PAGE_NUM					63

/*********************************************************************//**
 * Macro defines for EEPROM address register
 **********************************************************************/
#define EEPROM_PAGE_OFFSET(n)			(n&0x3F)
#define EEPROM_PAGE_ADRESS(n)			((n&0x3F)<<6)

/*********************************************************************//**
 * Macro defines for EEPROM write data register
 **********************************************************************/
#define	EEPROM_WDATA_8_BIT(n)			(n&0x000000FF)
#define EEPROM_WDATA_16_BIT(n)			(n&0x0000FFFF)
#define EEPROM_WDATA_32_BIT(n)			(n&0xFFFFFFFF)

/*********************************************************************//**
 * Macro defines for EEPROM read data register
 **********************************************************************/
#define	EEPROM_RDATA_8_BIT(n)			(n&0x000000FF)
#define EEPROM_RDATA_16_BIT(n)			(n&0x0000FFFF)
#define EEPROM_RDATA_32_BIT(n)			(n&0xFFFFFFFF)

/*********************************************************************//**
 * Macro defines for EEPROM power down register
 **********************************************************************/
#define EEPROM_PWRDWN					(1<<0)

#define EEPROM_ENDOF_RW					(26)
#define EEPROM_ENDOF_PROG				(28)

typedef enum
{
	MODE_8_BIT = 0,
	MODE_16_BIT,
	MODE_32_BIT
}EEPROM_Mode_Type;

void EepromInit(void);
void EepromWrite(uint16_t page_offset, uint16_t page_address, void* data, EEPROM_Mode_Type mode, uint32_t count);
void EepromRead(uint16_t page_offset, uint16_t page_address, void* data, EEPROM_Mode_Type mode, uint32_t count);
void EepromErase(uint16_t page_address);

#endif

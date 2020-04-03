#ifndef _MX25L_H
#define _MX25L_H


#define SPI_FLASH_INS_WREN     		0x06	// Write Enable
#define	SPI_FLASH_INS_WRDI     		0x04	// Write Disable
#define	SPI_FLASH_INS_RDSR			0x05	// Read Status Register
#define	SPI_FLASH_INS_WRSR     		0x01	// write status register
#define	SPI_FLASH_INS_READ			0x03	// Read Data Bytes
#define	SPI_FLASH_INS_FAST_READ		0x0B	// Read Data Bytes at Higher Speed
#define	SPI_FLASH_INS_PP			0x02	// Page Program
#define	SPI_FLASH_INS_SE			0x20	// Sector Erase
#define	SPI_FLASH_INS_BE      		0xD8	// Bulk Erase
#define	SPI_FLASH_INS_DP			0xB9	// Deep Power-down
#define	SPI_FLASH_INS_RDID			0x9F	// Read Identification
#define SPI_FLASH_INS_RES			0xAB
#define SPI_FLASH_INS_CE  			0xC7
#define SPI_FLASH_INS_CP 			0xAD

void	Mx25Init(void);
void 	Mx25write(uint32_t addr,uint8_t *data,uint16_t len);
void 	Mx25Read(uint32_t addr,uint8_t *data,uint16_t len);
void 	Mx25Erase(uint32_t addr);

static uint8_t 	Mx25GetSt(void);
static void 	Mx25Enable(void);
static void 	Mx25Disable(void);






#endif


#ifndef _RS485_H
#define _RS485_H


void    RS485Init(uint32_t bps);
uint16_t 	RS485Write(uint8_t *data, uint16_t datalen);
uint16_t 	RS485Read(uint8_t *data, uint16_t datalen);
void 	RS485Flush(void);




#endif




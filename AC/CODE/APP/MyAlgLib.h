#ifndef _MYALGLIB_H
#define _MYALGLIB_H


uint8_t 	MyXorSum(uint8_t *data,uint16_t len);
uint8_t 	MyCum8Sum(uint8_t *data,uint16_t len);
uint32_t 	MyCum32Sum(uint8_t *data,uint16_t len);
uint32_t  MyAtoi(uint8_t *data);
uint8_t   Mybcd2hex(uint8_t data);
uint32_t 	Mylbcdstr2hex(uint8_t *data,uint16_t len);
uint32_t 	Myhbcdstr2hex(uint8_t *data,uint16_t len);
uint16_t  Myhtons(uint16_t data);
uint32_t  Myhtonl(uint32_t data);
uint8_t   MyCrc8(uint8_t *ptr,uint8_t len);
uint16_t 	MyCrc16(uint8_t *buf, uint16_t len) ;
uint8_t 	MyCheckIsInTime(uint32_t curtime,uint32_t starttime,uint32_t endtime);


uint32_t Bcd32ToHex32(uint32_t u32bcd);
uint32_t Hex32ToBcd32(uint32_t u32hex);
void u32tobcdnomstr1(unsigned int sval,unsigned char *dbuf,unsigned int len);

void Decryption(uint8_t *in_str, uint8_t in_len, uint8_t *out_str);
void Dncryption(uint8_t *in_str, uint8_t in_len, uint8_t *out_str);
#endif





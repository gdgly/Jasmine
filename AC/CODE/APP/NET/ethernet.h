#ifndef __ETHERNET_H
#define __ETHERNET_H


uint8_t EthConn(uint8_t sn, uint8_t serverip[4], uint16_t serverport);
uint8_t EthSend(uint8_t sn, uint8_t *buffer, uint16_t len);
uint16_t EthRecv(uint8_t sn, uint8_t *buffer, uint16_t len);

void UDPSend(uint8_t* buf, uint16_t len);
void AnalyseMsgFromUdpPlatform(void);

void EthHandle(void);
void EthInit(void);
void EthTask(void *pdata);
#endif







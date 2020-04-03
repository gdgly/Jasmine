#ifndef _ENCRYPT_H_
#define _ENCRYPT_H_

//数据加密
void Encrypt(const uint8_t* input, const uint8_t* key, uint8_t *output, const uint32_t length);

//数据解密
void Decrypt(const uint8_t* input, const uint8_t* key, uint8_t *output, const uint32_t length);

//客户编号转密钥
uint64_t CalcKey(uint32_t nCustomerID);

#endif

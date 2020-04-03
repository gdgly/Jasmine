#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include"App.h"



/****************************************************************************\
 Function:  MyXorSum
 Descript:  抑或校验和
 Input:	    数据，长度	
 Output:    无
 Return:    校验和
 Author:    quanhouwei
 Datetime:  17-08-23
 *****************************************************************************/
uint8_t MyXorSum(uint8_t *data,uint16_t len)
{
	uint8_t sum=0;
	uint16_t i=0;
	
	if ((data==NULL) || (len==0))
	{
		return 0;
	}

	sum=data[0];
	for (i=1;i<len;i++)
	{
		sum ^= data[i];
	}

	return sum;	
}


/****************************************************************************\
 Function:  MyCum8Sum
 Descript:  8位累加和
 Input:	    数据，长度	
 Output:    无
 Return:    累加和
 Author:    quanhouwei
 Datetime:  17-08-25
 *****************************************************************************/
uint8_t MyCum8Sum(uint8_t *data,uint16_t len)
{
	uint8_t sum=0;
	uint16_t i=0;
	
	if ((data==NULL) || (len==0))
	{
		return 0;
	}

	sum=data[0];
	for (i=1;i<len;i++)
	{
		sum += data[i];
	}

	return sum;
}


/****************************************************************************\
 Function:  MyCum32Sum
 Descript:  32位累加和
 Input:	    数据，长度	
 Output:    无
 Return:    累加和
 Author:    quanhouwei
 Datetime:  17-08-25
 *****************************************************************************/
uint32_t MyCum32Sum(uint8_t *data,uint16_t len)
{
	uint32_t sum=0;
	uint16_t i=0;
	
	if ((data==NULL) || (len==0))
	{
		return 0;
	}

	sum=data[0];
	for (i=1;i<len;i++)
	{
		sum += data[i];
	}

	return sum;
}




/*************************************************************************************
 Function: MyAtoi
 Descript: ASCII转换成整形
 Input:    data 
 Output:   无
 Return:   转换成的整数
 Author  : quanhouwei
 Datetime: 17.08.25
*************************************************************************************/
uint32_t MyAtoi(uint8_t *data)
{
    uint8_t len=0,i=0;
    uint8_t buf[11]={0};
	uint32_t dat32=0;

    if (data == NULL)
    {
        return 0;
    }

    len=strlen((char   *)data);
    if(len == 0)
    {
        return 0;
    }
    if (len>10)
    { 
        len=10;  
    }
   
    memcpy(buf,data,len);
    
    for (i=0;i<len;i++)
    {
        if ((buf[i]<'0') || (buf[i]>'9'))
        {
            buf[i]=0;
            break;
        }
    }

	sscanf((char *)buf, "%d",&dat32);
	
	return dat32;
        
}




/*************************************************************************************
 Function: Mybcd2hex
 Descript: 单个BCD码转成HEX 如0X10 转成 10
 Input:    单个BCD码
 Output:   无
 Return:   转换成的HEX
 Author  : quanhouwei
 Datetime: 17.08.31
*************************************************************************************/
uint8_t Mybcd2hex(uint8_t data)
{
    uint8_t tmp=0, tmp1=0;

    tmp = data / 16;
    tmp1= data % 16;

    if ((tmp > 9)||(tmp1 > 9))
    {
        return 0;
    }
    
	return (tmp*10 + tmp1);
}


/*************************************************************************************
 Function: Mylbcdstr2hex
 Descript: BCD码串装HEX,低位在前 如0X10 0X20 转成 210
 Input:    BCD码串，长度
 Output:   无
 Return:   转换成的HEX
 Author  : quanhouwei
 Datetime: 17.08.31
*************************************************************************************/
uint32_t Mylbcdstr2hex(uint8_t *data,uint16_t len)
{	
	uint16_t i=0;
	uint32_t dat32=0;
	
	if ((data==NULL) || (len==0))
	{
		return 0;
	}

	for (i=0;i<len;i++)
	{
		dat32 = dat32*100+Mybcd2hex(data[len-1-i]);
	}

	return dat32;
}



/*************************************************************************************
 Function: Myhbcdstr2hex
 Descript: BCD码串装HEX,高位在前 如0X10 0X20 转成 120
 Input:    BCD码串，长度
 Output:   无
 Return:   转换成的HEX
 Author  : quanhouwei
 Datetime: 17.08.31
*************************************************************************************/
uint32_t Myhbcdstr2hex(uint8_t *data,uint16_t len)
{	
	uint16_t i=0;
	uint32_t dat32=0;
	
	if ((data==NULL) || (len==0))
	{
		return 0;
	}

	for (i=0;i<len;i++)
	{
		dat32 = dat32*100+Mybcd2hex(data[i]);
	}

	return dat32;
}


/*************************************************************************************
 Function: Myhtons
 Descript: unsigned short字节序转换
 Input:    需转换的SHORT型
 Output:   无
 Return:   转换成的SHORT
 Author  : quanhouwei
 Datetime: 17.09.04
*************************************************************************************/
uint16_t Myhtons(uint16_t data)
{
	uint16_t tmp=0;


	*((uint8_t *)&tmp)=*((uint8_t *)&data+1);
	*((uint8_t *)&tmp+1)=*((uint8_t *)&data);

	return tmp;
	
}




/*************************************************************************************
 Function: Myhtonl
 Descript: unsigned int字节序转换
 Input:    需转换的int型
 Output:   无
 Return:   转换成的int
 Author  : quanhouwei
 Datetime: 17.09.04
*************************************************************************************/
uint32_t Myhtonl(uint32_t data)
{
	uint32_t tmp=0;


	*((uint8_t *)&tmp)=*((uint8_t *)&data+3);
	*((uint8_t *)&tmp+1)=*((uint8_t *)&data+2);
	*((uint8_t *)&tmp+2)=*((uint8_t *)&data+1);
	*((uint8_t *)&tmp+3)=*((uint8_t *)&data);

	return tmp;
	
}



//CRC8多项式为 X8+X2+X1+1 = 0x07
uint8_t MyCrc8(uint8_t *ptr,uint8_t len)
{
	uint8_t crc;
	uint8_t i;
	
	crc = 0;
	while(len--)
	{
		crc ^= *ptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 0x80)
			{
				crc = (crc << 1) ^ 0x07;
			}
			else
			{
				crc <<= 1;
			}
		}
	}
	return crc;
} 
uint16_t MyCrc16(uint8_t *buf, uint16_t len) 
{
	uint16_t crc16tab[256]= {  
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,  
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,  
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,  
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,  
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,  
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,  
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,  
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,  
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,  
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,  
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,  
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,  
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,  
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,  
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,  
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,  
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,  
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,  
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,  
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,  
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,  
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,  
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,  
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,  
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,  
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,  
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,  
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,  
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,  
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,  
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,  
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0  
	};  
    uint16_t counter=0,crc=0;  
	
    for (counter = 0; counter < len; counter++)  
    {
    	crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *buf++)&0x00FF];  
    }
	
    return crc;  
}



/*************************************************************************************
 Function: MyCheckIsInTime
 Descript: 判断特定时间是否再时间段内
 Input:    特定时间，开始时间，结束时间
 Output:   无
 Return:   1 在时间段内   0 不在时间段内
 Author  : quanhouwei
 Datetime: 17.09.04
*************************************************************************************/
uint8_t MyCheckIsInTime(uint32_t curtime,uint32_t starttime,uint32_t endtime)
{
	if (endtime >= starttime)
	{
		if ((curtime>=starttime) && (curtime<=endtime))
		{	
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if ((curtime>=starttime) || (curtime<=endtime))
		{	
			return 1;
		}
		else
		{
			return 0;
		}
	}
}


	

























/************************************************************************************************************
** 函 数 名 : Bcd32ToHex32
** 功能描述 : 32位BCD码转成32位HEX码
** 输    入 : u32bcd 需要转换的32位BCD码
** 输    出 : 无
** 返    回 : 转换好的32位HEX码
*************************************************************************************************************
*/
uint32_t Bcd32ToHex32(uint32_t u32bcd)
{
   uint32_t result;
   
   result  =((u32bcd>>28)&0x0f)*10000000;
   result +=((u32bcd>>24)&0x0f)*1000000;
   result +=((u32bcd>>20)&0x0f)*100000;
   result +=((u32bcd>>16)&0x0f)*10000;
   result +=((u32bcd>>12)&0x0f)*1000;
   result +=((u32bcd>> 8)&0x0f)*100;
   result +=((u32bcd>> 4)&0x0f)*10;
   result +=u32bcd&0x0f;
   
   return(result);
}

/************************************************************************************************************
** 函 数 名 : Hex32ToBcd32
** 功能描述 : 32位HEX码转成32位BCD码
** 输    入 : u32hex 需要转换的32位HEX码
** 输    出 : 无
** 返    回 : 转换好的32位BCD码
*************************************************************************************************************
*/
uint32_t Hex32ToBcd32(uint32_t u32hex)
{
	uint32_t result;

	if (u32hex>=99999999){
		return(0xFFFFFFFF);
	}
	result	=(u32hex/10000000)<<28;
	u32hex %=10000000;
	result +=(u32hex/1000000)<<24;
	u32hex %=1000000;
	result +=(u32hex/100000)<<20;
	u32hex %=100000;
	result +=(u32hex/10000)<<16;
	u32hex %=10000;
	result +=(u32hex/1000)<<12;
	u32hex %=1000;
	result +=(u32hex/100)<<8;
	u32hex %=100;
	result +=(u32hex/10)<<4;
	u32hex %=10;
	result +=u32hex;

	return(result);
}






//32位数组扩展成8位BCD数组（高位在前）
void u32tobcdnomstr1(unsigned int sval,unsigned char *dbuf,unsigned int len)
{	
	if(len == 4)
	{
		dbuf[3] = sval%10;
		dbuf[2] = (sval/10)%10;
		dbuf[1] = (sval/100)%10;
		dbuf[0] = (sval/1000)%10;
	}
	else if(len == 6)
	{
	  dbuf[5] = sval%10;
		dbuf[4] = (sval/10)%10;
		dbuf[3] = (sval/100)%10;
		dbuf[2] = (sval/1000)%10;
		dbuf[1] = (sval/10000)%10;
		dbuf[0] = (sval/100000)%10;
	}
	else if(len == 8)
	{
		dbuf[7] = sval%10;
		dbuf[6] = (sval/10)%10;
		dbuf[5] = (sval/100)%10;
		dbuf[4] = (sval/1000)%10;
		dbuf[3] = (sval/10000)%10;
		dbuf[2] = (sval/100000)%10;
		dbuf[1] = (sval/1000000)%10;
		dbuf[0] = (sval/10000000)%10;
	}
	else if(len == 12)
	{
		dbuf[11] = sval%10;
		dbuf[10] = (sval/10)%10;
		dbuf[9] = (sval/100)%10;
		dbuf[8] = (sval/1000)%10;
		dbuf[7] = (sval/10000)%10;
		dbuf[6] = (sval/100000)%10;
		dbuf[5] = (sval/1000000)%10;
		dbuf[4] = (sval/10000000)%10;
		dbuf[3] = (sval/100000000)%10;
		dbuf[2] = (sval/1000000000)%10;
		dbuf[1] = (sval/10000000000)%10;
		dbuf[0] = (sval/100000000000)%10;
	}	
}


/************************************************************************************************************
** 函 数 名 : HexstrToDec
** 功能描述 : 十六进制字符转十进制数
** 输    入 : 待转换的十六进制字符串
** 输    出 : 无
** 返    回 : 十进制数
*************************************************************************************************************
*/
uint8_t HexstrToDec(const uint8_t *str)
{
	uint8_t i;
	uint8_t count;
	uint8_t ret = 0;
	char to_hex[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'}; 
	for(count = 0;count < 2;count++)
	{
		for(i = 0; i<16;i++)
		{
			if(str[count] == to_hex[i])
			{
				if(count == 0)
				{
					ret = i*16+ret;
				}
				else if(count == 1)
				{
					ret = i + ret;
				}
				break;				
			}
		}
		if(i>=16)
		{
			return 0;		
		}
	}	
	return ret;
}

//解密
void Decryption(uint8_t *in_str, uint8_t in_len, uint8_t *out_str)
{
	uint8_t i,j;
	uint8_t buf[64] = {0};
	if((in_len % 2) == 1 || in_len <4 )
		return;
	for (i = 0; i < (in_len/2); i++){
		buf[i] = HexstrToDec(in_str + (i * 2));	
	}	
	for (j = 0; j < (i-1); j++)
		*(out_str + j) = buf[j]-(j*(j%3+1))-(buf[i-1] - (i-1))+'0';	
}


//加密
void Dncryption(uint8_t *in_str, uint8_t in_len, uint8_t *out_str)
{
	uint8_t i;
	uint32_t SunNum = 0;
	uint8_t buf[64] = {0};
	for(i=0;i<in_len;i++)
		SunNum = SunNum + *(in_str + i)-'0';	
	for (i = 0; i<in_len; i++)			
		buf[i] = *(in_str + i) - '0' + (i*(i%3+1)) + (SunNum%15)+1;
	buf[i] = (SunNum%15)+1+i;	
	for (i=0;i<in_len+1;i++){
		sprintf((char *)(out_str+(i*2)),"%02x",buf[i]); 
	}
}



















#include "IAP.h"
#include "lpc177x_8x.h"
#include "includes.h"
__align(4) uint8_t GucIapTmp[1024];

void (*IAP_Entry) (uint32_t param_tab[], uint32_t result_tab[]);

uint32_t  paramin[8];
uint32_t  paramout[8];

uint32_t *GpulFlagPoint = (uint32_t *)USERFLAGADDR;

uint8_t SysUpdateFlag;
uint32_t update_len;
uint32_t update_last_remain_len;

/*********************************************************************************************************
** Function name:       sectorPrepare
** Descriptions:        IAP操作扇区选择，命令代码50
** input parameters:    sec1:           起始扇区
**                      sec2:           终止扇区
** output parameters:   paramout[0]:    IAP操作状态码,IAP返回值     
** Returned value:      paramout[0]:    IAP操作状态码,IAP返回值                     
*********************************************************************************************************/
uint32_t  sectorPrepare (uint8_t sec1, uint8_t sec2)
{  
    paramin[0] = IAP_Prepare;                                           /* 设置命令字                   */
    paramin[1] = sec1;                                                  /* 设置参数                     */
    paramin[2] = sec2;                            
    (*IAP_Entry)(paramin, paramout);                                    /* 调用IAP服务程序              */
   
    return (paramout[0]);                                               /* 返回状态码                   */
}

/*********************************************************************************************************
** Function name:       ramCopy
** Descriptions:        复制RAM的数据到FLASH，命令代码51
** input parameters:    dst:            目标地址，即FLASH起始地址。以512字节为分界
**                      src:            源地址，即RAM地址。地址必须字对齐
**                      no:             复制字节个数，为512/1024/4096/8192
** output parameters:   paramout[0]:    IAP操作状态码,IAP返回值     
** Returned value:      paramout[0]:    IAP操作状态码,IAP返回值                     
*********************************************************************************************************/
uint32_t  ramToFlash (uint32_t dst, uint32_t src, uint32_t no)
{  
    paramin[0] = IAP_RAMTOFLASH;                                        /* 设置命令字                   */
    paramin[1] = dst;                                                   /* 设置参数                     */
    paramin[2] = src;
    paramin[3] = no;
    paramin[4] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);                                    /* 调用IAP服务程序              */
    
    return (paramout[0]);                                               /* 返回状态码                   */
}

/*********************************************************************************************************
** Function name:       sectorErase
** Descriptions:        扇区擦除，命令代码52
** input parameters:    sec1            起始扇区
**                      sec2            终止扇区92
** output parameters:   paramout[0]:    IAP操作状态码,IAP返回值
** Returned value:      paramout[0]:    IAP操作状态码,IAP返回值                     
*********************************************************************************************************/
uint32_t  sectorErase (uint8_t sec1, uint8_t sec2)
{  
    paramin[0] = IAP_ERASESECTOR;                                       /* 设置命令字                   */
    paramin[1] = sec1;                                                  /* 设置参数                     */
    paramin[2] = sec2;
    paramin[3] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);                                    /* 调用IAP服务程序              */
   
    return (paramout[0]);                                               /* 返回状态码                   */
}

/*********************************************************************************************************
** Function name:       blankChk
** Descriptions:        扇区查空，命令代码53
** input parameters:    sec1:           起始扇区
**                      sec2:           终止扇区92
** output parameters:   paramout[0]:    IAP操作状态码,IAP返回值
** Returned value:      paramout[0]:    IAP操作状态码,IAP返回值                     
*********************************************************************************************************/
uint32_t  blankChk (uint8_t sec1, uint8_t sec2)
{  
    paramin[0] = IAP_BLANKCHK;                                          /* 设置命令字                   */
    paramin[1] = sec1;                                                  /* 设置参数                     */
    paramin[2] = sec2;
    (*IAP_Entry)(paramin, paramout);                                    /* 调用IAP服务程序              */

    return (paramout[0]);                                               /* 返回状态码                   */
}

/*********************************************************************************************************
** Function name:       parIdRead
** Descriptions:        扇区查空，命令代码54
** input parameters:    无
** output parameters:   paramout[0]:    IAP操作状态码,IAP返回值
** Returned value:      paramout[0]:    IAP操作状态码,IAP返回值                     
*********************************************************************************************************/
uint32_t  parIdRead (void)
{  
    paramin[0] = IAP_READPARTID;                                        /* 设置命令字                   */
    (*IAP_Entry)(paramin, paramout);                                    /* 调用IAP服务程序              */

    return (paramout[0]);                                               /* 返回状态码                   */
}

/*********************************************************************************************************
** Function name:       codeIdBoot
** Descriptions:        扇区查空，命令代码55
** input parameters:    无
** output parameters:   paramout[0]:    IAP操作状态码,IAP返回值
** Returned value:      paramout[0]:    IAP操作状态码,IAP返回值                     
*********************************************************************************************************/
uint32_t  codeIdBoot (void)
{  
    paramin[0] = IAP_BOOTCODEID;                                        /* 设置命令字                   */
    (*IAP_Entry)(paramin, paramout);                                    /* 调用IAP服务程序              */

    return (paramout[0]);                                               /* 返回状态码                   */
}

/*********************************************************************************************************
** Function name:       dataCompare
** Descriptions:        校验数据，命令代码56
** input parameters:    dst:            目标地址，即RAM/FLASH起始地址。地址必须字对齐
**                      src:            源地址，即FLASH/RAM地址。地址必须字对齐
**                      no:             复制字节个数，必须能被4整除
** output parameters:   paramout[0]:    IAP操作状态码,IAP返回值
** Returned value:      paramout[0]:    IAP操作状态码,IAP返回值                     
*********************************************************************************************************/
uint32_t  dataCompare (uint32_t dst, uint32_t src, uint32_t no)
{  
    paramin[0] = IAP_COMPARE;                                           /* 设置命令字                   */
    paramin[1] = dst;                                                   /* 设置参数                     */
    paramin[2] = src;
    paramin[3] = no;
    (*IAP_Entry)(paramin, paramout);                                    /* 调用IAP服务程序              */

    return (paramout[0]);                                               /* 返回状态码                   */
}

/*********************************************************************************************************
** Function name:			begin_update
** Descriptions:			开始升级时函数
** Input parameters:	无
** Output parameters:	无
** Returned value:		0失败 1成功
*********************************************************************************************************/
void begin_update(void)
{
	SysUpdateFlag = 1;
	update_len = 0;
	update_last_remain_len = 0;
	IAP_Entry = (void(*)())IAP_ENTER_ADR;
	OS_ENTER_CRITICAL();
	sectorPrepare(SECTORDOWNLOAD1, SECTORDOWNLOAD2);
	sectorErase(SECTORDOWNLOAD1, SECTORDOWNLOAD2);
	OS_EXIT_CRITICAL();
}

/*********************************************************************************************************
** Function name:			process_update
** Descriptions:			升级过程函数
** Input parameters:	pdat用户数据 flen待写入的用户数据字节数
** Output parameters:	无
** Returned value:		成功返回0 否则返回1
*********************************************************************************************************/
uint8_t process_update(uint32_t total_len,uint8_t *pdat,uint32_t flen)
{
	uint8_t sectoroffset;
	uint32_t sectoraddress;
	
	if(SysUpdateFlag != 1)
		return UPERR;

	while(flen){
		if((update_last_remain_len + flen) >= 512){
			memcpy(GucIapTmp+update_last_remain_len, pdat, 512 - update_last_remain_len);
			flen = flen - (512 - update_last_remain_len);
			pdat = pdat + (512 - update_last_remain_len);
			update_last_remain_len = 512;
		}else{
			memcpy(GucIapTmp + update_last_remain_len, pdat, flen);
			update_last_remain_len += flen;
			flen = 0;
		}
	
		if(update_last_remain_len == 512){
			sectoroffset = update_len / SECTORSIZE + SECTORDOWNLOAD1;  // 计算扇区偏移
			sectoraddress = DOWNLOADADDR + update_len;
			OS_ENTER_CRITICAL();
			sectorPrepare(sectoroffset, sectoroffset);
			ramToFlash(sectoraddress, (uint32_t)GucIapTmp, 512);
			if(0 != dataCompare((uint32_t)GucIapTmp, sectoraddress, 512)){
				OS_EXIT_CRITICAL();
				return UPERR;
			}
			OS_EXIT_CRITICAL();
			update_len += 512;
			update_last_remain_len = 0;
		}
	}
	if((update_last_remain_len != 0) && ((update_last_remain_len + update_len) >= total_len)){//升级文件已传输完成？
		memset(GucIapTmp+update_last_remain_len, 0xff, 512 - update_last_remain_len);
		sectoroffset = update_len / SECTORSIZE+SECTORDOWNLOAD1;
		sectoraddress = DOWNLOADADDR + update_len;
		OS_ENTER_CRITICAL();
		sectorPrepare(sectoroffset, sectoroffset);
		ramToFlash(sectoraddress,(uint32_t)GucIapTmp, 512);
		if(0 != dataCompare((uint32_t)GucIapTmp, sectoraddress, 512)){
			OS_EXIT_CRITICAL();
			return UPERR;
		}
		OS_EXIT_CRITICAL();
		update_len += update_last_remain_len;
		update_last_remain_len = 0;
	}
	
	return UPOK;
}

/*********************************************************************************************************
** Function name:	    	end_update
** Descriptions:	    	结束升级时函数
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void end_update(void)
{
	memset(GucIapTmp, 0xff, sizeof(GucIapTmp));
	memset(GucIapTmp, 0x5a, 4);
	OS_ENTER_CRITICAL();
	sectorPrepare(SECTORFLAG, SECTORFLAG);				/* 扇区准备	 */
	sectorErase(SECTORFLAG, SECTORFLAG);					/* 擦除扇区	 */
	sectorPrepare(SECTORFLAG, SECTORFLAG);				/* 选择扇区	 */
	ramToFlash(USERFLAGADDR,(uint32_t)GucIapTmp, 256);	/* 编程FLASH */
	OS_EXIT_CRITICAL();
}


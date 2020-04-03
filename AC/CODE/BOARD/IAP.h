#ifndef __IAP_H
#define	__IAP_H

#include <stdint.h>
#include <string.h>

#define UPOK			0
#define UPERR			1

#define IAP_FCCLK				(96000)			//定义CCLK值大小，单位为KHz 
#define IAP_ENTER_ADR		0x1FFF1FF1	//IAP入口地址定义

#define SECTORSIZE			(32*1024)		//扇区大小
 
//定义IAP命令字                                  
#define	IAP_Prepare				50		/* 选择扇区                     */
#define	IAP_RAMTOFLASH		51		/* 拷贝数据 FLASH目标地址       */
#define	IAP_ERASESECTOR		52		/* 擦除扇区    【起始扇区号     */
#define	IAP_BLANKCHK			53		/* 查空扇区    【起始扇区号、   */
#define	IAP_READPARTID		54		/* 读器件ID    【无】           */
#define	IAP_BOOTCODEID		55		/* 读Boot版本号【无】           */
#define	IAP_COMPARE				56		/* 比较命令    【Flash起始地址  */

#define USERFLAGADDR		0x0000F000	//用户程序标志区
#define DOWNLOADADDR		0x00048000	//下载区首地址

#define SECTORFLAG			15	//用户程序标志区扇区
#define SECTORDOWNLOAD1	23	//下载区开始扇区
#define SECTORDOWNLOAD2	29	//下载区结束扇区

extern void begin_update(void);
extern uint8_t process_update(uint32_t total_len,uint8_t *pdat,uint32_t flen);
extern void end_update(void);

#endif

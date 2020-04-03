#ifndef __XRD_H
#define __XRD_H
#include "stdio.h"
#include <stdint.h>

#define LCDDATAMAXLEN   512
typedef struct{
	uint8_t ReBuf[LCDDATAMAXLEN];
	uint16_t RecvWr;
	uint16_t RecvRd;
}LCDINFO;


#define PACKHEAD1 	0x5a
#define PACKHEAD2	  0xa5
#define WRITE_CMD       	0x80
#define READ_CMD          0x81
#define WRITE_VAR_CMD     0x82
#define READ_VAR_CMD      0x83 
#define PIC_ADDR          0x03

static uint8_t PacketHead[3]={PACKHEAD1,PACKHEAD2,'\0'};


#define KEY_RETURN   0x800             //按键返回值得地址
#define KEY_INPUT    0x790             //按键返回值得地址

#define PAGE0       0x00 
#define PAGE1       0x01
#define PAGE2       0x02
#define PAGE3       0x03
#define PAGE4       0x04
#define PAGE5       0x05
#define PAGE6       0x06
#define PAGE7       0x07
#define PAGE8       0x08
#define PAGE9       0x09
#define PAGE10      0x0A
#define PAGE11      0x0B
#define PAGE12      0x0C
#define PAGE13      0x0D
#define PAGE14      0x0E
#define PAGE15      0x0F
#define PAGE16      0x10
#define PAGE17      0x11
#define PAGE18      0x12
#define PAGE19      0x13
#define PAGE20      0x14
#define PAGE21      0x15
#define PAGE22      0x16
#define PAGE23      0x17
#define PAGE24      0x18
#define PAGE25      0x19
#define PAGE26      0x1A 
#define PAGE27      0x1B 
#define PAGE28      0x1C 
#define PAGE29      0x1D
#define PAGE30      0x1E


#define PAGE_All_TIME        0x690            //时间
#define PAGE_ON_LINEFLAG     0x6A8            //在线显示

#define PAGE_A_ERRID         0x720            // A故障ID
#define PAGE_B_ERRID         0x730            // B故障ID

#define PAGE0_VER_ADDR      0x700							//版本号
#define PAGE0_DAY_ADDR      0x720							//激活码
#define PAGE1_A_QR     			0x40
#define PAGE1_A_DEV_NUM  		0x00
#define PAGE1_A_CH_FLAG     0x468             //插枪标识

#define PAGE2_A_POWER       0x400             //A当前电量
#define PAGE2_A_COST        0x410             //A当前花费
#define PAGE2_A_I_A         0x418             //A电流A相
#define PAGE2_A_I_B         0x808           	//A电流B项
#define PAGE2_A_I_C         0x818         	  //A电流C项
#define PAGE2_A_VOLTAGE     0x420             //A电压
#define PAGE2_A_VOLTAGE_B   0x800             //A电压
#define PAGE2_A_VOLTAGE_C   0x810             //A电压
#define PAGE2_A_CH_TIME     0x448             //A充电时间


#define PAGE3_A_STOP_REASON   0x428             //A停止原因
#define PAGE3_A_CHARG_POWER   0x470            //A 充电电量
#define PAGE3_A_CHARG_MONEY   0x478            //A 花费金额
#define PAGE3_A_CHARG_PROMPT  0x480            //A停止提示


#define PAGE4_A_QR           0x40              //A二维码
#define PAGE4_A_DEV_NUM      0x00              //A设备号
#define PAGE4_A_CH_FLAG      0x468             //A插枪标识

#define PAGE4_B_QR           0x200             //B二维码
#define PAGE4_B_DEV_NUM      0x20              //B设备号
#define PAGE4_B_CH_FLAG      0x568             //B插枪标识


#define PAGE5_A_POWER       0x400             //A当前电量
#define PAGE5_A_COST        0x410             //A当前花费
#define PAGE5_A_I    		    0x418             //A电流
#define PAGE5_A_VOLTAGE     0x420             //A电压
#define PAGE5_A_CH_TIME     0x448             //A充电时间

#define PAGE5_B_QR           0x200             //B二维码
#define PAGE5_B_DEV_NUM      0x20              //B设备号
#define PAGE5_B_CH_FLAG      0x568             //B插枪标识


#define PAGE6_A_STOP_REASON   0x428             //A停止原因
#define PAGE6_A_STOP_REASON   0x428             //A停止原因
#define PAGE6_A_CHARG_POWER   0x470             //A 充电电量
#define PAGE6_A_CHARG_MONEY   0x478             //A 花费金额
#define PAGE6_A_CHARG_PROMPT  0x480             //A停止提示

#define PAGE6_B_QR           0x200             //B二维码
#define PAGE6_B_DEV_NUM      0x20              //B设备号
#define PAGE6_B_CH_FLAG      0x568             //B插枪标识

#define PAGE7_A_QR           0x40              //A二维码
#define PAGE7_A_DEV_NUM      0x00              //A设备号
#define PAGE7_A_CH_FLAG      0x468             //A插枪标识

#define PAGE7_B_POWER        0x500             //B当前电量
#define PAGE7_B_COST         0x510             //B当前花费
#define PAGE7_B_I            0x518             //B电流A
#define PAGE7_B_I_B          0x828           	 //B电流B
#define PAGE7_B_I_C          0x838             //B电流C
#define PAGE7_B_VOLTAGE      0x520             //B电压A
#define PAGE7_B_VOLTAGE_B    0x820             //B电压B
#define PAGE7_B_VOLTAGE_C    0x830             //B电压C
#define PAGE7_B_CH_TIME      0x548             //B充电时间

#define PAGE8_A_QR           0x40              //A二维码
#define PAGE8_A_DEV_NUM      0x00              //A设备号
#define PAGE8_A_CH_FLAG      0x468             //A插枪标识
#define PAGE8_B_STOP_REASON  0x528             //B停止原因
#define PAGE8_B_CHARG_POWER  0x570            //B充电电量
#define PAGE8_B_CHARG_MONEY  0x578            //B 花费金额
#define PAGE8_B_CHARG_PROMPT 0x580            //B停止提示

#define PAGE9_A_STOP_REASON   0x428             //A停止原因
#define PAGE9_A_STOP_REASON   0x428             //A停止原因
#define PAGE9_A_CHARG_POWER   0x470            //A 充电电量
#define PAGE9_A_CHARG_MONEY   0x478            //A 花费金额
#define PAGE9_A_CHARG_PROMPT  0x480            //A停止提示

#define PAGE9_B_POWER        0x500             //B当前电量
#define PAGE9_B_COST         0x510             //B当前花费
#define PAGE9_B_I            0x518             //B电流
#define PAGE9_B_VOLTAGE      0x520             //B电压
#define PAGE9_B_CH_TIME      0x548             //B充电时间

#define PAGE10_A_POWER       0x400             //A当前电量
#define PAGE10_A_COST        0x410             //A当前花费
#define PAGE10_A_I           0x418             //A电流
#define PAGE10_A_VOLTAGE     0x420             //A电压
#define PAGE10_A_CH_TIME     0x448             //A充电时间
#define PAGE10_B_STOP_REASON 0x528             //B停止原因
#define PAGE10_B_CHARG_POWER   0x570            //B充电电量
#define PAGE10_B_CHARG_MONEY   0x578            //B 花费金额
#define PAGE10_B_CHARG_PROMPT  0x580            //B停止提示


#define PAGE11_A_POWER       0x400             //A当前电量
#define PAGE11_A_COST        0x410             //A当前花费
#define PAGE11_A_I           0x418             //A电流
#define PAGE11_A_VOLTAGE     0x420             //A电压
#define PAGE11_A_CH_TIME     0x448             //A充电时间

#define PAGE11_B_POWER       0x500             //B当前电量
#define PAGE11_B_COST        0x510             //B当前花费
#define PAGE11_B_I           0x518             //B电流
#define PAGE11_B_VOLTAGE     0x520             //B电压
#define PAGE11_B_CH_TIME     0x548             //B充电时间

#define PAGE12_A_STOP_REASON  0x428             //A停止原因
#define PAGE12_A_STOP_REASON   0x428             //A停止原因
#define PAGE12_A_CHARG_POWER   0x470            //A 充电电量
#define PAGE12_A_CHARG_MONEY   0x478            //A 花费金额
#define PAGE12_A_CHARG_PROMPT  0x480            //A停止提示

#define PAGE12_B_STOP_REASON   0x528            //B停止原因
#define PAGE12_B_CHARG_POWER   0x570            //B充电电量
#define PAGE12_B_CHARG_MONEY   0x578            //B 花费金额
#define PAGE12_B_CHARG_PROMPT  0x580            //B停止提示

#define PAGE13_AB_POWER      0x600             //AB当前电量
#define PAGE13_AB_REMAIN     0x608             //余额
#define PAGE13_AB_CH_TIME    0x610             //总耗时
#define PAGE13_AB_DATE       0x620             //日期
#define PAGE13_AB_COST       0x6A0             //花费金额

#define PAGE15_AB_ERR        0x640             //电桩故障

#define PAGE16_AB_LOCK          0x670             //电桩锁定
#define PAGE17_AB_FAILURE_ID    0x650             //失败ID
#define PAGE19_AB_CURRENT_MONEY 0x6B0            //当前金额
#define PAGE21_AB_MONEY         0x700			//验证成功余额
#define PAGE22_AB_FAILURE       0x708			//验证失败
#define PAGE23_TIME             0x710           //启动时间 



#define DRAW_IOC              1                //显示图标
#define DRAW_IOC_NO           0                //不显示图标


/////////////////////////////// 长度
#define PAGE_All_TIME_LEN         16             //时间长度
#define PAGE_ON_LINEFLAG_LEN      8             //在线显示长度

#define PAGE_A_ERRID_LEN          16
#define PAGE_B_ERRID_LEN          16

#define PAGE0_VER_LEN            32
#define PAGE0_DAY_ADDR_LEN       32
#define PAGE1_A_QR_LEN     			128
#define PAGE1_A_DEV_NUM_LEN  		32
#define PAGE1_A_CH_FLAG_LEN     8             //插枪标识长度

#define PAGE2_A_POWER_LEN       8             //A当前电量长度
#define PAGE2_A_COST_LEN        8             //A当前花费长度
#define PAGE2_A_I_LEN           8             //A电流长度
#define PAGE2_A_VOLTAGE_LEN     8             //A电压长度
#define PAGE2_A_CH_TIME_LEN     32            //A充电时间长度


#define PAGE3_A_STOP_REASON_LEN   32           //停止原因长度
#define PAGE3_A_CHARG_POWER_LEN   8            //A 充电电量
#define PAGE3_A_CHARG_MONEY_LEN   8            //A 花费金额
#define PAGE3_A_CHARG_PROMPT_LEN  32           //A停止提示

#define PAGE4_A_QR_LEN           128             //A二维码长度
#define PAGE4_A_DEV_NUM_LEN      32              //A设备号长度
#define PAGE4_A_CH_FLAG_LEN      8               //A插枪标识长度

#define PAGE4_B_QR_LEN           128             //B二维码长度
#define PAGE4_B_DEV_NUM_LEN      32              //B设备号长度
#define PAGE4_B_CH_FLAG_LEN      8               //B插枪标识长度


#define PAGE5_A_POWER_LEN       8             //A当前电量长度
#define PAGE5_A_COST_LEN        8             //A当前花费长度
#define PAGE5_A_I_LEN    		    8             //A电流长度
#define PAGE5_A_VOLTAGE_LEN     8             //A电压长度
#define PAGE5_A_CH_TIME_LEN     32            //A充电时间长度

#define PAGE5_B_QR_LEN           128             //B二维码长度
#define PAGE5_B_DEV_NUM_LEN      32              //B设备号长度
#define PAGE5_B_CH_FLAG_LEN      8             //B插枪标识长度


#define PAGE6_A_STOP_REASON_LEN   32           //A停止原因长度
#define PAGE6_A_CHARG_POWER_LEN   8            //A 充电电量
#define PAGE6_A_CHARG_MONEY_LEN   8            //A 花费金额
#define PAGE6_A_CHARG_PROMPT_LEN  32           //A停止提示

#define PAGE6_B_QR_LEN            128           //B二维码长度
#define PAGE6_B_DEV_NUM_LEN       32            //B设备号长度
#define PAGE6_B_CH_FLAG_LEN       8             //B插枪标识长度

#define PAGE7_A_QR_LEN            128           //A二维码长度
#define PAGE7_A_DEV_NUM_LEN       32            //A设备号长度
#define PAGE7_A_CH_FLAG_LEN       8             //A插枪标识长度

#define PAGE7_B_POWER_LEN         8             //B当前电量长度
#define PAGE7_B_COST_LEN          8             //B当前花费长度
#define PAGE7_B_I_LEN             8             //B电流长度
#define PAGE7_B_VOLTAGE_LEN       8             //B电压长度
#define PAGE7_B_CH_TIME_LEN       32            //B充电时间长度

#define PAGE8_A_QR_LEN            128              //A二维码长度
#define PAGE8_A_DEV_NUM_LEN       32            //A设备号长度
#define PAGE8_A_CH_FLAG_LEN       8             //A插枪标识长度
#define PAGE8_B_STOP_REASON_LEN   32             //B停止原因长度
#define PAGE8_B_CHARG_POWER_LEN   8            //B 充电电量长度
#define PAGE8_B_CHARG_MONEY_LEN   8            //B 花费金额长度
#define PAGE8_B_CHARG_PROMPT_LEN  32           //B 停止提示长度

#define PAGE9_A_STOP_REASON_LEN    32          //A停止原因长度
#define PAGE9_A_CHARG_POWER_LEN   8            //A 充电电量
#define PAGE9_A_CHARG_MONEY_LEN   8            //A 花费金额
#define PAGE9_A_CHARG_PROMPT_LEN  32           //A停止提示

#define PAGE9_B_POWER_LEN         8             //B当前电量长度
#define PAGE9_B_COST_LEN          8             //B当前花费长度
#define PAGE9_B_I_LEN             8             //B电流长度
#define PAGE9_B_VOLTAGE_LEN       8             //B电压长度
#define PAGE9_B_CH_TIME_LEN       32            //B充电时间长度

#define PAGE10_A_POWER_LEN        8             //A当前电量长度
#define PAGE10_A_COST_LEN         8             //A当前花费长度
#define PAGE10_A_I_LEN            8             //A电流长度
#define PAGE10_A_VOLTAGE_LEN      8             //A电压长度
#define PAGE10_A_CH_TIME_LEN      32            //A充电时间长度

#define PAGE10_B_STOP_REASON_LEN   32            //B停止原因长度
#define PAGE10_B_CHARG_POWER_LEN   8            //B 充电电量长度
#define PAGE10_B_CHARG_MONEY_LEN   8            //B 花费金额长度
#define PAGE10_B_CHARG_PROMPT_LEN  32           //B 停止提示长度

#define PAGE11_A_POWER_LEN        8            //A当前电量长度
#define PAGE11_A_COST_LEN         8            //A当前花费长度
#define PAGE11_A_I_LEN            8            //A电流长度
#define PAGE11_A_VOLTAGE_LEN      8            //A电压长度
#define PAGE11_A_CH_TIME_LEN      32           //A充电时间长度

#define PAGE11_B_POWER_LEN        8             //B当前电量长度
#define PAGE11_B_COST_LEN         8             //B当前花费长度
#define PAGE11_B_I_LEN            8             //B电流长度
#define PAGE11_B_VOLTAGE_LEN      8             //B电压长度
#define PAGE11_B_CH_TIME_LEN      32             //B充电时间长度

#define PAGE12_A_STOP_REASON_LEN   32           //A停止原因长度
#define PAGE12_A_CHARG_POWER_LEN   8            //A 充电电量
#define PAGE12_A_CHARG_MONEY_LEN   8            //A 花费金额
#define PAGE12_A_CHARG_PROMPT_LEN  32           //A停止提示
#define PAGE12_B_STOP_REASON_LEN   32           //B停止原因长度
#define PAGE12_B_CHARG_POWER_LEN   8            //B 充电电量长度
#define PAGE12_B_CHARG_MONEY_LEN   8            //B 花费金额长度
#define PAGE12_B_CHARG_PROMPT_LEN  32           //B 停止提示长度

#define PAGE13_AB_POWER_LEN       8             //AB当前电量长度
#define PAGE13_AB_REMAIN_LEN      8             //余额长度
#define PAGE13_AB_CH_TIME_LEN     16             //总耗时长度
#define PAGE13_AB_DATE_LEN        32             //日期长度
#define PAGE13_AB_COST_LEN        8             //花费金额长度

#define PAGE15_AB_ERR_LEN         16             //电桩故障长度

#define PAGE16_AB_LOCK_LEN           32             //电桩锁定长度
#define PAGE17_AB_FAILURE_ID_LEN     16             //失败ID长度
#define PAGE19_AB_CURRENT_MONEY_LEN  8           //当前金额长度  
#define PAGE21_AB_MONEY_LEN          8			//验证成功余额长度
#define PAGE22_AB_FAILURE_LEN        32
#define PAGE23_TIME_LEN              8 



#define PAGE28_BASE_RECORD_ID            0x900
#define PAGE28_BASE_RECORD_STARTTIME     0x910
#define PAGE28_BASE_RECORD_CH_TIME       0x930
#define PAGE28_BASE_RECORD_CH_POWER      0x950
#define PAGE28_BASE_RECORD_CH_MONEY      0x958
#define PAGE28_BASE_RECORD_ERR_CODE      0x960
#define PAGE28_STEP_LENGTH               0x80

#define PAGE28_BASE_RECORD_ID_LEN            16
#define PAGE28_BASE_RECORD_STARTTIME_LEN     32
#define PAGE28_BASE_RECORD_CH_TIME_LEN       32
#define PAGE28_BASE_RECORD_CH_POWER_LEN      8
#define PAGE28_BASE_RECORD_CH_MONEY_LEN      8
#define PAGE28_BASE_RECORD_ERR_CODE_LEN      8

#define PAGE29_CARDID_ADDR       0x0C00
#define PAGE29_STARTTYPE_ADDR    0x0C10
#define PAGE29_SUMMONEY_ADDR     0x0C20
#define PAGE29_STARTSOC_ADDR     0x0C28
#define PAGE29_STARTTIME_ADDR    0x0C30
#define PAGE29_SUMTIME_ADDR      0x0C50
#define PAGE29_CHGPORT_ADDR      0x0C60
#define PAGE29_SUMENERGY_ADDR    0x0C68
#define PAGE29_STOPCAUSE_ADDR    0x0C70  
#define PAGE29_STOPSOC_ADDR      0x0C78
#define PAGE29_STOPTIME_ADDR     0x0C80
#define PAGE29_STARTMONEY_ADDR   0x0CA0
#define PAGE30_UPDATA_PERSENT_ADDR     0x0700
#define PAGE30_UPDATA_TIP_ADDR  0x0708

#define PAGE29_CARDID_ADDR_LEN       16
#define PAGE29_STARTTYPE_ADDR_LEN    16
#define PAGE29_SUMMONEY_ADDR_LEN      8
#define PAGE29_STARTSOC_ADDR_LEN      8
#define PAGE29_STARTTIME_ADDR_LEN     32
#define PAGE29_SUMTIME_ADDR_LEN       16
#define PAGE29_CHGPORT_ADDR_LEN       8
#define PAGE29_SUMENERGY_ADDR_LEN     8
#define PAGE29_STOPCAUSE_ADDR_LEN     8  
#define PAGE29_STOPSOC_ADDR_LEN       8
#define PAGE29_STOPTIME_ADDR_LEN      32
#define PAGE29_STARTMONEY_ADDR_LEN    8
#define PAGE30_UPDATA_PERSENT_ADDR_LEN  8
#define PAGE30_UPDATA_TIP_ADDR_LEN    8


#define SHOW_LOG             0x0401
#define RETURN_BUTTON        0x0700   
#define OK_BUTTON            0x0790
#define NEXT_PAGE_BUTTON     0x0710
#define PRE_PAGR_BUTTON      0x0711

#define LINE1_BUTTON         0x0712
#define LINE2_BUTTON         0x0713
#define LINE3_BUTTON         0x0714
#define LINE4_BUTTON         0x0715
#define LINE5_BUTTON         0x0716












#define PAGE3_B_MORE         0x0301
#define PAGE5_B_MORE         0x0501
#define PAGE7_A_MORE         0x0701
#define PAGE8_A_MORE         0x0801
#define PAGE9_A_MORE         0x0901
#define PAGE9_B_MORE         0x0902

#define PAGE11_OK            0x0790                  //数据录入地址

#define PAGE12_BACK          0x0C01
#define PAGE13_START_WORK    0x0D01
#define PAGE13_AUTO_SW       0x0D02
#define PAGE13_12V           0x0D03
#define PAGE13_24V           0x0D04
#define PAGE13_AUTO_FULL     0x0D05
#define PAGE13_MONEY         0x0D06
#define PAGE13_POWER         0x0D07
#define PAGE13_TIME          0x0D08

#define PAGE21_MORE          0x1501


void XRD_Load_Page(uint16_t page);// 切换图片
void XRD_Write_Addr(uint16_t addr, char* str, uint8_t slen);
void XRD_Draw_Ioc(uint16_t addr, uint16_t num);
uint8_t GetTouchInfo(uint8_t* strbuf, uint8_t strlen, uint16_t* keyfunc);

void XRDClearInput(void);

#endif



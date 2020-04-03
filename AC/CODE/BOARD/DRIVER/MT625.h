#ifndef _MT625_H
#define _MT625_H



#define VIF_REG_SECTOR		0x01 //注册信息扇区号
#define VIF_DATA_SECTOR		0x02 //验证数据扇区号
#define VIF_REG_BLOCK			0x00 //注册信息块号
#define VIF_DATA_BLOCK    0x00 //0x00~0x03  验证块序号
#define VIF_BLOCK_NUM_BA   0x01 //0x00~0x03


typedef struct 
{
	uint8_t type;     	// 命令字
	uint8_t cmd; 			// 命令参数字
	uint8_t buflen; 		// 接受到的数据长度
	uint8_t buf[128];		// 接受到的数据
}MTRcvMsg;


typedef struct 
{
	uint8_t    sectorx;   //扇区号
    uint8_t    bldata[16];
	
}M1BLOCK;





typedef struct{
	uint8_t KeyA[6];   //验证B密码
	uint8_t KeyB[6];   //验证A密码

	uint32_t cardID;//卡ID  
	uint8_t type;//类型 0x01:离线卡 0x02:在线卡
	uint8_t lockst;//卡状态 0x01:正常 0x02:灰锁
	uint32_t Money;//余额
	uint8_t recdInx;//6充电记录号
	uint8_t rechargeInx;//充值记录号
	uint8_t PileNumber[4];//桩编号后4位
}M1CARDVIF;//m1卡验证块读取

typedef struct 
{
    uint8_t    sectorx; //扇区号
    
	uint8_t    KeyA[6]; //验证B密码
	uint8_t    KeyB[6]; //验证A密码
	uint32_t   cardID;
	
  //卡片记录充电
    uint8_t   deviceNoH;  //1桩编号      0xFFFF
	uint8_t   deviceNoL;
	uint32_t 	timestpA;	//3~6时间戳      0xFFFFFFFF
	uint32_t 	timestpB;	//3~6时间戳      0xFFFFFFFF
	uint32_t 	timestpC;	//3~6时间戳      0xFFFFFFFF
	uint32_t 	timestpD;	//3~6时间戳      0xFFFFFFFF
	uint16_t 	usepowerA;	//7~9使用电量    0x00FFFFFF 
    uint16_t 	usepowerB;	//7~9使用电量    0x00FFFFFF 
    uint16_t 	usepowerC;	//7~9使用电量    0x00FFFFFF 	
	uint32_t  conmoneyA;	//10~12使用金额  0x00FFFFFF    10000.00 元，保留两位小数
	uint32_t  conmoneyB;	//10~12使用金额  0x00FFFFFF    10000.00 元，保留两位小数
	uint32_t  conmoneyC;	//10~12使用金额  0x00FFFFFF    10000.00 元，保留两位小数
    uint8_t   rfu[3];     //13~15预留 
	uint8_t   crc;        //16           crc8
		
}M1CARDREC;//m1卡记录




#define    TYPE_START               0x01	    // 开始
#define    TYPE_END					0x02	    // 结束

#define  	MT_CMD_M1_FIND   		0x30        // 寻卡M1卡
#define  	MT_CMD_M1_READID   	    0x31        // 获取M1卡号
#define  	MT_CMD_M1_VIFKEYA       0x32        // 验证M1卡号A密码
#define  	MT_CMD_M1_VIFKEYB       0x39        // 验证M1卡号B密码

#define  	MT_CMD_M1_BLOCKRD       0x33        // 读取块内容
#define  	MT_CMD_M1_BLOCKWR       0x34        // 写块内容

#define 	MT_STX 					0x02        // 头
#define 	MT_ETX	 				0x03	    // 尾

#define     MT_CMD_TYPE1   			0x51   	    // 类别1
#define     MT_CMD_TYPE2        	0x34        // 类别2

#define  	MT_CMD_CARDFIND   		0x37        // 寻卡
#define  	MT_CMD_CARDREADINFO		0x36        // 读卡信息
#define  	MT_CMD_CARDSTART  		0x30	    // 开始加电
#define  	MT_CMD_CARDEND			0x34        // 结束加电
#define  	MT_CMD_CARDSUPPLY 		0x35        // 补充交易
#define  	MT_CMD_CARDREADID		0x56        // 读卡ID
#define  	MT_CMD_CARDCLOSEANT		0x42        // 关闭射频天线


#define     MT_ACK_OK        		0x59        // 正确
#define     MT_ACK_FAIL				0x4e        // 失败
#define     MT_ACK_NOCARD          	0x45        // 无卡
#define     MT_ACK_PINERR			0x83	    // 个人PIN失败
#define     MT_ACK_CARDERR1			0x23        // 卡片不适配
#define     MT_ACK_CARDERR2      	0x47        // psam复位失败
#define     MT_ACK_CARDERR3        	0x48	    // 用户卡复位失败





void MTInit(void);
uint8_t MTSearch(uint32_t *UID);
uint8_t MTRead(M1CARDVIF *vifdata);
uint8_t MTWrite(M1CARDVIF *vifdata);


#endif

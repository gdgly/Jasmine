#ifndef _CAN_H
#define _CAN_H

#define DEV_CAN1 0
#define DEV_CAN2 1

#define CAN_MSG_NUMBER 100

typedef enum {
	DATA_FRAME = 0,
	REMOTE_FRAME = 1
} CAN_FRAME_Type;

typedef enum {
	STD_ID_FORMAT = 0,
	EXT_ID_FORMAT = 1
} CAN_ID_FORMAT_Type;

typedef struct {
	uint32_t id;
	uint8_t dataA[4];
	uint8_t dataB[4];
	uint8_t len;
	uint8_t format;
	uint8_t type;
}CAN_MSG_Type;

typedef	struct {
	uint16_t Wr;
	uint16_t Rd;
	CAN_MSG_Type Msg[CAN_MSG_NUMBER];
}CANBUF;

void CANInit(uint8_t no, uint32_t clk);
uint8_t CANWrite(unsigned char port,uint32_t id,uint8_t data[8],uint8_t len);
uint8_t CANRead(unsigned char port,uint32_t *id,uint8_t data[8]);

#endif


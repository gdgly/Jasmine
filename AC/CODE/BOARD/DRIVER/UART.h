#ifndef _UART_H
#define _UART_H


#define IER_RBR     0x01
#define IER_THRE    0x02
#define IER_RLS     0x04

#define IIR_PEND    0x01
#define IIR_RLS     0x03
#define IIR_RDA     0x02
#define IIR_CTI     0x06
#define IIR_THRE    0x01

#define LSR_RDR     0x01
#define LSR_OE      0x02
#define LSR_PE      0x04
#define LSR_FE      0x08
#define LSR_BI      0x10
#define LSR_THRE    0x20
#define LSR_TEMT    0x40
#define LSR_RXFE    0x80

// 串口号
#define DEV_UART0	       0
#define DEV_UART1	       1
#define DEV_UART2	       2
#define DEV_UART3	       3
#define DEV_UART4		   4	
#define UART_MAX	       5






// 收发缓冲区
#define BUFFER_LEN	       1024    

typedef	struct 
{
	uint16_t Wr;
	uint16_t Rd;
	uint8_t  buf[BUFFER_LEN];
}UARTBUF;


void    UartInit(uint8_t no, uint32_t bps);
uint16_t  UartRead(uint8_t no,uint8_t *data, uint16_t datalen);
uint16_t  UartWrite( uint8_t no, uint8_t *data, uint16_t datalen);
void 	UartFlush( uint8_t no);

void    print( const char *format,...);
void 	printx(uint8_t *data,uint16_t datalen);



#endif

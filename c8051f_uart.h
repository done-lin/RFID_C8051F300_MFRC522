#ifndef __C8051F_UART_H__
#define __C8051F_UART_H__

#define BAUDRATE      4800           // Baud rate of UART in bps
#define UART_BUFFERSIZE 22
#define UART_QTY	1
#define NR_UART0	0

typedef struct PunctureInfo
{
	unsigned char  checksum_2;//1
	unsigned int manufactureSN;//3
	unsigned char  country;//4,China is one
	unsigned char  model;//5
	unsigned char  checksum_3;//6
	unsigned char  intervalTime;//7,in minutes
	unsigned char  reserved_1;//8
	unsigned char   serviceTime;//9,in minutes
	unsigned long lastServiceTime;//13
	unsigned char  reserved_2;//14
	unsigned char  manufactureChecksum;//15,
}PUNCTURE_INFO, *PUNCTURE_INFO_P;

extern void UART_send_str(unsigned char UartNr, unsigned char *ucString);
extern void UART_send_array(unsigned char UartNr, unsigned char *ucData, unsigned char numQty);
extern void UART0_init(void);


extern unsigned char gUARTTxBuffer[UART_QTY][UART_BUFFERSIZE];
extern unsigned char gUARTRxBuffer[UART_QTY][14];
extern unsigned char gUARTBufferSize[UART_QTY];
extern unsigned char gUARTInputFirst[UART_QTY];
extern unsigned char gUARTOutputFirst[UART_QTY];
extern unsigned char gTXReady[UART_QTY];
extern char data gByte[UART_QTY];//the SBUF byte, it tells what byte you are handling
extern unsigned char gCardSn[5];
extern unsigned char gUartRecStatusFlag;
extern unsigned char xorForSendData5A;
extern const PUNCTURE_INFO code myPunctureInfo;
extern unsigned char gI2CBuffer[16];
sbit LEDControlIO = P0^0;
sbit RS485TxEnIO = P0^1;


#endif

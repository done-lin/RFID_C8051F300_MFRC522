#ifndef __C8051F_UART_H__
#define __C8051F_UART_H__

#define BAUDRATE      115200           // Baud rate of UART in bps
#define UART_BUFFERSIZE 16
#define UART_QTY	1
#define NR_UART0	0


extern void UART_send_str(unsigned char UartNr, unsigned char *ucString);
extern void UART_send_array(unsigned char UartNr, unsigned char *ucData, unsigned char numQty);
extern void UART0_init(void);


extern unsigned char gUARTBuffer[UART_QTY][UART_BUFFERSIZE];
extern unsigned char gUARTBufferSize[UART_QTY];
extern unsigned char gUARTInputFirst[UART_QTY];
extern unsigned char gUARTOutputFirst[UART_QTY];
extern unsigned char gTXReady[UART_QTY];
extern char data gByte[UART_QTY];//the SBUF byte, it tells what byte you are handling

#endif

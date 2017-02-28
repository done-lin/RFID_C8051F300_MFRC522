#include <intrins.h>
#include <stdio.h>
#include <string.h>
#include "c8051f300.h"
#include "c8051f_cfg.h"
#include "c8051f_uart.h"
#include "common_i2c.h"
#include "mfrc522.h"

//sbit testing_bit = P0^3;
unsigned char code authentKeyA[2][6]={{0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
	
unsigned char gI2CBuffer[8]={0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72};

void main(void)
{
	unsigned char i,j;

	PCA0MD &= ~0x40;                    // WDTE = 0 (clear watchdog timer
	c8051f_oscillator_init();                  // enable)
	c8051f_port_init();                        // Initialize Port I/O
	UART0_init();
	EA = 1;

	I2C_read_str(0x50, 0xa2, gI2CBuffer, 8);
	//UART_send_array(NR_UART0, gI2CBuffer, 8);
	UART_send_str(NR_UART0, "dduman rfid\n");

	while(gTXReady[NR_UART0]==0){;};

	if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
		UART_send_str(NR_UART0, "testing c8051f\n");//send a string
		}
	while(gTXReady[NR_UART0]==0){;};
	if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
		UART_send_str(NR_UART0, "testing serial\n");//send a string
		}
		
	pcdReset();
	while (1){
	
	if(pcdRequest(gI2CBuffer, &i) == MI_OK){
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "RQSOK\n");
	}
		
	pcdAnticoll(gI2CBuffer);
		
	while(gTXReady[NR_UART0]==0){;};
	UART_send_array(NR_UART0, gI2CBuffer, 6);

	if(pcdSelect(gI2CBuffer) == MI_OK){
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "SlectOK\n");
	}
	
	if( PcdAuthState(PICC_AUTHENT1A, 1, authentKeyA[0], gI2CBuffer)== MI_OK){
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "AuthenOK\n");
	}

		
	for(i=0; i<250; i++){
		for(j=0; j<250; j++){
			_nop_();_nop_();_nop_();
		}
	}
	      
	}                                   // End of while(1)
}                                      
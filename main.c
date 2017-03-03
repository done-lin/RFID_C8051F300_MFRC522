#include <intrins.h>
#include <stdio.h>
#include <string.h>
#include "c8051f300.h"
#include "c8051f_cfg.h"
#include "c8051f_uart.h"
#include "common_i2c.h"
#include "mfrc522.h"

	

//sbit testing_bit = P0^3;
const unsigned char code gAuthentKeyA[2][6]={{0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
	
const unsigned char code gTestWriteData[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x88};
	
unsigned char gI2CBuffer[16]={0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72};


const unsigned char code gNewKeyA[16][6]={
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11},
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11},
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11},
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11},
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11},
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11},
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11},
		{0x11,0x22,0x33,0x44,0x55,0x66},
		{0x66,0x55,0x44,0x33,0x22,0x11}
};

void main(void)
{
	unsigned char i;

	PCA0MD &= ~0x40;                    // WDTE = 0 (clear watchdog timer
	c8051f_oscillator_init();                  // enable)
	c8051f_port_init();                        // Initialize Port I/O
	UART0_init();
	EA = 1;



	if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
		UART_send_str(NR_UART0, "c8051f Start\n");//send a string
		}
		
	while(gTXReady[NR_UART0]==0){;};
	UART_send_str(NR_UART0, "testingUART\n");//send a string

		
	pcd_reset();
		
	while (1)
		{
			if(pcd_request(gI2CBuffer, &i) == MI_OK){
			} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "RQERR\n");
			continue ;
			}
		
		if(pcd_anti_coll(gI2CBuffer) == MI_ERR){
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "AtCollErr\n");
		continue ; 
		}

	if(pcd_select(gI2CBuffer) == MI_OK){
			break;
		} else {
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "SelErr\n");
		continue ;
		}
	}                                   // End of while(1)
	
	
#ifdef DEF_SET_AUTH_KEYA 
	if( pcd_auth_state(PICC_AUTHENT1A, 1, gAuthentKeyA[0], gI2CBuffer)== MI_OK ){
		pcd_set_keyA(3, gAuthentKeyA[0]);
	} else {
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "AuthErr\n");
	}
#else	
	
	if( pcd_auth_state(PICC_AUTHENT1A, 1, gAuthentKeyA[0], gI2CBuffer)== MI_OK ){
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "AuthenOK\n");
			
		
		if(pcd_write(1, &myPunctureInfo.checksum_2) == MI_OK){
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "wrOK\n");
				if(pcd_read(1,gI2CBuffer) == MI_OK){
					
					while(gTXReady[NR_UART0]==0){;};
					UART_send_array(NR_UART0, gI2CBuffer, 16);
				} else {
					while(gTXReady[NR_UART0]==0){;};
					UART_send_str(NR_UART0, "rdErr\n");

				}
					
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "wrErrend\n");
		}
	} else {
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "AuthenEr\n");
	}

#endif
	
	while(1) {;};
	
}                                      

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
	
unsigned char gI2CBuffer[16];


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
	
	//----- puntrueInit ---------//
	xorForSendData5A=0x00;
	RS485TxEnIO = 0;
	LEDControlIO = 1;

	//---- puntrueInit ----------//

	if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
		UART_send_str(NR_UART0, "PuntrueGuid Start\n");//send a string
		}
		
//	while(gTXReady[NR_UART0]==0){;};
//	UART_send_str(NR_UART0, "testingUART\n");//send a string

		
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
	
	for(i=0; i<4; i++){
		gCardSn[i] = gI2CBuffer[i];
	}
	
	/////////////////////
	
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
			
			
			
/////////////////////
		gUARTTxBuffer[NR_UART0][0] = 0x5a;
		gUARTTxBuffer[NR_UART0][1] = gCardSn[0];//card id, last byte. low
		gUARTTxBuffer[NR_UART0][2] = gCardSn[1];
		gUARTTxBuffer[NR_UART0][3] = gCardSn[2];
		gUARTTxBuffer[NR_UART0][4] = gCardSn[3];//card id, High
		gUARTTxBuffer[NR_UART0][5] = (unsigned char)(gCardSn[0]+gCardSn[1]+gCardSn[2]+gCardSn[3]);//checksum1; card id
		gUARTTxBuffer[NR_UART0][6] = myPunctureInfo.checksum_2;
		gUARTTxBuffer[NR_UART0][7] = (unsigned char)myPunctureInfo.manufactureSN;
		gUARTTxBuffer[NR_UART0][8] = (unsigned char)(myPunctureInfo.manufactureSN>>8);
		gUARTTxBuffer[NR_UART0][9] = myPunctureInfo.country;
		gUARTTxBuffer[NR_UART0][10] = myPunctureInfo.model;
		gUARTTxBuffer[NR_UART0][11] = myPunctureInfo.checksum_3;
		gUARTTxBuffer[NR_UART0][12] = myPunctureInfo.intervalTime;
		gUARTTxBuffer[NR_UART0][13] = myPunctureInfo.reserved_1;
		gUARTTxBuffer[NR_UART0][14] = myPunctureInfo.serviceTime;
		gUARTTxBuffer[NR_UART0][15] = (unsigned char)myPunctureInfo.lastServiceTime;
		gUARTTxBuffer[NR_UART0][16] = (unsigned char)(myPunctureInfo.lastServiceTime>>8);
		gUARTTxBuffer[NR_UART0][17] = (unsigned char)(myPunctureInfo.lastServiceTime>>16);
		gUARTTxBuffer[NR_UART0][18] = (unsigned char)(myPunctureInfo.lastServiceTime>>24);
		gUARTTxBuffer[NR_UART0][19] = myPunctureInfo.reserved_2;
		gUARTTxBuffer[NR_UART0][20] = myPunctureInfo.manufactureChecksum;
				
		for(i=0; i<21; i++){
			xorForSendData5A ^= gUARTTxBuffer[NR_UART0][i];
		}
	
		gI2CBuffer[0] = gUARTTxBuffer[NR_UART0][5];//checksum_1
		gI2CBuffer[1] = gUARTTxBuffer[NR_UART0][7];
		gI2CBuffer[2] = gUARTTxBuffer[NR_UART0][8];
		gI2CBuffer[3] = gUARTTxBuffer[NR_UART0][9];
		gI2CBuffer[4] = gUARTTxBuffer[NR_UART0][10];
		gI2CBuffer[5] = gUARTTxBuffer[NR_UART0][12];
		gI2CBuffer[6] = gUARTTxBuffer[NR_UART0][14];
		gI2CBuffer[7] = gUARTTxBuffer[NR_UART0][15];
		gI2CBuffer[8] = gUARTTxBuffer[NR_UART0][16];
		gI2CBuffer[9] = gUARTTxBuffer[NR_UART0][17];
		gI2CBuffer[10] = gUARTTxBuffer[NR_UART0][18];

////////////////////
		
		if(pcd_write(1, &myPunctureInfo.checksum_2) == MI_OK){
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "wrOK\n");
				if(pcd_read(1,gI2CBuffer) == MI_OK){
					
					while(gTXReady[NR_UART0]==0){;};
					//UART_send_array(NR_UART0, gI2CBuffer, 16);
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

		gI2CBuffer[11] = 1;
#endif
	
	while(1) {;};
	
}                                      

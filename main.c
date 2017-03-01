#include <intrins.h>
#include <stdio.h>
#include <string.h>
#include "c8051f300.h"
#include "c8051f_cfg.h"
#include "c8051f_uart.h"
#include "common_i2c.h"
#include "mfrc522.h"

typedef struct PunctureInfo
{
	unsigned char model[16];
	
	//----------------------//
	unsigned char contryID;
	unsigned char manufactureTime[6];//����ʱ�䣬�����գ�ʱ����
	unsigned char lastServiceTime[9];//����ʱ��,��Ϊ��λ
	
	
	//----------------------//
	unsigned long serviceTime;//��������ʹ��ʱ�䣬��Ϊ��λ
	unsigned long intervalTime;//���ʹ��ʱ�䣬��Ϊ��λ
	unsigned char firstUseTime[8];//�״ο���ʱ�䣬������ʱ����
	
	//----------------------//
	unsigned char verify[16];
	
}PUNCTURE_INFO, *PUNCTURE_INFO_P;

//sbit testing_bit = P0^3;
const unsigned char code gAuthentKeyA[2][6]={{0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
	
//const unsigned char code gTestWriteData[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x88};
	
unsigned char gI2CBuffer[16]={0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72,0x72};


const PUNCTURE_INFO code myPunctureInfo={
	{"DM18G"},//�ͺ� 
	86,//���Һ���
	{17,3,1,15,44, 0},//����ʱ��,������,ʱ����
	{18,3,1,15,44, 0},//����ʱ��,������,ʱ����
	240,//����֮������ʹ�õ�ʱ��, 4Сʱ
	30,//ʹ�ù���,����γ���ʱ��
	{0,0,0,0,0,0,0,0},//�״ο���ʱ��
	{"0123456789abcde"}//У����
};

void main(void)
{
	unsigned char i,j;

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

		} else {
		continue ;
		}
	
	if( pcd_auth_state(PICC_AUTHENT1A, 1, gAuthentKeyA[0], gI2CBuffer)== MI_OK){
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "AuthenOK\n");
			
		
		if(pcd_write(1, myPunctureInfo.verify) == MI_OK){
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "wrOK\n");
				if(pcd_read(1,gI2CBuffer) == MI_OK){
					
					while(gTXReady[NR_UART0]==0){;};
					UART_send_array(NR_UART0, gI2CBuffer, 16);
					while(gTXReady[NR_UART0]==0){;};
						
					break;
				} else {
					while(gTXReady[NR_UART0]==0){;};
					UART_send_str(NR_UART0, "rdErr\n");
					break;
				}
					
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "wrErr,end\n");
			break;
		}
	} else {
		continue ;
	}

		
		for(i=0; i<250; i++){
			for(j=0; j<250; j++){
				_nop_();_nop_();_nop_();
			}
		}
	      
	}                                   // End of while(1)
}                                      

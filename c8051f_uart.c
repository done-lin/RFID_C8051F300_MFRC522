#include <c8051f300.h>
#include "c8051f_cfg.h"
#include "c8051f_uart.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

#define PGF_MODEL_SCAN 10
#define PGF_MODEL_18G 80
#define PGF_MODEL_20G 81
#define PGF_MODEL_22G 82
				


unsigned char gUARTTxBuffer[UART_QTY][UART_BUFFERSIZE];
unsigned char gUARTRxBuffer[UART_QTY][14];
unsigned char gUARTBufferSize[UART_QTY] = 0;
unsigned char gUARTInputFirst[UART_QTY] = 0;
unsigned char gUARTOutputFirst[UART_QTY] = 0;
unsigned char gTXReady[UART_QTY] =1;
unsigned char gUartRecStatusFlag = 0x00;
char data gByte[UART_QTY];//the SBUF byte, it tells what byte you are handling
unsigned char gCardSn[5]={0x00,0x00,0x00,0x00};
//unsigned char xorForSendData5A;
const PUNCTURE_INFO code myPunctureInfo={
	0x55,//checksum_2，1
	0x56,//86,manufactureSN，3
	1,//country, China is one
	PGF_MODEL_18G,//model, 80 is 18g
	86+1+80,//checksum_3,86+1+80=0xa7
	30,//0x1e, intervalTime;//12,in minutes
	0x00,//reserved_1;//13
	240,//0xf0,serviceTime;//14,in minutes
	946648800,//58B95E53, lastServiceTime;//18
	0x00,//reserved_2;//19
	0xaa//manufactureChecksum;//20
};

//#define Lin_Debug
void UART_send_str(unsigned char UartNr, unsigned char *ucString)
{
#ifdef Lin_Debug
	unsigned char strCnt = 0;
	while(*(ucString+strCnt) != 0){
	gUARTTxBuffer[UartNr][strCnt] = *(ucString+strCnt);
	strCnt++;
	}


	gUARTBufferSize[UartNr] = strCnt;
	gUARTInputFirst[UartNr] = strCnt;
	gUARTOutputFirst[UartNr] = 0;
	gByte[UartNr] = 0;
	gTXReady[UartNr] = 0;
	RS485TxEnIO = 1;
	TI0 = 1; //start to send uart string
#else
	;
#endif
}


void UART_send_array(unsigned char UartNr, unsigned char *ucData, unsigned char numQty)
{
	unsigned char strCnt;
	for(strCnt = 0; strCnt < numQty; strCnt++){
	gUARTTxBuffer[UartNr][strCnt] = *(ucData+strCnt);
	}

	gUARTBufferSize[UartNr] = numQty;
	gUARTInputFirst[UartNr] = numQty;
	gUARTOutputFirst[UartNr] = 0;
	gByte[UartNr]=0;
	gTXReady[UartNr] = 0;
	RS485TxEnIO = 1;
	TI0 = 1; //start to send uart string	
}


//-----------------------------------------------------------------------------
// UART0_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1.
//-----------------------------------------------------------------------------
void UART0_init(void)
{
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                       //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI0 and TI0 bits
   if (SYSCLK/BAUDRATE/2/256 < 1) {
      TH1 = -(SYSCLK/BAUDRATE/2);
      CKCON |= 0x10;                   // T1M = 1; SCA1:0 = xx
   } else if (SYSCLK/BAUDRATE/2/256 < 4) {
      TH1 = -(SYSCLK/BAUDRATE/2/4);
      CKCON |=  0x01;                  // T1M = 0; SCA1:0 = 01
      CKCON &= ~0x12;
   } else if (SYSCLK/BAUDRATE/2/256 < 12) {
      TH1 = -(SYSCLK/BAUDRATE/2/12);
      CKCON &= ~0x13;                  // T1M = 0; SCA1:0 = 00
   } else {
      TH1 = -(SYSCLK/BAUDRATE/2/48);
      CKCON |=  0x02;                  // T1M = 0; SCA1:0 = 10
      CKCON &= ~0x11;
   }

   TL1 = 0xff;                         // set Timer1 to overflow immediately
   TMOD |= 0x20;                       // TMOD: timer 1 in 8-bit autoreload
   TMOD &= ~0xD0;                      // mode
   TR1 = 1;                            // START Timer1
   gTXReady[NR_UART0] = 1;                       // Flag showing that UART can transmit
   IP |= 0x10;                         // Make UART high priority
   ES0 = 1;                            // Enable UART0 interrupts
}



//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// UART0_Interrupt
//-----------------------------------------------------------------------------
//
// This routine is invoked whenever a character is entered or displayed on the
// Hyperterminal.
//
//-----------------------------------------------------------------------------
void UART0_Interrupt(void) interrupt 4
{

    if (RI0 == 1){
			RI0 = 0; 
			gByte[NR_UART0] = SBUF0;
        if( gUARTBufferSize[NR_UART0] == 0) {        // If new word is entered
            gUARTInputFirst[NR_UART0] = 0;
						switch(SBUF0)
						{
						case 0xA5://这里经过对比,数据顺序和大小没问题
							gUARTTxBuffer[NR_UART0][0] = 0x5a;
							gUARTTxBuffer[NR_UART0][1] = gCardSn[0];//card id, last byte. low
							gUARTTxBuffer[NR_UART0][2] = gCardSn[1];
							gUARTTxBuffer[NR_UART0][3] = gCardSn[2];
							gUARTTxBuffer[NR_UART0][4] = gCardSn[3];//card id, High
							gUARTTxBuffer[NR_UART0][5] = 0;//checksum1; card id
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
							gUARTTxBuffer[NR_UART0][19] = myPunctureInfo.reserved_2;//0X00
							gUARTTxBuffer[NR_UART0][20] = myPunctureInfo.manufactureChecksum;//0XAA
							
							gUARTTxBuffer[NR_UART0][21] = gUARTTxBuffer[NR_UART0][0]^gUARTTxBuffer[NR_UART0][1]^gUARTTxBuffer[NR_UART0][2]^gUARTTxBuffer[NR_UART0][3]^
									gUARTTxBuffer[NR_UART0][4]^gUARTTxBuffer[NR_UART0][5]^gUARTTxBuffer[NR_UART0][6]^gUARTTxBuffer[NR_UART0][7]^gUARTTxBuffer[NR_UART0][8]^
									gUARTTxBuffer[NR_UART0][9]^gUARTTxBuffer[NR_UART0][10]^gUARTTxBuffer[NR_UART0][11]^gUARTTxBuffer[NR_UART0][12]^gUARTTxBuffer[NR_UART0][13]^
									gUARTTxBuffer[NR_UART0][14]^gUARTTxBuffer[NR_UART0][15]^gUARTTxBuffer[NR_UART0][16]^gUARTTxBuffer[NR_UART0][17]^gUARTTxBuffer[NR_UART0][18]^
									gUARTTxBuffer[NR_UART0][19]^gUARTTxBuffer[NR_UART0][20];
							
							gUARTBufferSize[NR_UART0] = 22;
							gUARTOutputFirst[NR_UART0] = 0;
							gUARTInputFirst[NR_UART0] = 0;
							gTXReady[NR_UART0] = 0;
							RS485TxEnIO = 1;
							TI0 = 1;
							
							break;
						
						case 0xa6:
								gUartRecStatusFlag=0xa6;
								break;
						
						case 0xC3:
								gUartRecStatusFlag=0xc3;
								gUARTInputFirst[NR_UART0]++;
								gUARTBufferSize[NR_UART0]++;
							break;
						case 0xD4://write data in rfid
								gUartRecStatusFlag=0xd4;
								gUARTInputFirst[NR_UART0]++;
								gUARTBufferSize[NR_UART0]++;
							break;
						case 0x20:
								gUartRecStatusFlag=0x20;
								gUARTInputFirst[NR_UART0]++;
								gUARTBufferSize[NR_UART0]++;
							break;
						default:
							gUARTInputFirst[NR_UART0]=0;
							gUARTBufferSize[NR_UART0]=0;
							break;
        }
        
        } 
				else //--------------------------------------------//
					{
          gUARTRxBuffer[NR_UART0][gUARTInputFirst[NR_UART0]] = gByte[NR_UART0];     // Store in array
					gUARTInputFirst[NR_UART0]++;
          gUARTBufferSize[NR_UART0]++;                       // Update array's size
					if(gUartRecStatusFlag == 0x20){
						if(gUARTInputFirst[NR_UART0]==2 && gByte[NR_UART0] == 0x00){
							LEDControlIO=1;
							gUartRecStatusFlag=0x00;
						}else if(gUARTInputFirst[NR_UART0]==2 && gByte[NR_UART0] == 0x10){
							LEDControlIO=0;
							gUartRecStatusFlag=0x00;
						}
						gUARTInputFirst[NR_UART0]=0;
						gUARTBufferSize[NR_UART0]=0;
					}else if(gUartRecStatusFlag == 0xd4){
						if(gUARTInputFirst[NR_UART0]==14){
							gUARTInputFirst[NR_UART0]=0;
							gUARTBufferSize[NR_UART0]=0;
							gUartRecStatusFlag=0x00;
							}
					}else if(gUartRecStatusFlag == 0xc3){
						
						switch(gUARTInputFirst[NR_UART0])//the secode bytes after 0xc3
						{
							case 3://the first bytes after 0xc3, should be card id
								//SN, low 8
								break;
							
							case 4:
								//SN, high 8
								break;
							
							case 5:
								//country
								break;
							
							case 6:
								//model
								break;
							
							case 7://intervalTime
								
								break;
							
							case 8:
								//update RFID service time;
								break;
							//////////////////////////////////////////
							case 9://update last service,in scond, low 8;
								break;
							
							case 10://last_service_time2
								break;
							
							case 11://last_service_time3
								break;
							
							case 12://last_service_time4, hight 8
								break;
							///////////////////////////////////////////
							
							case 13://checksum
								if(gByte[NR_UART0] != (unsigned char)(gUARTRxBuffer[NR_UART0][1] + gUARTRxBuffer[NR_UART0][2] + gUARTRxBuffer[NR_UART0][3] +
									gUARTRxBuffer[NR_UART0][4] + gUARTRxBuffer[NR_UART0][5] + gUARTRxBuffer[NR_UART0][6] + gUARTRxBuffer[NR_UART0][7] +
								gUARTRxBuffer[NR_UART0][8]+gUARTRxBuffer[NR_UART0][9]+gUARTRxBuffer[NR_UART0][10]+gUARTRxBuffer[NR_UART0][11]))//0x00 is reserve_2; 0xaa is manufactureSN;
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
									return ;
								} 
								else //update RFID, UPDATE service time and last service time;
								{
									 gI2CBuffer[0] = gUARTRxBuffer[NR_UART0][1];//RESERVED
									 gI2CBuffer[1] = gUARTRxBuffer[NR_UART0][2];//SN LOW8
									 gI2CBuffer[2] = gUARTRxBuffer[NR_UART0][3];//SN HIGH8
									 gI2CBuffer[3] = gUARTRxBuffer[NR_UART0][4];//COUNTRY
									 gI2CBuffer[4] = gUARTRxBuffer[NR_UART0][5];//MODEL
									 gI2CBuffer[5] = gUARTRxBuffer[NR_UART0][6];//INTERVAL TIME
									 gI2CBuffer[6] = gUARTRxBuffer[NR_UART0][7];//update service time in c8051f ram
									
									 gI2CBuffer[7] = gUARTRxBuffer[NR_UART0][8];//update laset service time in c8051f ram;
									 gI2CBuffer[8] = gUARTRxBuffer[NR_UART0][9];//laset service time 
									 gI2CBuffer[9] = gUARTRxBuffer[NR_UART0][10];//laset service time 
									 gI2CBuffer[10] = gUARTRxBuffer[NR_UART0][11];//laset service time 
									 gUartRecStatusFlag=0x3c;
									 gUARTInputFirst[NR_UART0]=0;
							     gUARTBufferSize[NR_UART0]=0;
									
										gUARTTxBuffer[NR_UART0][0] = 0x4D;
										gUARTTxBuffer[NR_UART0][1] = gCardSn[0];
										gUARTTxBuffer[NR_UART0][2] = gCardSn[1];
										gUARTTxBuffer[NR_UART0][3] = gCardSn[2];
										gUARTTxBuffer[NR_UART0][4] = gCardSn[3];
										gUARTTxBuffer[NR_UART0][5] = gCardSn[0]^gCardSn[1]^gCardSn[2]^gCardSn[3]^0x4d;
										
										gUARTBufferSize[NR_UART0] = 6;
										gUARTOutputFirst[NR_UART0] = 0;
										gUARTInputFirst[NR_UART0] = 0;
										gTXReady[NR_UART0] = 0;
										RS485TxEnIO = 1;
										TI0 = 1;
									 return ; 
								}
								
								break;
							
							
						}
						
						if(gUARTInputFirst[NR_UART0]==13){
							gUARTInputFirst[NR_UART0]=0;
							gUARTBufferSize[NR_UART0]=0;
							gUartRecStatusFlag=0x00;
						}
					}
    }
    }
    
    if (TI0 == 1) { // Check if transmit flag is set
        TI0 = 0;                           // Clear interrupt flag
        if (gUARTBufferSize[NR_UART0] != 0) {// If buffer not empty
            // If a new word is being output
            if ( gUARTBufferSize[NR_UART0] == gUARTInputFirst[NR_UART0] ) {
                gUARTOutputFirst[NR_UART0] = 0;
            }
            // Store a character in the variable byte
            gByte[NR_UART0] = gUARTTxBuffer[NR_UART0][gUARTOutputFirst[NR_UART0]];
            SBUF0 = gByte[NR_UART0];                   // Transmit to Hyperterminal
            gUARTOutputFirst[NR_UART0]++;            // Update counter
            gUARTBufferSize[NR_UART0]--;             // Decrease array size
        } else {
				 RS485TxEnIO = 0;//RECEIVE
         gTXReady[NR_UART0] = 1;                    // Indicate transmission complete
        }
    }

}


//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
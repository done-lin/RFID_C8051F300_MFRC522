#include <c8051f300.h>
#include "c8051f_cfg.h"
#include "c8051f_uart.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------


unsigned char gUARTTxBuffer[UART_QTY][UART_BUFFERSIZE];
unsigned char gUARTRxBuffer[UART_QTY][14];
unsigned char gUARTBufferSize[UART_QTY] = 0;
unsigned char gUARTInputFirst[UART_QTY] = 0;
unsigned char gUARTOutputFirst[UART_QTY] = 0;
unsigned char gTXReady[UART_QTY] =1;
unsigned char gUartRecStatusFlag = 0x00;
char data gByte[UART_QTY];//the SBUF byte, it tells what byte you are handling
unsigned char gCardSn[5]={0x00,0x00,0x00,0x00};
unsigned char xorForSendData5A;
const PUNCTURE_INFO code myPunctureInfo={
	0x55,//checksum_2£¬1
	0x56,//86,manufactureSN£¬3
	1,//country, China is one
	80,//model, 80 is 18g
	86+1+80,//checksum_3,86+1+80=0xa7
	30,//0x1e, intervalTime;//12,in minutes
	0x00,//reserved_1;//13
	240,//0xf0,serviceTime;//14,in minutes
	1488543315,//58B95E53, lastServiceTime;//18
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
						case 0xA5:
							if(gCardSn[0]!= 0 || gCardSn[1]!= 0 || gCardSn[2]!= 0&& gI2CBuffer[11] == 0x01){// 
								gUARTRxBuffer[NR_UART0][0] = gByte[NR_UART0];
								gUARTTxBuffer[NR_UART0][0] = 0x5a;
								gUARTTxBuffer[NR_UART0][1] = gCardSn[0];
								gUARTTxBuffer[NR_UART0][2] = gCardSn[1];
								gUARTTxBuffer[NR_UART0][3] = gCardSn[2];
								gUARTTxBuffer[NR_UART0][4] = gCardSn[3];
								gUARTTxBuffer[NR_UART0][5] = (unsigned char)(gCardSn[0]+gCardSn[1]+gCardSn[2]+gCardSn[3]);
								gUARTTxBuffer[NR_UART0][6] = 0x55; //checkum_2
								gUARTTxBuffer[NR_UART0][7] = gI2CBuffer[1];//manufactureSN. LOW
								gUARTTxBuffer[NR_UART0][8] = gI2CBuffer[2];//SN high 8;
								gUARTTxBuffer[NR_UART0][9] = gI2CBuffer[3];//country
								gUARTTxBuffer[NR_UART0][10] = gI2CBuffer[4];//model
								gUARTTxBuffer[NR_UART0][11] = (unsigned char)(gI2CBuffer[1]+gI2CBuffer[2]+gI2CBuffer[3]+gI2CBuffer[4]);
								gUARTTxBuffer[NR_UART0][12] = gI2CBuffer[5];//intervalTime;
								gUARTTxBuffer[NR_UART0][13] = 0x00;//reserved_1;
								gUARTTxBuffer[NR_UART0][14] = gI2CBuffer[6];//serviceTime;
								gUARTTxBuffer[NR_UART0][15] = gI2CBuffer[7];//lastServiceTime;
								gUARTTxBuffer[NR_UART0][16] = gI2CBuffer[8];//lastServiceTime>>8;
								gUARTTxBuffer[NR_UART0][17] = gI2CBuffer[9];//lastServiceTime>>16;
								gUARTTxBuffer[NR_UART0][18] = gI2CBuffer[10];//lastServiceTime>>24;
								gUARTTxBuffer[NR_UART0][19] = 0x00;//reserved_2;
								gUARTTxBuffer[NR_UART0][20] = 0xaa;//manufactureChecksum;
								gUARTTxBuffer[NR_UART0][21] = xorForSendData5A;

								gUARTBufferSize[NR_UART0] = 22;
								gUARTOutputFirst[NR_UART0] = 0;
								gUARTInputFirst[NR_UART0] = 0;
								gTXReady[NR_UART0] = 0;
								RS485TxEnIO = 1;
								TI0 = 1;
							} else {
								gUARTTxBuffer[NR_UART0][0] = 0x5a;
								gUARTTxBuffer[NR_UART0][1] = 0;
								gUARTTxBuffer[NR_UART0][2] = 0;
								gUARTTxBuffer[NR_UART0][3] = 0;
								gUARTTxBuffer[NR_UART0][4] = 0;
								gUARTTxBuffer[NR_UART0][5] = 0;
								gUARTTxBuffer[NR_UART0][6] = 0;
								gUARTTxBuffer[NR_UART0][7] = 0;
								gUARTTxBuffer[NR_UART0][8] = 0;
								gUARTTxBuffer[NR_UART0][9] = 0;
								gUARTTxBuffer[NR_UART0][10] = 0;
								gUARTTxBuffer[NR_UART0][11] = 0;
								gUARTTxBuffer[NR_UART0][12] = 0;
								gUARTTxBuffer[NR_UART0][13] = 0;
								gUARTTxBuffer[NR_UART0][14] = 0;
								gUARTTxBuffer[NR_UART0][15] = 0;
								gUARTTxBuffer[NR_UART0][16] = 0;
								gUARTTxBuffer[NR_UART0][17] = 0;
								gUARTTxBuffer[NR_UART0][18] = 0;
								gUARTTxBuffer[NR_UART0][19] = 0;
								gUARTTxBuffer[NR_UART0][20] = 0;
								gUARTTxBuffer[NR_UART0][21] = 0x5a;

								gUARTBufferSize[NR_UART0] = 22;
								gUARTOutputFirst[NR_UART0] = 0;
								gUARTInputFirst[NR_UART0] = 0;
								gTXReady[NR_UART0] = 0;
								RS485TxEnIO = 1;
								TI0 = 1;
							}
								
							break;
						case 0xC3:
								gUartRecStatusFlag=0xc3;
								gUARTInputFirst[NR_UART0]++;
								gUARTBufferSize[NR_UART0]++;
							break;
						case 0xD4:
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
					}else if(gUartRecStatusFlag == 0xc3){
						if(gUARTInputFirst[NR_UART0]==14){
							gUARTInputFirst[NR_UART0]=0;
							gUARTBufferSize[NR_UART0]=0;
							gUartRecStatusFlag=0x00;
							}
					}else if(gUartRecStatusFlag == 0xd4){
						
						switch(gUARTInputFirst[NR_UART0])
						{
							case 1:
								if(gByte[NR_UART0] != gCardSn[0])
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								}
								break;
							
							case 2:
								if(gByte[NR_UART0] != gCardSn[1])
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								}
								break;
							
							case 3:
								if(gByte[NR_UART0] != gCardSn[2])
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								}
								break;
							
							case 4:
								if(gByte[NR_UART0] != gCardSn[3])
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								}
								break;
							
							case 5://reserved_1;
								
								if(gByte[NR_UART0] != 0x00)
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								}//// add for test
								
								break;
							
							case 6:
								//update RFID service time;

									
								break;
							//////////////////////////////////////////
							case 7://update last service,in scond;
								break;
							
							case 8:
								break;
							
							case 9:
								break;
							
							case 10:
								break;
							///////////////////////////////////////////
							case 11:
								if(gByte[NR_UART0] != 0x00)
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								}
								break;
							
							case 12:
								if(gByte[NR_UART0] != 0xaa)
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								}
								break;
							
							case 13://checksum
								if(gByte[NR_UART0] != (gCardSn[0]^gCardSn[1]^gCardSn[2]^gCardSn[3]^gUARTTxBuffer[NR_UART0][13]^//reserve_2;
									gUARTTxBuffer[NR_UART0][14]^gUARTTxBuffer[NR_UART0][15]^gUARTTxBuffer[NR_UART0][16]^gUARTTxBuffer[NR_UART0][17]^//lastServiceTime;
									gUARTTxBuffer[NR_UART0][18]^0x00^0xAA) && gI2CBuffer[11] == 0x01)//0x00 is reserve_2; 0xaa is manufactureSN;
								{
									gUARTInputFirst[NR_UART0]=0;
									gUARTBufferSize[NR_UART0]=0;
									gUartRecStatusFlag = 0x00;
								} 
								else //update RFID, UPDATE service time and last service time;
								{
									 gI2CBuffer[6] = gUARTRxBuffer[NR_UART0][6];
									 gI2CBuffer[7] = gUARTRxBuffer[NR_UART0][7];
									 gI2CBuffer[8] = gUARTRxBuffer[NR_UART0][8];
									 gI2CBuffer[9] = gUARTRxBuffer[NR_UART0][9];
									 gI2CBuffer[10] = gUARTRxBuffer[NR_UART0][10];
								}
								
								break;
							
							
						}
						
						if(gUARTInputFirst[NR_UART0]==14){
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
				 RS485TxEnIO = 0;
         gTXReady[NR_UART0] = 1;                    // Indicate transmission complete
        }
    }

}


//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
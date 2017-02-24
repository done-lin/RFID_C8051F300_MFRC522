#include <c8051f300.h>
#include "c8051f_cfg.h"
#include "c8051f_uart.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

unsigned char gUARTTxBuffer[UART_QTY][UART_BUFFERSIZE];
unsigned char gUARTRxBuffer[UART_QTY][UART_BUFFERSIZE];
unsigned char gUARTBufferSize[UART_QTY] = 0;
unsigned char gUARTInputFirst[UART_QTY] = 0;
unsigned char gUARTOutputFirst[UART_QTY] = 0;
unsigned char gTXReady[UART_QTY] =1;
char data gByte[UART_QTY];//the SBUF byte, it tells what byte you are handling

const unsigned char code gTestingArray[5]={10,11,12,13,14};

////#define Lin_Debug
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
	TI0 = 1; //start to send uart string
#elseif
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
        if( gUARTBufferSize[NR_UART0] == 0) {        // If new word is entered
            gUARTInputFirst[NR_UART0] = 0;
        }
        
        RI0 = 0;                            // Clear interrupt flag
        gByte[NR_UART0] = SBUF0;                       // Read a character from UART

      if (gUARTBufferSize[NR_UART0] < UART_BUFFERSIZE) {
          gUARTRxBuffer[NR_UART0][gUARTInputFirst[NR_UART0]] = gByte[NR_UART0];     // Store in array
          gUARTBufferSize[NR_UART0]++;                       // Update array's size
          gUARTInputFirst[NR_UART0]++;             // Update counter
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
         gTXReady[NR_UART0] = 1;                    // Indicate transmission complete
        }
    }
}


//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
#include <c8051f300.h>                 // SFR declarations

/**
*	OSCILLATOR_Init - 初始化c8051f的系统时钟振荡器
*	@void: none
*/
void c8051f_oscillator_init(void)
{
   OSCICN |= 0x07;                     // Configure internal oscillator for
                                       // its maximum frequency
   RSTSRC  = 0x04;                     // Enable missing clock detector                                    // its maximum frequency (24.5 Mhz)
}

void c8051f_port_init(void)
{
	P0MDIN |= 0x0f;                     // P0.2 and P03. are digital

	P0MDOUT = 0x03;                     // P0.3 and P0.2 all is push-pull
	P0MDOUT |= 0x10;                    // set UART TX to push-pull output
	
	XBR1    = 0x03;                     // Enable UTX, URX as push-pull output
	XBR2    = 0x40;                     // Enable crossbar, weak pull-ups
	                                   // disabled
	P0     |= 0x08;                     // Set P0.3 latch to '1'
	
}

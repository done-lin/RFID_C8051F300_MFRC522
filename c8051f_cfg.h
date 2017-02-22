#ifndef __C8051F_CFG_H__
#define __C8051F_CFG_H__

#define SYSCLK      24500000           // SYSCLK frequency in Hz

extern void c8051f_oscillator_init(void);
extern void c8051f_port_init(void);

#endif

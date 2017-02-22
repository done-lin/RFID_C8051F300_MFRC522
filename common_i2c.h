#ifndef __COMMON_I2C_H__
#define __COMMON_I2C_H__

extern void I2C_nop_delay_us(unsigned char us);
extern void I2C_start(void);
extern void I2C_stop(void);
extern void I2C_send_ack(bit ackBit);
extern bit I2C_write_one_byte(unsigned char ucData);
extern unsigned char I2C_read_one_byte(void);
extern unsigned char I2C_read_str(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucBuf,unsigned char ucCount);
extern unsigned char I2C_write_str(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucData,unsigned char ucNo);

#endif

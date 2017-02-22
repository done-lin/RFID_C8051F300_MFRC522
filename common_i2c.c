#include <intrins.h>
#include "common_i2c.h"
#include "c8051f300.h"
/**
*	I2C_nop_delay - 进行I2C最基本的延迟
*	@void: none
*/
sbit SDA=P0^3;
sbit SCL=P0^2;

void I2C_nop_delay_us(unsigned char us)//大约延迟4.7us，根据实际系统时钟微调
{
	while(us--){
	_nop_();_nop_();
	_nop_();_nop_();
	_nop_();_nop_();
	_nop_();_nop_();
	//_nop_();_nop_();
	//_nop_();_nop_();
	//_nop_();

	}
}


/**
*	I2C_start - 启动I2C传输
*	@void: none
*/
void I2C_start(void)
{
	SDA=1; //发送起始条件的数据信号
	I2C_nop_delay_us(1);
	SCL=1; //发送起始条件的时钟信号
	I2C_nop_delay_us(5);//起始条件建立时间大于4.7us，不能少于
	SDA=0; //发送起始信号，起始条件锁定时间大于4us，这里也必须大于4us
    I2C_nop_delay_us(5);
	SCL=0; //钳住I2C总线,准备发送或接收
    I2C_nop_delay_us(2);
}


/**
*	I2C_stop - 停止I2C传输
*	@void: none
*/
void I2C_stop(void) //释放I2C总线
{
	SDA=0;//发送停止条件的数据信号
	I2C_nop_delay_us(2);
	SCL=1; 
	I2C_nop_delay_us(5);//起始条件建立时间大于4us
	SDA=1;//发送I2C总线停止信号
	I2C_nop_delay_us(5);//停止条件锁定时间大于4us
}


/**
*	I2C_stop - 停止I2C传输
*	@ackBit: 主机对从机发送的应答信号，在数据显示，1 bit，应答为0，非应答为1
*/
void I2C_send_ack(bit ackBit)
{
	SDA=ackBit;//发送的应答或非应答信号
	I2C_nop_delay_us(2);
	SCL=1;//置时钟线为高使应答位有效
	I2C_nop_delay_us(5);//时钟高周期大于4us，不同于器件发送到主机的应答信号
	SCL=0;
	I2C_nop_delay_us(2);
}


/**
*	I2C_write_one_byte - 向I2C发送一个字节的数据
*	@ucData: 无符号8位，要发送的数据
*
*	return: bAck,成功写入返回1，否则，失败返回0
*/
bit I2C_write_one_byte(unsigned char ucData)
{
	bit bACK;
	unsigned char data i;

	i=8;
	while(i--){//8 位没发送完继续发送
	if((ucData & 0x80)==0x80)
		SDA=1;//I2C MSB高位先发
	else 
		SDA=0;
	I2C_nop_delay_us(2); 
	SCL=1;//置时钟线为高通知被控器开始接收数据位
	I2C_nop_delay_us(5);
	SCL=0;

	ucData=ucData<<1;
	}
	I2C_nop_delay_us(1);
	SDA=1; //8位数据发送完,释放I2C总线,准备接收应答位
	I2C_nop_delay_us(2);
	SCL=1;//开始接收应答信号
	I2C_nop_delay_us(5);
	if(SDA){//应答只需普通最小数据锁存的延时时间
		bACK=0;
	}else{
		bACK=1;
		}
	SCL=0;//发送结束钳住总线准备下一步发送或接收数据
	I2C_nop_delay_us(2);
	return(bACK);//正确应答返回0
}


/**
*	I2C_read_one_byte - 向I2C读取一个字节的数据
*	@void: none
*	
*	return: 返回一个读到的数据，1个字节
*/
unsigned char I2C_read_one_byte(void)
{
	unsigned char data i=0,byteData=0;
	SDA=1;//置数据线为输入方式
	
	i=8;
	while(i--){
		I2C_nop_delay_us(1);
		SCL=0; //置钟线为零准备接收数据
		I2C_nop_delay_us(5);//时钟低周期大于4.7us
		
		SCL=1;//置时钟线为高使数据线上数据有效
		I2C_nop_delay_us(1);
		byteData=byteData<<1;
		if(SDA) byteData++;
		I2C_nop_delay_us(1);
		}

	SCL=0;//8 位接收完置时钟线和数据线为低准备发送应答或非应答信号
	I2C_nop_delay_us(1);
	return(byteData);
}


/**
*	I2C_read_str - 从I2C设备读入一串数据
*	@ucSla: slave，从器件地址
*	@ucAddress: 器件里面，要读的寄存器地址
*	@ucBuf: 要读入的buf，读到数据存在这里
*	@ucCount: 计划读入的字节数
*
*	return: 读入数成功返回1，否则返回0
*/
unsigned char I2C_read_str(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucBuf,unsigned char ucCount)
{
	unsigned char idata i=0;

	I2C_start();
	 
	if(!I2C_write_one_byte(ucSla)){//write one byte里包含应答
		I2C_stop();
		return 0;//选从器件的地址
		}

	if(!I2C_write_one_byte(ucAddress)){//选第一个寄存器地址
		I2C_stop();
		return 0;
		}

	I2C_start();

	if(!I2C_write_one_byte(ucSla+1)){//发送读器件命令，ucSla+1表示要读的器件是ucSla
		I2C_stop();
		return 0;
		}

	i=ucCount;
	while(i--){
		*ucBuf=I2C_read_one_byte();//读从器件寄存器
		if(i)
			I2C_send_ack(0);//未接收完所有字节,发送应答信号
		ucBuf++;
		}
	I2C_send_ack(1);//接收完所有字节,发送非应答信号
	I2C_stop();
	return 1;
}


/**
*	I2C_write_str - 向I2C设备写入一串数据
*	@ucSla: slave，从器件地址
*	@ucAddress: 器件里面，要读的寄存器地址
*	@ucData: 要写入的数据数组
*	@ucNo: 期望写入的个数
*
*	return: 正确返回0，否则返回1
*/
unsigned char I2C_write_str(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucData,unsigned char ucNo)
{
	unsigned char data i;
	I2C_start();

	if(!I2C_write_one_byte(ucSla)){
		I2C_stop();//写入失败，直接发停止命令
		return 0;//往I2C写命令
		}

	if(!I2C_write_one_byte(ucAddress)){
		I2C_stop();
		return 0;//写寄存器地址
		}


	i=ucNo;
	while(i--){
		if(!I2C_write_one_byte(*ucData)){
			I2C_stop();
			return 0;//写数据
			}
		ucData++;
		}
	
	I2C_stop();//最后停止，返回1表示成功写入数组
	return 1;
}


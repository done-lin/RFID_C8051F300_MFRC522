#include <intrins.h>
#include "common_i2c.h"
#include "c8051f300.h"
/**
*	I2C_nop_delay - ����I2C��������ӳ�
*	@void: none
*/
sbit SDA=P0^3;
sbit SCL=P0^2;

void I2C_nop_delay_us(unsigned char us)//��Լ�ӳ�4.7us������ʵ��ϵͳʱ��΢��
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
*	I2C_start - ����I2C����
*	@void: none
*/
void I2C_start(void)
{
	SDA=1; //������ʼ�����������ź�
	I2C_nop_delay_us(1);
	SCL=1; //������ʼ������ʱ���ź�
	I2C_nop_delay_us(5);//��ʼ��������ʱ�����4.7us����������
	SDA=0; //������ʼ�źţ���ʼ��������ʱ�����4us������Ҳ�������4us
    I2C_nop_delay_us(5);
	SCL=0; //ǯסI2C����,׼�����ͻ����
    I2C_nop_delay_us(2);
}


/**
*	I2C_stop - ֹͣI2C����
*	@void: none
*/
void I2C_stop(void) //�ͷ�I2C����
{
	SDA=0;//����ֹͣ�����������ź�
	I2C_nop_delay_us(2);
	SCL=1; 
	I2C_nop_delay_us(5);//��ʼ��������ʱ�����4us
	SDA=1;//����I2C����ֹͣ�ź�
	I2C_nop_delay_us(5);//ֹͣ��������ʱ�����4us
}


/**
*	I2C_stop - ֹͣI2C����
*	@ackBit: �����Դӻ����͵�Ӧ���źţ���������ʾ��1 bit��Ӧ��Ϊ0����Ӧ��Ϊ1
*/
void I2C_send_ack(bit ackBit)
{
	SDA=ackBit;//���͵�Ӧ����Ӧ���ź�
	I2C_nop_delay_us(2);
	SCL=1;//��ʱ����Ϊ��ʹӦ��λ��Ч
	I2C_nop_delay_us(5);//ʱ�Ӹ����ڴ���4us����ͬ���������͵�������Ӧ���ź�
	SCL=0;
	I2C_nop_delay_us(2);
}


/**
*	I2C_write_one_byte - ��I2C����һ���ֽڵ�����
*	@ucData: �޷���8λ��Ҫ���͵�����
*
*	return: bAck,�ɹ�д�뷵��1������ʧ�ܷ���0
*/
bit I2C_write_one_byte(unsigned char ucData)
{
	bit bACK;
	unsigned char data i;

	i=8;
	while(i--){//8 λû�������������
	if((ucData & 0x80)==0x80)
		SDA=1;//I2C MSB��λ�ȷ�
	else 
		SDA=0;
	I2C_nop_delay_us(2); 
	SCL=1;//��ʱ����Ϊ��֪ͨ��������ʼ��������λ
	I2C_nop_delay_us(5);
	SCL=0;

	ucData=ucData<<1;
	}
	I2C_nop_delay_us(1);
	SDA=1; //8λ���ݷ�����,�ͷ�I2C����,׼������Ӧ��λ
	I2C_nop_delay_us(2);
	SCL=1;//��ʼ����Ӧ���ź�
	I2C_nop_delay_us(5);
	if(SDA){//Ӧ��ֻ����ͨ��С�����������ʱʱ��
		bACK=0;
	}else{
		bACK=1;
		}
	SCL=0;//���ͽ���ǯס����׼����һ�����ͻ��������
	I2C_nop_delay_us(2);
	return(bACK);//��ȷӦ�𷵻�0
}


/**
*	I2C_read_one_byte - ��I2C��ȡһ���ֽڵ�����
*	@void: none
*	
*	return: ����һ�����������ݣ�1���ֽ�
*/
unsigned char I2C_read_one_byte(void)
{
	unsigned char data i=0,byteData=0;
	SDA=1;//��������Ϊ���뷽ʽ
	
	i=8;
	while(i--){
		I2C_nop_delay_us(1);
		SCL=0; //������Ϊ��׼����������
		I2C_nop_delay_us(5);//ʱ�ӵ����ڴ���4.7us
		
		SCL=1;//��ʱ����Ϊ��ʹ��������������Ч
		I2C_nop_delay_us(1);
		byteData=byteData<<1;
		if(SDA) byteData++;
		I2C_nop_delay_us(1);
		}

	SCL=0;//8 λ��������ʱ���ߺ�������Ϊ��׼������Ӧ����Ӧ���ź�
	I2C_nop_delay_us(1);
	return(byteData);
}


/**
*	I2C_read_str - ��I2C�豸����һ������
*	@ucSla: slave����������ַ
*	@ucAddress: �������棬Ҫ���ļĴ�����ַ
*	@ucBuf: Ҫ�����buf���������ݴ�������
*	@ucCount: �ƻ�������ֽ���
*
*	return: �������ɹ�����1�����򷵻�0
*/
unsigned char I2C_read_str(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucBuf,unsigned char ucCount)
{
	unsigned char idata i=0;

	I2C_start();
	 
	if(!I2C_write_one_byte(ucSla)){//write one byte�����Ӧ��
		I2C_stop();
		return 0;//ѡ�������ĵ�ַ
		}

	if(!I2C_write_one_byte(ucAddress)){//ѡ��һ���Ĵ�����ַ
		I2C_stop();
		return 0;
		}

	I2C_start();

	if(!I2C_write_one_byte(ucSla+1)){//���Ͷ��������ucSla+1��ʾҪ����������ucSla
		I2C_stop();
		return 0;
		}

	i=ucCount;
	while(i--){
		*ucBuf=I2C_read_one_byte();//���������Ĵ���
		if(i)
			I2C_send_ack(0);//δ�����������ֽ�,����Ӧ���ź�
		ucBuf++;
		}
	I2C_send_ack(1);//�����������ֽ�,���ͷ�Ӧ���ź�
	I2C_stop();
	return 1;
}


/**
*	I2C_write_str - ��I2C�豸д��һ������
*	@ucSla: slave����������ַ
*	@ucAddress: �������棬Ҫ���ļĴ�����ַ
*	@ucData: Ҫд�����������
*	@ucNo: ����д��ĸ���
*
*	return: ��ȷ����0�����򷵻�1
*/
unsigned char I2C_write_str(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucData,unsigned char ucNo)
{
	unsigned char data i;
	I2C_start();

	if(!I2C_write_one_byte(ucSla)){
		I2C_stop();//д��ʧ�ܣ�ֱ�ӷ�ֹͣ����
		return 0;//��I2Cд����
		}

	if(!I2C_write_one_byte(ucAddress)){
		I2C_stop();
		return 0;//д�Ĵ�����ַ
		}


	i=ucNo;
	while(i--){
		if(!I2C_write_one_byte(*ucData)){
			I2C_stop();
			return 0;//д����
			}
		ucData++;
		}
	
	I2C_stop();//���ֹͣ������1��ʾ�ɹ�д������
	return 1;
}


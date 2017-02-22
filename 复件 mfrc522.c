#include "c8051f300.h"
#include "mfrc522.h"
#include <intrins.h>
#include "common_i2c.h" 

//MFRC522
sbit     MF522_RST  =    P0^6;                   //RC500片选
                       
/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          	  pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char pcdRequest(unsigned char req_code,unsigned char *pTagType)
{
	char status;  
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MAXRLEN]; 
	//  unsigned char xTest ;
	clearBitMask(Status2Reg,0x08);
	writeRawRc(BitFramingReg,0x07);

	//  xTest = readRawRc(BitFramingReg);
	//  if(xTest == 0x07 )
	//   { LED_GREEN  =0 ;}
	// else {LED_GREEN =1 ;while(1){}}
	setBitMask(TxControlReg,0x03);

	ucComMF522Buf[0] = req_code;

	status = pcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
	//     if(status  == MI_OK )
	//   { LED_GREEN  =0 ;}
	//   else {LED_GREEN =1 ;}
	if ((status == MI_OK) && (unLen == 0x10)) {
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}else {
		status = MI_ERR; 
	}
	
	return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
char pcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    clearBitMask(Status2Reg,0x08);
    writeRawRc(BitFramingReg,0x00);
    clearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = pcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = MI_ERR;    }
    }
    
    setBitMask(CollReg,0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char pcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    calulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    clearBitMask(Status2Reg,0x08);

    status = pcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////               
char pcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
 //   memcpy(&ucComMF522Buf[2], pKey, 6); 
 //   memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = pcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(readRawRc(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
///////////////////////////////////////////////////////////////////// 
char pcdRead(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    calulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = pcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
 //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////                  
char pcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    calulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = pcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    ucComMF522Buf[i] = *(pData+i);   }
        calulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = pcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}



/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
/*
char pcdHalt(void)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    calulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = pcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}
*/

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void calulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{/*
    unsigned char i,n;
    clearBitMask(DivIrqReg,0x04);
    writeRawRc(CommandReg,PCD_IDLE);
    setBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   writeRawRc(FIFODataReg, *(pIndata+i));   }
    writeRawRc(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = readRawRc(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = readRawRc(CRCResultRegL);
    pOutData[1] = readRawRc(CRCResultRegM);
	*/
}

/////////////////////////////////////////////////////////////////////
//功    能：复位RC522
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char pcdReset(void)
{
    MF522_RST=1;
    _nop_();
    MF522_RST=0;
    _nop_();
    MF522_RST=1;
     _nop_();

    writeRawRc(CommandReg,PCD_RESETPHASE);
    _nop_();
    
    writeRawRc(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
    writeRawRc(TReloadRegL,30);           
    writeRawRc(TReloadRegH,0);
    writeRawRc(TModeReg,0x8D);
    writeRawRc(TPrescalerReg,0x3E);
    writeRawRc(TxAutoReg,0x40);     
    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//设置RC632的工作方式 
//////////////////////////////////////////////////////////////////////
char m500PcdConfigISOType(unsigned char type)
{
   if (type == 'A')                     //ISO14443_A
   { 
       clearBitMask(Status2Reg,0x08);

 /*     writeRawRc(CommandReg,0x20);    //as default   
       writeRawRc(ComIEnReg,0x80);     //as default
       writeRawRc(DivlEnReg,0x0);      //as default
	   writeRawRc(ComIrqReg,0x04);     //as default
	   writeRawRc(DivIrqReg,0x0);      //as default
	   writeRawRc(Status2Reg,0x0);//80    //trun off temperature sensor
	   writeRawRc(WaterLevelReg,0x08); //as default
       writeRawRc(ControlReg,0x20);    //as default
	   writeRawRc(CollReg,0x80);    //as default
*/
       writeRawRc(ModeReg,0x3D);//3F
/*	   writeRawRc(TxModeReg,0x0);      //as default???
	   writeRawRc(RxModeReg,0x0);      //as default???
	   writeRawRc(TxControlReg,0x80);  //as default???

	   writeRawRc(TxSelReg,0x10);      //as default???
   */
       writeRawRc(RxSelReg,0x86);//84
 //      writeRawRc(RxThresholdReg,0x84);//as default
 //      writeRawRc(DemodReg,0x4D);      //as default

 //      writeRawRc(ModWidthReg,0x13);//26
       writeRawRc(RFCfgReg,0x7F);   //4F
	/*   writeRawRc(GsNReg,0x88);        //as default???
	   writeRawRc(CWGsCfgReg,0x20);    //as default???
       writeRawRc(ModGsCfgReg,0x20);   //as default???
*/
   	   writeRawRc(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
	   writeRawRc(TReloadRegH,0);
       writeRawRc(TModeReg,0x8D);
	   writeRawRc(TPrescalerReg,0x3E);
	   

  //     PcdSetTmo(106);
	   delay10Ms(1);
       pcdAntennaOn();
   }
   else{ return -1; }
   
   return MI_OK;
}



/////////////////////////////////////////////////////////////////////
//功    能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
unsigned char readRawRC(unsigned char Address)
{
	unsigned char data tmp;
//I2CRdStr(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucBuf,unsigned char ucCount)
    I2C_read_str(SLA_ADDR,Address,&tmp,0x01);
	return tmp;
}
/////////////////////////////////////////////////////////////////////
//功    能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void writeRawRc(unsigned char Address, unsigned char value)
{
    I2C_write_str(SLA_ADDR,Address,&value,0x01);
}


/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void setBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = readRawRc(reg);
    writeRawRc(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void clearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = readRawRc(reg);
    writeRawRc(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char pcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    writeRawRc(ComIEnReg,irqEn|0x80);
    clearBitMask(ComIrqReg,0x80);
    writeRawRc(CommandReg,PCD_IDLE);
    setBitMask(FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   writeRawRc(FIFODataReg, pInData[i]);    }
    writeRawRc(CommandReg, Command);
   
    
    if (Command == PCD_TRANSCEIVE)
    {    setBitMask(BitFramingReg,0x80);  }
    
//    i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
 i = 2000;
    do 
    {
         n = readRawRc(ComIrqReg);
         i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    clearBitMask(BitFramingReg,0x80);
	      
    if (i!=0)
    {    
         if(!(readRawRc(ErrorReg)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   status = MI_NOTAGERR;   }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = readRawRc(FIFOLevelReg);
              	lastBits = readRawRc(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = readRawRc(FIFODataReg);    }
            }
         }
         else
         {   status = MI_ERR;   }
        
   }
   

   setBitMask(ControlReg,0x80);           // stop timer now
   writeRawRc(CommandReg,PCD_IDLE); 
   return status;
}


/////////////////////////////////////////////////////////////////////
//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void pcdAntennaOn()
{
    unsigned char i;
    i = readRawRc(TxControlReg);
    if (!(i & 0x03))
    {
        setBitMask(TxControlReg, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//关闭天线
/////////////////////////////////////////////////////////////////////
void pcdAntennaOff()
{
    clearBitMask(TxControlReg, 0x03);
}

//等待卡离开
void waitCardOff(void)
{
	char status, TagType[2];

	while(1)
	{
		status = pcdRequest(REQ_ALL, TagType);
		if(status)
		{
			status = pcdRequest(REQ_ALL, TagType);
			if(status)
			{
				status = pcdRequest(REQ_ALL, TagType);
				if(status)
				{
					return;
				}
			}
		}
		delay10Ms(1);
	}
}


///////////////////////////////////////////////////////////////////////
// Delay 10ms
///////////////////////////////////////////////////////////////////////
void delay10Ms(unsigned char _10ms)
{
	unsigned int i, j;

	for(i=0; i<_10ms; i++)
	{
		for(j=0; j<10000; j++);
	}
}


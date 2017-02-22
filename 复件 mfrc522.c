#include "c8051f300.h"
#include "mfrc522.h"
#include <intrins.h>
#include "common_i2c.h" 

//MFRC522
sbit     MF522_RST  =    P0^6;                   //RC500Ƭѡ
                       
/////////////////////////////////////////////////////////////////////
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          	  pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ 
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          pData[OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          pData[IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
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
//��MF522����CRC16����
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
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
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
    
    writeRawRc(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    writeRawRc(TReloadRegL,30);           
    writeRawRc(TReloadRegH,0);
    writeRawRc(TModeReg,0x8D);
    writeRawRc(TPrescalerReg,0x3E);
    writeRawRc(TxAutoReg,0x40);     
    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//����RC632�Ĺ�����ʽ 
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
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
/////////////////////////////////////////////////////////////////////
unsigned char readRawRC(unsigned char Address)
{
	unsigned char data tmp;
//I2CRdStr(unsigned char ucSla,unsigned char ucAddress,unsigned char *ucBuf,unsigned char ucCount)
    I2C_read_str(SLA_ADDR,Address,&tmp,0x01);
	return tmp;
}
/////////////////////////////////////////////////////////////////////
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
/////////////////////////////////////////////////////////////////////
void writeRawRc(unsigned char Address, unsigned char value)
{
    I2C_write_str(SLA_ADDR,Address,&value,0x01);
}


/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void setBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = readRawRc(reg);
    writeRawRc(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void clearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = readRawRc(reg);
    writeRawRc(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pInData[IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOutData[OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
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
    
//    i = 600;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
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
//��������  
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
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
//�ر�����
/////////////////////////////////////////////////////////////////////
void pcdAntennaOff()
{
    clearBitMask(TxControlReg, 0x03);
}

//�ȴ����뿪
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


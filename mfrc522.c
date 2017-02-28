#include "c8051f300.h"
#include "mfrc522.h"
#include <intrins.h>
#include "common_i2c.h"
#include "c8051f_uart.h"



signed char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned char *pOutLenBit)
{
    signed char returnStatus;
    unsigned char irqEn;
    unsigned char waitFor;
    unsigned char lastBits;
    unsigned char n;
	
    if (Command == PCD_AUTHENT){
          irqEn   = 0x12;
          waitFor = 0x10;
			}else if(Command == PCD_TRANSCEIVE){
          irqEn   = 0x77;
          waitFor = 0x30;
			}
		//---------------------------------//
		lastBits = irqEn|0x80;
		if(I2C_write_str(SLA_ADDR, ComIEnReg, &lastBits, 0x01)) {
			returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0, "ComIe\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//---------------------------------//
		
		if(I2C_read_str(SLA_ADDR, ComIrqReg, &lastBits, 0x01)){//clearbitmask
			lastBits &= 0x7f;
			I2C_write_str(SLA_ADDR, ComIrqReg, &lastBits, 0x01);
		} else {
			UART_send_str(NR_UART0, "comIR\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//---------------------------------//
		
		lastBits = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &lastBits, 0x01)) {
			returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0,  "CMD00\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//---------------------------------//
		
    if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &lastBits, 0x01)){//setbitmask
			lastBits |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &lastBits, 0x01);
			returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0, "FILev\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
    //---------------------------------//
		
		if(I2C_write_str(SLA_ADDR, FIFODataReg, pInData, InLenByte)) {
			returnStatus = MI_OK;
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "TFIFODa\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		lastBits = Command;
    if(I2C_write_str(SLA_ADDR, CommandReg, &lastBits, 0x01)) {
		returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0, "TranCMD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
   
    
    if (Command == PCD_TRANSCEIVE){
			if(I2C_read_str(SLA_ADDR, BitFramingReg, &lastBits, 0x01)){//setbitmask
				lastBits |= 0x80;
				I2C_write_str(SLA_ADDR, BitFramingReg, &lastBits, 0x01);
				returnStatus = MI_OK;
			} else {
				UART_send_str(NR_UART0, "BitFmin\n");
				returnStatus = MI_ERR;
				return returnStatus;
			}
		}
    for(returnStatus=0; returnStatus<127; returnStatus++){
			lastBits = 255;
			do 
			{
        		I2C_read_str(SLA_ADDR, ComIrqReg, &n, 0x01);
        		lastBits--;
			}
			while ((lastBits!=0) && !(n&0x01) && !(n&waitFor));
		}
		
		if(lastBits == 0) {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComRD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
    //ClearBitMask(BitFramingReg,0x80);
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &returnStatus, 0x01)){//clearbitmask
			returnStatus &= 0x7f;
			I2C_write_str(SLA_ADDR, BitFramingReg, &returnStatus, 0x01);
			returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0, "BitFrm2\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
	      
    if(I2C_read_str(SLA_ADDR, ErrorReg, &lastBits, 0x01)){
			if(!(lastBits&0x1B)) {
				if(n & irqEn & 0x01) {
					returnStatus = MI_NOTAGERR;
					} else {
						if (Command == PCD_TRANSCEIVE){
							I2C_read_str(SLA_ADDR, FIFOLevelReg, &n, 0x01);
							I2C_read_str(SLA_ADDR, ControlReg, &lastBits, 0x01);
							lastBits &= 0x07;
							if (lastBits) {
								*pOutLenBit = (n-1)*8 + lastBits;
								} else { 
								*pOutLenBit = n*8; 
								}
							if(n == 0){ 
								n = 1;
								}
							if(n > MAXRLEN){
								n = MAXRLEN;
								}
						
							if(!I2C_read_str(SLA_ADDR, FIFODataReg, pOutData, n)){
								while(gTXReady[NR_UART0]==0){;};
								UART_send_str(NR_UART0, "FID\n");
								returnStatus =  MI_ERR;
							} else {
								returnStatus = MI_OK;
							}
						}else
							{
							returnStatus = MI_OK;
							}
					}
			} else {
				while(gTXReady[NR_UART0]==0){;};
				UART_send_str(NR_UART0, "30Err\n");
				returnStatus = MI_ERR;
			}
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "31Err\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
   

   	if(I2C_read_str(SLA_ADDR, ControlReg, &lastBits, 0x01)){//setbitmask
			lastBits |= 0x80;
			I2C_write_str(SLA_ADDR, ControlReg, &lastBits, 0x01);
			returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0, "18ComIrq\n");
			returnStatus = MI_ERR;
		}
		
   	lastBits = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &lastBits, 0x01)) {
		returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0, "CMD04\n");
			returnStatus = MI_ERR;
		} 
   return returnStatus;
}


void calulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
	unsigned char tmpVal, n;
	if(I2C_read_str(SLA_ADDR, DivIrqReg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xfb;
			I2C_write_str(SLA_ADDR, DivIrqReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "50divirq\n");
			return;
		}
		
	tmpVal = PCD_IDLE;
	if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "51CMD\n");
			return ;
		}
		
	if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "52FIFOL\n");
			return ;
		}
		
	if(I2C_write_str(SLA_ADDR, FIFODataReg, pIndata, len)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "53Dat\n");
			return ;
		}
		
	tmpVal = PCD_CALCCRC;
	if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "54CMD\n");
			return ;
		}
	
	
	tmpVal = 0xFF;
	do
		{
			I2C_read_str(SLA_ADDR, DivIrqReg, &n, 0x01);
			tmpVal--;
		}while ((tmpVal!=0) && !(n&0x04));
		
	I2C_read_str(SLA_ADDR, CRCResultRegL, pOutData, 0x01);
	I2C_read_str(SLA_ADDR, CRCResultRegM, pOutData+1, 0x01); 
		return;
}

signed char pcdReset(void)
{
		unsigned char tmpVal;
		tmpVal = PCD_RESETPHASE;
		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
			UART_send_str(NR_UART0, "step1\n");
		}
		I2C_write_str(SLA_ADDR,CommandReg, &tmpVal, 0x01);
    _nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();
    
		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
			UART_send_str(NR_UART0, "step2\n");
		}
		tmpVal = 0x3d;
    if(I2C_write_str(SLA_ADDR, ModeReg, &tmpVal, 0x01)) {           //和Mifare卡通讯，CRC初始值0x6363
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "01ModeReg\n");
			return MI_ERR;
		}
		
		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) { 
			UART_send_str(NR_UART0, "step3\n");
		}
		tmpVal = 0x86;
		if(I2C_write_str(SLA_ADDR, RxSelReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "02RxSelReg\n");
			return MI_ERR;
		}
		
		tmpVal = 30;
		if(I2C_write_str(SLA_ADDR, TReloadRegL, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "03TReloL\n");
			return MI_ERR;
		}

		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) { 
			UART_send_str(NR_UART0, "step4\n");	
		}
		tmpVal = 0;	
    if(I2C_write_str(SLA_ADDR, TReloadRegH, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "04TReloH\n");
			return MI_ERR;
		}
		
		
		tmpVal = 0x8d;
    if(I2C_write_str(SLA_ADDR, TModeReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "05TModeR\n");
			return MI_ERR;
		}
		
		
		tmpVal = 0x3e;
    if(I2C_write_str(SLA_ADDR, TPrescalerReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "06TPrescal\n");
			return MI_ERR;
		}
		
		
		tmpVal = 0x40;
    if(I2C_write_str(SLA_ADDR, TxAutoReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "07TxAuto\n");
			return MI_ERR;
		}
		
		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
			UART_send_str(NR_UART0, "step5\n");
		}
		if(I2C_read_str(SLA_ADDR, TxControlReg, &tmpVal, 0x01)){ //turn off the antenna and turn it on again
			
			tmpVal &= 0xfc;
			I2C_write_str(SLA_ADDR, TxControlReg, &tmpVal, 0x01);
			_nop_();_nop_();_nop_();_nop_();
			_nop_();_nop_();_nop_();_nop_();
			
			tmpVal |= 0x03;
			I2C_write_str(SLA_ADDR, TxControlReg, &tmpVal, 0x01);
			
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "08TxCtl\n");
			return MI_ERR;
		}
		
		while(gTXReady[NR_UART0]==0){;};
		UART_send_str(NR_UART0, "rstok\n");
    return MI_OK;
}


signed char pcdRequest(unsigned char *pOutData, unsigned char *pOutLenBit)
{
		unsigned char tmpVal, returnStatus;
	
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xf7;
			I2C_write_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "19statu2\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal = 0x07;
    if(I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "20BitF\n");
			return MI_ERR;
		}
		

		if(I2C_read_str(SLA_ADDR, TxControlReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x03;
			I2C_write_str(SLA_ADDR, TxControlReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "21txq\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal=0x52;
		returnStatus = PcdComMF522(PCD_TRANSCEIVE,&tmpVal,1,pOutData,pOutLenBit);
			
		return returnStatus;
}


signed char pcdAnticoll(unsigned char *pSnr)
{
		unsigned char tmpVal, returnStatus;
		unsigned char comBuf[2];
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xf7;
			I2C_write_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "34statu2\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal = 0x00;
    if(I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "35BitF\n");
			return MI_ERR;
		}
		

		if(I2C_read_str(SLA_ADDR, CollReg, &tmpVal, 0x01)){//setbitmask
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, CollReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "36coll\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//-------------------------------------------//
		tmpVal = 0x77|0x80;
    if(I2C_write_str(SLA_ADDR, ComIEnReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComIEn\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComIrq\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "Comma\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComIrqR\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		

		comBuf[0] = PICC_ANTICOLL1;  comBuf[1] = 0x20;
		if(I2C_write_str(SLA_ADDR, FIFODataReg, comBuf, 0x02)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "TFIFODa\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal = 0x0c;//trans cmd
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "41.x CMD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "BitF\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		for(returnStatus = 0; returnStatus < 50; returnStatus++) {
		 		tmpVal = 255;
				do 
				{
						 I2C_read_str(SLA_ADDR, ComIrqReg, comBuf, 0x01);
						 tmpVal--;
				}
				while ((tmpVal != 0) && !(comBuf[0] & 0x01) && !(comBuf[0] & 0x30));
		}
			
		if(tmpVal == 0) {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "43cantRD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "44BitFram\n");
			returnStatus = MI_ERR;
		}
		
		if(I2C_read_str(SLA_ADDR, ErrorReg, &tmpVal, 0x01)){
			if(!(tmpVal&0x1B)) {
				if(comBuf[0] & 0x77 & 0x01) {
					//while(gTXReady[NR_UART0]==0){;};
					//UART_send_str(NR_UART0, "NOTAG2\n");
					returnStatus = MI_NOTAGERR;
					} else {
						//while(gTXReady[NR_UART0]==0){;};//add for test
						//UART_send_str(NR_UART0, "t2\n");//add for test
						I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
            if(tmpVal > MAXRLEN){
							tmpVal=MAXRLEN;
						} else if(tmpVal == 0) {
							tmpVal = 1;
						}
						if(!I2C_read_str(SLA_ADDR, FIFODataReg, pSnr, tmpVal)){
								while(gTXReady[NR_UART0]==0){;};
								UART_send_str(NR_UART0, "CntRDSn\n");
								returnStatus = MI_ERR;
								return returnStatus;
							} else {
								returnStatus = MI_OK;
							}
					}
			} else {
				while(gTXReady[NR_UART0]==0){;};
				UART_send_str(NR_UART0, "45Err\n");
				returnStatus = MI_ERR;
				return returnStatus;
			}
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "46Err\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		if(I2C_read_str(SLA_ADDR, ControlReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, ControlReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "47ComIrq\n");
			returnStatus = MI_ERR;
		}
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "48Comma\n");
			returnStatus = MI_ERR;
		}
		//---------------------------------------------------//
		
		
		if(I2C_read_str(SLA_ADDR, CollReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, CollReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "49ComIrq\n");
			returnStatus = MI_ERR;
		}
		
		return returnStatus;
}

signed char pcdSelect(unsigned char *pSnr)
{
		unsigned char ucComMF522Buf[9];
		unsigned char tmpVal, returnStatus, lastBits;
	
		ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (tmpVal = 0; tmpVal < 4; tmpVal++){
    	ucComMF522Buf[tmpVal+2] = *(pSnr+tmpVal);
    	ucComMF522Buf[6]  ^= *(pSnr+tmpVal);
    }
		
    calulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
		
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xf7;
			I2C_write_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "Sel_sta2\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		//-------------------------------------------//
		tmpVal = 0x77|0x80;
    if(I2C_write_str(SLA_ADDR, ComIEnReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComIEn\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComIrq\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "Comma\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComIrqR\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		

		if(I2C_write_str(SLA_ADDR, FIFODataReg, ucComMF522Buf, 0x09)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "TFIFODa\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal = 0x0c;//trans cmd
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "41.x CMD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "BitF\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		for(returnStatus = 0; returnStatus < 50; returnStatus++) {
		 		tmpVal = 255;
				do 
				{
						 I2C_read_str(SLA_ADDR, ComIrqReg, ucComMF522Buf, 0x01);
						 tmpVal--;
				}
				while ((tmpVal != 0) && !(ucComMF522Buf[0] & 0x01) && !(ucComMF522Buf[0] & 0x30));
		}
			
		if(tmpVal == 0) {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "43cantRD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "44BitFram\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, ErrorReg, &tmpVal, 0x01)){
			if(!(tmpVal&0x1B)) {
				if(ucComMF522Buf[0] & 0x77 & 0x01) {
					returnStatus = MI_NOTAGERR;
					} else {

						I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
						I2C_read_str(SLA_ADDR, ControlReg, &lastBits, 0x01);
						lastBits &= 0x07;
            if (lastBits) {
							lastBits = (tmpVal-1)*8 + lastBits;
							} else { 
							lastBits = tmpVal*8; 
						}
						
            if(tmpVal > MAXRLEN){
							tmpVal=MAXRLEN;
						} else if(tmpVal == 0) {
							tmpVal = 1;
						}
						
						if(!I2C_read_str(SLA_ADDR, FIFODataReg, ucComMF522Buf, tmpVal)){
								while(gTXReady[NR_UART0]==0){;};
								UART_send_str(NR_UART0, "CntRDSn\n");
								returnStatus = MI_ERR;
								return returnStatus;
							} else {
								returnStatus = MI_OK;
							}
					}
			} else {
				while(gTXReady[NR_UART0]==0){;};
				UART_send_str(NR_UART0, "45Err\n");
				returnStatus = MI_ERR;
				return returnStatus;
			}
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "46Err\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		if(I2C_read_str(SLA_ADDR, ControlReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, ControlReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "47ComIrq\n");
			returnStatus = MI_ERR;
		}
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "48Comma\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//---------------------------------------------------//
		if ((returnStatus == MI_OK) && (lastBits == 0x18)){
			returnStatus = MI_OK;
		} else {
			if(returnStatus != MI_OK) {
				while(gTXReady[NR_UART0]==0){;};
				UART_send_str(NR_UART0, "statErr\n");
				returnStatus = MI_ERR;
			}else if(lastBits != 0x18) {
				while(gTXReady[NR_UART0]==0){;};
				UART_send_str(NR_UART0, "outbitErr\n");
				returnStatus = MI_ERR;
			}
		}
		
		return returnStatus;
}


signed char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
		unsigned char tmpVal, returnVal, returnStatus;
		unsigned char ucComMF522Buf[12];
	
		ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
	
    for (tmpVal=0; tmpVal<6; tmpVal++){
			ucComMF522Buf[tmpVal+2] = *(pKey + tmpVal);
		}
		
    for (tmpVal=0; tmpVal<4; tmpVal++){
			ucComMF522Buf[tmpVal+8] = *(pSnr + tmpVal);
		}

		tmpVal = 0x12|0x80;
    if(I2C_write_str(SLA_ADDR, ComIEnReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "09ComIEn\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "10ComIrq\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "11Comma\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "12ComIrqReg\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		

			if(I2C_write_str(SLA_ADDR, FIFODataReg, ucComMF522Buf, 12)) {
			} else {
				UART_send_str(NR_UART0, "13FIFODa\n");
				returnStatus = MI_ERR;
				return returnStatus;
			}
		
		tmpVal = 0x0E;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "15CMD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		for(returnStatus = 0; returnStatus < 50; returnStatus++) {
			tmpVal = 255;
			do 
			{
				 I2C_read_str(SLA_ADDR, ComIrqReg, &returnVal, 0x01);
				 tmpVal--;
			}
			while ((tmpVal != 0) && !(returnVal & 0x01) && !(returnVal & 0x10));
		}
			
		if(tmpVal == 0) {
			UART_send_str(NR_UART0, "cantRD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "14BitFram\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, ErrorReg, &tmpVal, 0x01)){
			if(!(tmpVal&0x1B)) {
				if(returnVal&0x01&0x12) { 
					returnStatus = MI_NOTAGERR;
					} else {
						returnStatus = MI_OK;
					}
			} else {
				UART_send_str(NR_UART0, "15Err\n");
				returnStatus = MI_ERR;
			}
		} else {
			UART_send_str(NR_UART0, "16Err\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		

		if(I2C_read_str(SLA_ADDR, ControlReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, ControlReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "18ComIrq\n");
			returnStatus = MI_ERR;
		}
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "17Comma\n");
			returnStatus = MI_ERR;
		}
		
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){
			if(!(tmpVal & 0x08)) {
				returnStatus = MI_ERR;
				return returnStatus;
			}
		}
		
		return returnStatus;
	}
	
	

signed char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    calulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    //status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        for (i=0; i<16; i++) { 
					ucComMF522Buf[i] = *(pData+i);
				}
				
				calulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        //status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}

	
#include "c8051f300.h"
#include "mfrc522.h"
#include <intrins.h>
#include "common_i2c.h"
#include "c8051f_uart.h"



signed char pcd_com_MF522(unsigned char Command, 
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
		lastBits = (irqEn|0x80);
		if(I2C_write_str(SLA_ADDR, ComIEnReg, &lastBits, 0x01)) {
			returnStatus = MI_OK;
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "ComIe\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//---------------------------------//
		
		if(I2C_read_str(SLA_ADDR, ComIrqReg, &lastBits, 0x01)){//clearbitmask
			lastBits &= 0x7f;
			I2C_write_str(SLA_ADDR, ComIrqReg, &lastBits, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "comIR\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//---------------------------------//
		
		lastBits = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &lastBits, 0x01)) {
			returnStatus = MI_OK;
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0,  "CMD00\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		//---------------------------------//
		
    if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &lastBits, 0x01)){//setbitmask
			lastBits |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &lastBits, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "FILev\n");
			returnStatus = MI_ERR;
			goto label_end_com;
		}
    //---------------------------------//
		
		if(I2C_write_str(SLA_ADDR, FIFODataReg, pInData, InLenByte)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "TFIFODa\n");
			returnStatus = MI_ERR;
			goto label_end_com;
		}
		
    if(I2C_write_str(SLA_ADDR, CommandReg, &Command, 0x01)) {
		returnStatus = MI_OK;
		} else {
			UART_send_str(NR_UART0, "TranCMD\n");
			returnStatus = MI_ERR;
			goto label_end_com;
		}
   
    
    if (Command == PCD_TRANSCEIVE){
			if(I2C_read_str(SLA_ADDR, BitFramingReg, &lastBits, 0x01)){//setbitmask
				lastBits |= 0x80;
				I2C_write_str(SLA_ADDR, BitFramingReg, &lastBits, 0x01);

			} else {
				UART_send_str(NR_UART0, "BitFmin\n");
				returnStatus = MI_ERR;
				goto label_end_com;
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
			goto label_end_com;
		}
		
    //ClearBitMask(BitFramingReg,0x80);
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &returnStatus, 0x01)){//clearbitmask
			returnStatus &= 0x7f;
			I2C_write_str(SLA_ADDR, BitFramingReg, &returnStatus, 0x01);
		} else {
			UART_send_str(NR_UART0, "BitFrm2\n");
			returnStatus = MI_ERR;
			goto label_end_com;
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
		}
   
label_end_com:
   	if(I2C_read_str(SLA_ADDR, ControlReg, &lastBits, 0x01)){//setbitmask
			lastBits |= 0x80;
			I2C_write_str(SLA_ADDR, ControlReg, &lastBits, 0x01);
		} else {
			UART_send_str(NR_UART0, "18ComIrq\n");
			returnStatus = MI_ERR;
		}
		
   	lastBits = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &lastBits, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "CMD04\n");
			returnStatus = MI_ERR;
		} 
   return returnStatus;
}


void calulate_CRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
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

signed char pcd_reset(void)
{
		unsigned char tmpVal;
		tmpVal = PCD_RESETPHASE;
		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
			//UART_send_str(NR_UART0, "step1\n");
		}
		I2C_write_str(SLA_ADDR,CommandReg, &tmpVal, 0x01);
    _nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();
    
		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) {
			//UART_send_str(NR_UART0, "step2\n");
		}
		tmpVal = 0x3d;
    if(I2C_write_str(SLA_ADDR, ModeReg, &tmpVal, 0x01)) {           //和Mifare卡通讯，CRC初始值0x6363
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "01ModeReg\n");
			return MI_ERR;
		}
		
		if(gTXReady[NR_UART0] == 1 && gUARTBufferSize[NR_UART0] == 0) { 
			//UART_send_str(NR_UART0, "step3\n");
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
			//UART_send_str(NR_UART0, "step4\n");	
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
			//UART_send_str(NR_UART0, "step5\n");
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


signed char pcd_request(unsigned char *pOutData, unsigned char *pOutLenBit)
{
		unsigned char tmpVal, returnStatus;
	
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xf7;
			I2C_write_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01);
			returnStatus = MI_OK;
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "19statu2\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal = 0x07;
    if(I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)) {
			returnStatus = MI_OK;
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "20BitF\n");
			return MI_ERR;
		}
		

		if(I2C_read_str(SLA_ADDR, TxControlReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x03;
			I2C_write_str(SLA_ADDR, TxControlReg, &tmpVal, 0x01);
			returnStatus = MI_OK;
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "21txq\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal=0x52;
		returnStatus = pcd_com_MF522(PCD_TRANSCEIVE,&tmpVal,1,pOutData,pOutLenBit);
			
		return returnStatus;
}


signed char pcd_anti_coll(unsigned char *pSnr)
{
		unsigned char tmpVal, returnStatus, snrCheck;
		unsigned char data ucComMF522Buf[5];
		
		snrCheck = 0;
	
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xf7;
			I2C_write_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "34statu2\n");
			returnStatus = MI_ERR;
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
			return MI_ERR;
		}
		
		ucComMF522Buf[0] = PICC_ANTICOLL1;
		ucComMF522Buf[1] = 0x20;


		returnStatus = pcd_com_MF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&tmpVal);
		
		if (returnStatus == MI_OK){
    	 for (tmpVal=0; tmpVal<4; tmpVal++){   
             *(pSnr+tmpVal)  = ucComMF522Buf[tmpVal];
             snrCheck ^= ucComMF522Buf[tmpVal];
         }
				 
         if (snrCheck != ucComMF522Buf[tmpVal]){
						returnStatus = MI_ERR;
				}
    }
		
		if(I2C_read_str(SLA_ADDR, CollReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, CollReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "49ComIrq\n");
			return MI_ERR;
		}
		
		return returnStatus;
}

signed char pcd_select(unsigned char *pSnr)
{
		unsigned char ucComMF522Buf[9];
		unsigned char tmpVal, returnStatus;
	
		ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (tmpVal = 0; tmpVal < 4; tmpVal++){
    	ucComMF522Buf[tmpVal+2] = *(pSnr+tmpVal);
    	ucComMF522Buf[6]  ^= *(pSnr+tmpVal);
    }
		
    calulate_CRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
		
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xf7;
			I2C_write_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "Sel_sta2\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}

		returnStatus = pcd_com_MF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&tmpVal);
		
		if ((returnStatus == MI_OK) && (tmpVal == 0x18)){
			returnStatus = MI_OK;
		} else {
			if(returnStatus != MI_OK) {
				while(gTXReady[NR_UART0]==0){;};
				UART_send_str(NR_UART0, "statErr\n");
				returnStatus = MI_ERR;
			}else if(tmpVal != 0x18) {
				while(gTXReady[NR_UART0]==0){;};
				UART_send_str(NR_UART0, "outbitErr\n");
				returnStatus = MI_ERR;
			}
		}
		
		return returnStatus;
}


signed char pcd_auth_state(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
		unsigned char tmpVal, returnStatus;
		unsigned char ucComMF522Buf[12];
	
		ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
	
    for (tmpVal=0; tmpVal<6; tmpVal++){
			ucComMF522Buf[tmpVal+2] = *(pKey + tmpVal);
		}
		
    for (tmpVal=0; tmpVal<4; tmpVal++){
			ucComMF522Buf[tmpVal+8] = *(pSnr + tmpVal);
		}

		returnStatus = pcd_com_MF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&tmpVal);
		
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){
			if(!(tmpVal & 0x08)) {
				returnStatus = MI_ERR;
				return returnStatus;
			}
		}
		
		return returnStatus;
	}
	
	
signed char pcd_read(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
	
			
    calulate_CRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
					
    status = pcd_com_MF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&i);
					
    if ((status == MI_OK) && (i == 0x90)){
        for (i=0; i<16; i++){
					*(pData+i) = ucComMF522Buf[i];
				}
    } else {
			status = MI_ERR;
		}
    
    return status;
}

         
signed char pcd_write(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    calulate_CRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = pcd_com_MF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&i);

    if ((status != MI_OK) || (i != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A)){
			status = MI_ERR;
		}
        
    if (status == MI_OK){
        for (i=0; i<16; i++){
						ucComMF522Buf[i] = *(pData+i);
				}
        calulate_CRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = pcd_com_MF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&i);
        if ((status != MI_OK) || (i != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A)){
					status = MI_ERR;
				}
		}
    
    return status;
}


#ifdef DEF_SET_AUTH_KEYA 
signed char pcd_set_keyA(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
	
    if(addr%4 != 3){
			return MI_ERR;
		}
		
			
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    calulate_CRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = pcd_com_MF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&i);

    if ((status != MI_OK) || (i != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A)){
			status = MI_ERR;
		}
        
    if (status == MI_OK){
        for (i=0; i<6; i++){
						ucComMF522Buf[i] = *(pData+i);
				}
				
				ucComMF522Buf[6] = 0xff; ucComMF522Buf[7] = 0x07; ucComMF522Buf[8] = 0x80; ucComMF522Buf[9] = *pData;
				
				for (i=10; i<16; i++){
						ucComMF522Buf[i] = 0x00;
				}
				
        calulate_CRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = pcd_com_MF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&i);
        if ((status != MI_OK) || (i != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A)){
					status = MI_ERR;
				}
		}
    
    return status;
}
#endif
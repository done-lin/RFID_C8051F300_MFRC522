#include "c8051f300.h"
#include "mfrc522.h"
#include <intrins.h>
#include "common_i2c.h"
#include "c8051f_uart.h"

unsigned char code authentKeyA[2][12]={{0x60, 0x3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00},
	{0x60, 0x3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00}};

	
void calulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
	unsigned char tmpVal, n;
	if(I2C_read_str(SLA_ADDR, DivIrqReg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xfb;
			I2C_write_str(SLA_ADDR, DivIrqReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "50divirq\n");
			return;
		}
		
	tmpVal = PCD_IDLE;
	if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "51CMD\n");
			return ;
		}
		
	if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "52FIFOL\n");
			return ;
		}
		
	if(I2C_write_str(SLA_ADDR, FIFODataReg, pIndata, len)) {
		} else {
			UART_send_str(NR_UART0, "53Dat\n");
			return ;
		}
		
	tmpVal = PCD_CALCCRC;
	if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
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
		unsigned char tmpVal, returnVal, returnStatus, lastBits;

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
		
		tmpVal = 0xf7;
    if(I2C_write_str(SLA_ADDR, ComIEnReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "22ComIEn\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "23ComIrq\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "24Comma\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "25ComIrqReg\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		

		tmpVal = 0x52;
		if(I2C_write_str(SLA_ADDR, FIFODataReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "26TFIFODa\n");
			return MI_ERR;
		}
		
		tmpVal = 0x0c;
		if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "26.5cmd\n");
			return MI_ERR;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "27BitF\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		for(lastBits = 0; lastBits < 50; lastBits++) {
			tmpVal = 255;
			do 
			{
				 I2C_read_str(SLA_ADDR, ComIrqReg, &returnVal, 0x01);
				 tmpVal--;
			}
			while ((tmpVal != 0) && !(returnVal & 0x01) && !(returnVal & 0x30));
		}
			
		if(tmpVal == 0) {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "28cantRD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "29BitFram\n");
			returnStatus = MI_ERR;
		}

			
		if(I2C_read_str(SLA_ADDR, ErrorReg, &tmpVal, 0x01)){
			if(!(tmpVal&0x1B)) {
				if(returnVal & 0x77 & 0x01) {
					//UART_send_str(NR_UART0, "NOTAGERR\n");
					returnStatus = MI_NOTAGERR;
					} else {
						//while(gTXReady[NR_UART0]==0){;};//add for test
						//UART_send_str(NR_UART0, "t1\n");//add for test
							
						if(!I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)) { 
							while(gTXReady[NR_UART0]==0){;};
							UART_send_str(NR_UART0, "FIL\n");
						}
            I2C_read_str(SLA_ADDR, ControlReg, &lastBits, 0x01);
						lastBits &= 0x07;
            if (lastBits) {
							*pOutLenBit = (tmpVal-1)*8 + lastBits;
							} else { 
							*pOutLenBit = tmpVal*8; 
							}
						if (tmpVal == 0) { 
							tmpVal = 1;
							}
						if (tmpVal > MAXRLEN){
							tmpVal = MAXRLEN;
							}
						
						if(!I2C_read_str(SLA_ADDR, FIFODataReg, pOutData, tmpVal)){
								while(gTXReady[NR_UART0]==0){;};
								UART_send_str(NR_UART0, "FID\n");
								return MI_ERR;
							} else {
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

		
		if(I2C_read_str(SLA_ADDR, ControlReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, ControlReg, &tmpVal, 0x01);
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "33ComIrq\n");
			returnStatus = MI_ERR;
		}
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			while(gTXReady[NR_UART0]==0){;};
			UART_send_str(NR_UART0, "32Comma\n");
			returnStatus = MI_ERR;
		}
		//while(gTXReady[NR_UART0]==0){;};
		//UART_send_array(0, testBuf, 4);
			
		//while(gTXReady[NR_UART0]==0){;};
		//UART_send_str(NR_UART0, "rq_ok");
			
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
		unsigned char tmpVal, returnStatus;
		unsigned char comBuf[2];
		if(I2C_read_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01)){//clearbitmask
			tmpVal &= 0xf7;
			I2C_write_str(SLA_ADDR, Status2Reg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "34statu2\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal = 0x00;
    if(I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "35BitF\n");
			return MI_ERR;
		}
		

		if(I2C_read_str(SLA_ADDR, CollReg, &tmpVal, 0x01)){//setbitmask
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, CollReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "36coll\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		tmpVal = 0x77|0x80;
    if(I2C_write_str(SLA_ADDR, ComIEnReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "37ComIEn\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, ComIrqReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "38ComIrq\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "39Comma\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "40ComIrqReg\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		

		comBuf[0] = PICC_ANTICOLL1;  comBuf[1] = 0x20;
		if(I2C_write_str(SLA_ADDR, FIFODataReg, comBuf, 0x02)) {
		} else {
			UART_send_str(NR_UART0, "41TFIFODa\n");
			return MI_ERR;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){//setbitmask
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "42BitF\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		for(returnStatus = 0; returnStatus < 5; returnStatus++) {
		 		tmpVal = 255;
				do 
				{
						 I2C_read_str(SLA_ADDR, ComIrqReg, comBuf, 0x01);
						 tmpVal--;
				}
				while ((tmpVal != 0) && !(comBuf[0] & 0x01) && !(comBuf[0] & 0x30));
		}
			
		if(tmpVal == 0) {
			UART_send_str(NR_UART0, "43cantRD\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		if(I2C_read_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01)){
			tmpVal &= 0x7f;
			I2C_write_str(SLA_ADDR, BitFramingReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "44BitFram\n");
			returnStatus = MI_ERR;
		}
		
		if(I2C_read_str(SLA_ADDR, ErrorReg, &tmpVal, 0x01)){
			if(!(tmpVal&0x1B)) {
				if(comBuf[0]&0x77) { 
					returnStatus = MI_NOTAGERR;
					} else {
						
						I2C_read_str(SLA_ADDR, FIFOLevelReg, &tmpVal, 0x01);
            if(tmpVal > MAXRLEN){
						} else if(tmpVal == 0) {
							tmpVal = 1;
						}
						if(!I2C_read_str(SLA_ADDR, FIFODataReg, pSnr, tmpVal)){
								return MI_ERR;
							} else {
								returnStatus = MI_OK;
							}
					}
			} else {
				UART_send_str(NR_UART0, "45Err\n");
				returnStatus = MI_ERR;
			}
		} else {
			UART_send_str(NR_UART0, "46Err\n");
			returnStatus = MI_ERR;
			return returnStatus;
		}
		
		
		if(I2C_read_str(SLA_ADDR, ControlReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, ControlReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "47ComIrq\n");
			returnStatus = MI_ERR;
		}
		
		if(I2C_read_str(SLA_ADDR, CollReg, &tmpVal, 0x01)){
			tmpVal |= 0x80;
			I2C_write_str(SLA_ADDR, CollReg, &tmpVal, 0x01);
		} else {
			UART_send_str(NR_UART0, "49ComIrq\n");
			returnStatus = MI_ERR;
		}

		tmpVal = PCD_IDLE;
    if(I2C_write_str(SLA_ADDR, CommandReg, &tmpVal, 0x01)) {
		} else {
			UART_send_str(NR_UART0, "48Comma\n");
			returnStatus = MI_ERR;
		}
		return returnStatus;
}


signed char pcdAuthent(void)
{
		unsigned char tmpVal, returnVal, returnStatus;
	
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
			UART_send_str(NR_UART0, "10ComIrqReg\n");
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
		
		for(tmpVal=0; tmpVal<14; tmpVal++)
		{
			if(I2C_write_str(SLA_ADDR, FIFODataReg, authentKeyA[0], 0x01)) {
			} else {
				UART_send_str(NR_UART0, "13FIFODa\n");
				returnStatus = MI_ERR;
				return returnStatus;
			}
		}
		
		for(returnStatus = 0; returnStatus < 5; returnStatus++) {
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
				if(returnVal&0xf7) { 
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
				
		return returnStatus;
	}
	
	
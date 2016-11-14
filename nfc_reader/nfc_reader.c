/**
 *
 * @brief       :  
 *
 * @file        : nfc_reader.c
 * @author      : wangjian
 * Version      : v0.0.1
 * Date         : 2016-11-09
 *
 * Change Logs  :
 *
 * Date                 Version           Author          Notes
 * 2016-11-09           v0.1              wangjian        first version 
*/
/* Includes ------------------------------------------------------------------*/
#include "nfc_reader.h"
#include "bsp_com.h"
#include <app_frame.h>
#include <app_send.h>
#include <blu_tx.h>

/* Define --------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
extern osel_etimer_t nfc_reader_rx_data_timer; 
extern osel_etimer_t nfc_reader_tag_process_timer;
RFID_DATA  RxBuffer_RFID_DATA = 
{
  0
};

/* Function prototypes -------------------------------------------------------*/

/********************************************************************
�������ܣ����ڳ�ʼ����
��ڲ������ޡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void NfcReaderInit(void)
{
    unsigned int i;

    hal_uart_init(__BSP_COM1__, 115200); 
    
    BspCom1Init(115200);

    RxBuffer_RFID_DATA.queue.count = 0;

    MasterStopReadEpc();
    MasterStopReadEpc();

    for(i = 0; i < 5; i++)
    {
        ReaderInfoGet();
        delay_ms(100);
    }
	//serial_write(__BSP_COM1__, "nfc_reader_init", sizeof("nfc_reader_init")-1);
}
////////////////////////End of function//////////////////////////////

//=================================================================
//У��ͺ���
//=================================================================
unsigned char SumCheck(unsigned char *uBuff, unsigned int uBuffLen) 
{
    unsigned int i;
	unsigned int uSum=0;
	unsigned char SumCheck=0;

    for(i=0;i<uBuffLen;i++)
    {  
      uSum = uSum + uBuff[i];
    }

    SumCheck = 	uSum & 0x0ff;

    return SumCheck;
}

//=================================================================
//��������:mcuӦ��+����
//�������: ����,Ӧ������+���ݳ���
//����ֵ����
//=================================================================
void MasterSetSelcetVolatiLen(unsigned char *EpcData,unsigned char EpcLen/*EPC LEN*/)
{
	 unsigned char i = 0;
	 unsigned char TxBuf[100]={0x00};
	
	 TxBuf[0] = 0xAA;
	 TxBuf[1] = 0x00;/*Ptr*/
	 TxBuf[2] = 0x0C;//����
	 TxBuf[3] = 0x00;//���ݳ���
	 TxBuf[4] = EpcLen + 7;//���ݳ���
	 TxBuf[5] = 0x01; /*param*/
	 TxBuf[6] = 0x00;/*PtrMSB*/
	 TxBuf[7] = 0x00;
	 TxBuf[8] = 0x00;
	 TxBuf[9] = 0x20;/*PtrLSB*/
	 TxBuf[10] = 0x60;/*masklen*/
	 TxBuf[11] = 0x00;/*truncate*/

	 for(i=0;i<EpcLen;i++)
	 {
	     TxBuf[12+i] = *EpcData;//����
	     EpcData++;
	 }

	 TxBuf[12+i] = SumCheck(&TxBuf[1],EpcLen + 11);//У��ͣ�֡ͷ֡β������У��
	 TxBuf[13+i] = 0x8E;

	 USART_PutStr(TxBuf,14+i);
}

//=================================================================
//��������:mcuӦ��+����
//�������: ����,Ӧ������+���ݳ���
//����ֵ����
//=================================================================
void MasterSetSelcet(unsigned char *EpcData)
{
	 unsigned char i = 0;
	 unsigned char TxBuf[100]={0x00};
	
	 TxBuf[0] = 0xAA;
	 TxBuf[1] = 0x00;/*Ptr*/
	 TxBuf[2] = 0x0C;//����
	 TxBuf[3] = 0x00;//���ݳ���
	 TxBuf[4] = 0x13;//���ݳ���
	 TxBuf[5] = 0x01; /*param*/
	 TxBuf[6] = 0x00;/*PtrMSB*/
	 TxBuf[7] = 0x00;
	 TxBuf[8] = 0x00;
	 TxBuf[9] = 0x20;/*PtrLSB*/
	 TxBuf[10] = 0x60;/*masklen*/
	 TxBuf[11] = 0x00;/*truncate*/

	 for(i = 0;i < 12;i++)
	 {
	     TxBuf[12+i] = *EpcData;//����
	     EpcData++;
	 }

	 TxBuf[24] = SumCheck(&TxBuf[1],23);//У��ͣ�֡ͷ֡β������У��
	 TxBuf[25] = 0x8E;

	 USART_PutStr(TxBuf,26);
}

//=================================================================
//��������:mcuӦ��+����
//�������: ����,Ӧ������+���ݳ���
//����ֵ����
//=================================================================	 
uint8_t ReadEpcExCMD[9] = {0xAA,0x00,0x27,0x00,0x02,0x01,0xf4, 0x1E, 0X8E};                 

void MasterReadExEpc(void)
{
  USART_PutStr(ReadEpcExCMD, 9);
}

//=================================================================
//��������:mcuӦ��+����
//�������: ����,Ӧ������+���ݳ���
//����ֵ����
//=================================================================	 
uint8_t ReadEpcCMD[7] = {0xAA,0x00,0x22,0x00,0x00,0x22,0x8E};

void MasterReadEpc(void)
{
  USART_PutStr(ReadEpcCMD,7);
}

//=================================================================
//��������:mcuӦ��+����
//�������: ����,Ӧ������+���ݳ���
//����ֵ����
//=================================================================	 
uint8_t ReadEpcStopCMD[7] = {0xAA,0x00,0x28,0x00,0x00,0x28,0x8E};

void MasterStopReadEpc(void)
{
  USART_PutStr(ReadEpcStopCMD,7);
}

uint8_t ReadReaderInfoCMD[8] = {0xAA,0x00,0x03,0x00,0x01,0x00,0x04,0x8E};

void ReaderInfoGet(void)
{
  USART_PutStr(ReadReaderInfoCMD,8);
}


//=================================================================
//��������:mcuӦ��+����
//�������: ����,Ӧ������+���ݳ���
//����ֵ����
//=================================================================	 
uint8_t ReadTidCMD[16] = {0xAA,0x00,0x39,0x00,0x09,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x08,0x4C,0x8E};

void MasterReadTid(void)
{
  USART_PutStr(ReadTidCMD,16);
}

uint8_t SetPowerCMD[9] = {0xAA,0x00,0xB6,0x00,0x02,0x0A,0x8C,0x4E,0x8E};

void MasterSetPower(void)
{
  USART_PutStr(SetPowerCMD,9);
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : USARTx����1-3,*UartDat�������ݣ�*RxBuffer���ջ�����
* Output         : None
* Return         : TRUE ������Ч��FALSE������Ч
*******************************************************************************/
unsigned char CheckUartDat(unsigned char *UartDat,unsigned char *RxBuffer)
{
  unsigned char CheckSum = 0;
  unsigned char i = 0;
  static unsigned char RxStatus = 0;   
  static unsigned int RxDatCnt = 0;  
  static unsigned char NewDatFlag = 0;
  
  switch(RxStatus)
  {
     case 0: 
			 if(*UartDat == MasterFrameHeader1)/* 0xAA*/
			 {//���֡ͷ1�� 
					RxStatus = 1;//������һ���ж��Ƿ�Ϊ֡ͷ2
			 }
			 else
			 {
					RxStatus = 0;
			 }
     break;
			 
     case 1:

		  if(RxDatCnt > 3)
		  {
	        RxBuffer_RFID_DATA.DataPL[1] = RxBuffer[3];
		  }
					       
		  if((*UartDat == MasterFrameTail1) && (RxDatCnt >= RxBuffer_RFID_DATA.DataPL[1]+ 5)) /* 0x8e*/
		  {//����յ�֡β1				    
				CheckSum = SumCheck(&RxBuffer[0],RxDatCnt-1);//У������
				if(CheckSum == RxBuffer[RxDatCnt-1])//���У���
				{	 
				    RxStatus = 0;
				    RxDatCnt = 0;

					RxBuffer_RFID_DATA.Type = RxBuffer[0];
					RxBuffer_RFID_DATA.Cmd = RxBuffer[1];
					RxBuffer_RFID_DATA.DataPL[0] = RxBuffer[2];
					RxBuffer_RFID_DATA.DataPL[1] = RxBuffer[3];
                    RxBuffer_RFID_DATA.EPCLen = 0;
                    
					/*****************read EPC********************/
					if((RxBuffer_RFID_DATA.Cmd == 0x22) && (RxBuffer_RFID_DATA.Type == 0x02))
					{
						RxBuffer_RFID_DATA.RSSIBuf = RxBuffer[4];
						RxBuffer_RFID_DATA.PCBuf[0] = RxBuffer[5];
						RxBuffer_RFID_DATA.PCBuf[1] = RxBuffer[6];

						if((RxBuffer_RFID_DATA.DataPL[1] > 100) && (RxBuffer_RFID_DATA.DataPL[0] != 0))
						{
						  return FALSE;
						}

						for(i = 7;i < RxBuffer_RFID_DATA.DataPL[1]+2;i++)
						{
						   RxBuffer_RFID_DATA.EPCBuf[i - 7] = RxBuffer[i];
						}
                        
                        RxBuffer_RFID_DATA.EPCLen = i - 7;
					}
					
					/*****************read TID********************/
					if((RxBuffer_RFID_DATA.Cmd == 0x39) && (RxBuffer_RFID_DATA.Type == 0x01))
					{
						RxBuffer_RFID_DATA.RSSIBuf = RxBuffer[4];
						RxBuffer_RFID_DATA.PCBuf[0] = RxBuffer[5];
						RxBuffer_RFID_DATA.PCBuf[1] = RxBuffer[6];

						if((RxBuffer_RFID_DATA.DataPL[1] > 100) && (RxBuffer_RFID_DATA.DataPL[0] != 0))
						{
						  return FALSE;
						}

						for(i = 7 + RxBuffer_RFID_DATA.EPCLen;i < RxBuffer_RFID_DATA.DataPL[1]+2;i++)
						{
						   RxBuffer_RFID_DATA.TIDBuf[i - 7 - RxBuffer_RFID_DATA.EPCLen] = RxBuffer[i];
						}
					}
                    
					/*****************Set Power********************/
                    if((RxBuffer_RFID_DATA.Cmd == 0xB6) && (RxBuffer_RFID_DATA.Type == 0x01))
					{
						return TRUE;
					}
                    
					return TRUE;
				}
				else
				{
				       //UartPutChar(CheckSum);//���򷵻�У��ֵ
				}

				/********************/
				RxStatus = 0;
				RxDatCnt = 0;
		  }
		  else
		  {	
		    RxBuffer[RxDatCnt++] = *UartDat;//��������
		  }	

		  /*********************/
		 if(RxDatCnt >= 100)//����������ݳ��ȳ������������֡������Ч
		 {
				RxStatus = 0;
				RxDatCnt = 0;
		 }

     break;

     default:
			 RxStatus = 0;
			 RxDatCnt = 0; 
			 NewDatFlag = 0;    
     break;
  }
  return FALSE;
}

////////////////////////End of function//////////////////////////////

/********************************************************************
�������ܣ������ڷ���һ�ֽ����ݡ�
��ڲ�����d: Ҫ���͵��ֽ����ݡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void UartPutChar(uint8_t d)
{
    BspCom1SendData(&d, 1);

//	SBUF0 = d;
//	while(TI0==0);
//	TI0 = 0;				  

}

/********************************************************************
�������ܣ������ڷ���һ�ֽ����ݡ�
��ڲ�����d: Ҫ���͵��ֽ����ݡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void USART_PutStr(uint8_t * pd, uint8_t Len)
{
    uint8_t i = 0;

	while(i++ < Len) //�����ַ���
	{
	  UartPutChar(*pd); //����һ���ַ�
	  pd++;  //�ƶ�����һ���ַ�
	}			  
}


/********************************************************************
�������ܣ������жϴ���
��ڲ������ޡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void NfcReaderRxProcess(void)
{
	unsigned char UartData;
    unsigned char i;
    RfidTagInfo item;
    RfidTagInfo *temp;
    unsigned char flag = 0;
    
    while (!BspCom1RxFIFOIsEmpty())
    {
        UartData = BspCom1RxFIFOOut();
        
        /**************************/
        if(RxBuffer_RFID_DATA.UartDataCheckOk == 0)
        {    
              if(CheckUartDat(&UartData, RxBuffer_RFID_DATA.UartRxBuffer)==TRUE)
              {
                   RxBuffer_RFID_DATA.UartDataCheckOk = 1;
                   
                   if ((RxBuffer_RFID_DATA.EPCLen <= 12) && (RxBuffer_RFID_DATA.EPCLen > 0))
                   {    
                       for (i = 0; i < RxBuffer_RFID_DATA.queue.count; i++)
                       {
                            temp = &RxBuffer_RFID_DATA.queue.item[i];
                            
                            if (memcmp(RxBuffer_RFID_DATA.EPCBuf, temp->epc, temp->ep_len) == 0)
                            {
                                break;
                            }
                       }

                       if ((i >= RxBuffer_RFID_DATA.queue.count) && (i < RFID_TAG_QUEUE_SIZE))
                       {
                            temp = &RxBuffer_RFID_DATA.queue.item[i];
                            
                            temp->pc      = RxBuffer_RFID_DATA.PCBuf[0];
                            temp->rssi    = RxBuffer_RFID_DATA.RSSIBuf;
                            temp->ep_len  = RxBuffer_RFID_DATA.EPCLen;
                            memcpy(temp->epc, RxBuffer_RFID_DATA.EPCBuf, temp->ep_len);

                            RxBuffer_RFID_DATA.queue.count++;
                       }
                   }
                   
                   flag = 1;
                   RxBuffer_RFID_DATA.UartDataCheckOk = 0;
              }
        }

    }
//RxBuffer_RFID_DATA.queue.count = 10;
    if (flag == 1)
    {
      //  BoxSendRfidTagInfo();

      //  RxBuffer_RFID_DATA.queue.count = 0;
    }

    //
}
void NfcReaderTagInfoUpdate(void)
{
    BoxSendRfidTagInfo();
    
    RxBuffer_RFID_DATA.queue.count = 0;
}

void NfcReaderRxProcesstimerStart(void)
{
    //MasterReadEpc();
    //MasterReadExEpc();
    
    osel_etimer_disarm(&nfc_reader_rx_data_timer);//GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS

    osel_etimer_arm(&nfc_reader_rx_data_timer, (91 / OSEL_TICK_PER_MS), 0);//GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS
}

void NfcReaderRxProcesstimerStop(void)
{
    osel_etimer_disarm(&nfc_reader_rx_data_timer);//GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS
}

void NfcReaderTagProcesstimerStart(uint16_t delay)
{
    BspCom1RxFIFOClear();
    
    MasterSetPower();

    delay_ms(10);
    
    //MasterReadEpc();
    MasterReadExEpc();
    
    osel_etimer_disarm(&nfc_reader_tag_process_timer);//GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS

    osel_etimer_arm(&nfc_reader_tag_process_timer, (delay * 100/OSEL_TICK_PER_MS), 0);//GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS

}

void NfcReaderTagProcesstimerStop(void)
{
    MasterStopReadEpc();

    MasterStopReadEpc();
    
    osel_etimer_disarm(&nfc_reader_tag_process_timer);//GPS_OPEN_TIME*1000/OSEL_TICK_PER_MS
}

void BoxSendRfidTagInfo(void)
{
    uint8_t buffer[RFID_TAG_QUEUE_SIZE * 12 + 1 + 1] = {0};
    uint8_t *p;
    uint8_t i;
    uint8_t len;
    RfidTagInfo *temp;
    
    extern uint16_t BluDataSn;

    buffer[0] = BOX_BLU_CMD_TAGINFO + BOX_BLU_CMD_REPLY_HEAD;
    buffer[1] = RxBuffer_RFID_DATA.queue.count;
    p = &buffer[2];
    len = 1 + buffer[1] * 12;
    
    for (i = 0; i < RxBuffer_RFID_DATA.queue.count; i++)
    {
        temp = &RxBuffer_RFID_DATA.queue.item[i];
        
        memcpy (&p[12 * i],temp->epc, temp->ep_len); 
    }
    
    blu_tran_send(buffer, len, BluDataSn);
    
}


/**
 *
 * @brief       :  
 *
 * @file        : nfc_reader.h
 * @author      : wangjian
 * Version      : v0.0.1
 * Date         : 2016-11-09
 *
 * Change Logs  :
 *
 * Date                 Version           Author          Notes
 * 2016-11-09           v0.1              wangjian        first version 
*/
#ifndef  __NFC_READER__
#define  __NFC_READER__

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <gznet.h>

/* Define --------------------------------------------------------------------*/
#define USE_RFID_READER          0
#define MasterFrameHeader1       0xAA //帧头1
#define MasterFrameTail1         0x8E //帧尾1
#define NFC_READER_RX_DATA_TIME  2
#define RFID_TAG_QUEUE_SIZE      10

/* Exported types ------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
typedef enum 
{
	READ_EPC_CMD = 0,  /*EPC*/						
	EPC_SEL_CMD,    /*EPC SEL*/
	READ_TID_CMD,	   /*READ TID*/	
	NC_CMD,	  
}CMDStatus__;

#pragma pack(1)
typedef struct
{
    uint16_t pc;
    uint8_t rssi;
    uint8_t ep_len;
    uint8_t epc[12];
} RfidTagInfo;
#pragma pack()

typedef struct 
{
    uint8_t count;
    RfidTagInfo item[RFID_TAG_QUEUE_SIZE];       
}RfidTagQueue;


typedef struct 
{
	uint8_t Type;	  /*帧类型*/
	uint8_t Cmd;	 /*命令*/
	uint16_t DataPL[2]; /*参数长度*/
	uint8_t RSSIBuf;	/*RSSI*/
	uint16_t PCBuf[2];	/*PC*/
	uint8_t EPCBuf[50];/*EPC号码*/
	uint8_t TIDBuf[50];/*TID号码*/
	uint8_t TID_EPC_TmpBuf[50];/*TID EPC号码*/
	uint8_t EPCLen;/*EPC号码*/
	uint8_t TIDLen;/*EPC号码*/

	uint8_t UartRxBuffer[100];
	uint8_t UartDataCheckOk;	/*接收OK标志*/
	uint8_t TimeOverCnt;	/*超时计数*/
    RfidTagQueue queue;
}RFID_DATA;

/* Function prototypes -------------------------------------------------------*/
void NfcReaderInit(void);
void NfcReaderRxProcess(void);
void MasterReadExEpc(void);
void MasterSetPower(void);
void ReaderInfoGet(void);
void MasterReadEpc(void);
void MasterStopReadEpc(void);
void BoxSendRfidTagInfo(void);

void UartPutChar(uint8_t);
void UartPutString(uint8_t * pd,uint8_t cnt);
void USART_PutStr(uint8_t * pd,uint8_t Len);
void MasterSetSelcet(uint8_t *EpcData);
void MasterSetSelcetVolatiLen(uint8_t *EpcData, uint8_t EpcLen/*EPC LEN*/);

#ifdef __cplusplus
}
#endif

#endif

#pragma once

#define         APP_TASK_PRIO           (3u)

typedef enum
{
    APP_SERIAL_EVENT =  ((APP_TASK_PRIO<<8) | 0x01),
    APP_RF_TXOK_EVENT,
    APP_RF_RXOK_EVENT,
} app_task_sig_enum_t;



#define APP_CMD_SIZE_MAX                    50u

#define SERIAL_LEN_MAX                     (44u)   // 协议中最大帧长
#define SERIAL_LEN_MIN                     (1u)    // 协议中最小帧长

#define CMD_AT_HEAD_SIZE                    2u
#define CMD_CR                              0x0D
#define CMD_LF                              0x0A

#define CMD_SYSSTART                        "SYSSTART\r\n"
#define CMD_OK                              "OK\r\n"

#define CMD_ERROR00                         "ERROR00\r\n"
#define CMD_ERROR01                         "ERROR01\r\n"
#define CMD_ERROR02                         "ERROR02\r\n"
#define CMD_ERROR03                         "ERROR03\r\n"
#define CMD_ERROR04                         "ERROR04\r\n"
#define CMD_ERROR05                         "ERROR05\r\n"
#define CMD_ERROR06                         "ERROR06\r\n"

#define CMD_TEST_TYPE                       0x0D

#define CMD_EXTER_TYPE                      '&'

#define CMD_EXTER_NFC_ID                    "NFCID"
#define CMD_EXTER_NFC_RD                    "NFCRD"

#define CMD_EXTER_RF_TX_ST                  "RFTXS" 
#define CMD_EXTER_RF_TX_ED                  "RFTXE"
#define CMD_EXTER_RF_RX_ST                  "RFRXS"
#define CMD_EXTER_RF_RX_ED                  "RFRXE"
#define CMD_EXTER_RF_CH                     "RFCH"  //*< 有参数
#define CMD_EXTER_RF_PR                     "RFPO"  //*< 有参数

#define CMD_EXTER_BAT                       "BAT"
#define CMD_EXTER_SLEEP                     "SLEEP"


void app_init(void);

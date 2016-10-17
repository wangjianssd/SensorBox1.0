#pragma once
#include "common/lib/data_type_def.h"
#include "stack/custom/mac/core/sync/sync_define.h"
#include "stack/custom/mac/core/asyn/asyn_define.h"

#pragma pack(1)
typedef struct{
    uint8_t ip[4];
    uint8_t gateway_ip[4];
    uint8_t dip[4];
    uint16_t port;
}intra_ip_t;

typedef struct{
    bool_t type;
    union
	{
		supf_spec_t supf_cfg_arg;
		asyn_cfg_t asyn_cfg_arg;
	} parameter;
}exter_mac_t;
#pragma pack()

typedef void (*at_send_cb)(char *pload, uint8_t len);
typedef void (*at_recv_cb)(uint8_t *pload, uint8_t len);
typedef void (*at_interface)(uint8_t *pload, uint8_t len, void *ob);
typedef void (*config_callback)(uint8_t *pload, uint8_t len);
typedef void (*select_callback)(uint8_t *pload, uint8_t *len);


#define CMD_TEST_TYPE                       '\r'
#define CMD_INTRA_TYPE                      'S'     /**< AT内部指令标志位 */
#define CMD_EXTER_TYPE                      '&'     /**< AT外部指令标志位 */

/**< 请求指令 */
#define CMD_EXTER_V_TYPE                    "V"
#define CMD_EXTER_RFPWR_TYPE                "RFPWR"
#define CMD_EXTER_F_TYPE                    "F"
#define CMD_EXTER_RS_TYPE                   "RS"
#define CMD_EXTER_RFO_TYPE                  "RFO"
#define CMD_EXTER_RFS_TYPE                  "RFC"
#define CMD_EXTER_SYNC_TYPE                 "SYNC"
#define CMD_EXTER_ASYN_TYPE                 "ASYN"
#define CMD_EXTER_CH_TYPE                   "RFCH"
#define CMD_EXTER_RFSD_TYPE                 "RFSD"
#define CMD_INTRA_LICENSE_TYPE              "197"
#define CMD_INTRA_DEVICE_TYPE               "198"
#define CMD_INTRA_ID_TYPE                   "199"
#define CMD_INTRA_IP_TYPE                   "200"

/**< 应答指令 */
#define CMD_OK        "\r\nOK\r\n"
#define CMD_SYSSTART  "SYSSTART\r\n"
#define CMD_ERROR00   "ERROR00\r\n"            /**< 未明确意义 */
#define CMD_ERROR01   "ERROR01\r\n"            /**< 指令参数无效 */
#define CMD_ERROR02   "ERROR02\r\n"            /**< 模块数据溢出 */
#define CMD_ERROR03   "ERROR03\r\n"            /**<  */
#define CMD_ERROR04   "ERROR04\r\n"            /**<  */
#define CMD_ERROR05   "ERROR05\r\n"            /**<  */

#define CMD_ERROR06   "no register callback\r\n"            /**< 回调未注册 */

#define AT_SIZE   (80u)



uint8_t ustrtok(char *src, char **des, char *delim);

uint8_t my_strlen(const uint8_t *str);

bool_t ascii_to_hex(uint8_t hi, uint8_t lo, uint8_t *hex);

bool_t hex_to_ascii(uint8_t *buf, uint8_t dat);

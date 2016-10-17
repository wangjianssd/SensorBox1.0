#include "at_mod.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
typedef enum
{
    VER,          /**< Ver */
    RFPWR,      /**< rf power */
    FR,          /**< factory reset */
    RS,         /**< restart */
    RFCH,       /**< ch */
    RFO,        /**< open rf */
    RFS,        /**< shut down rf */
    SYNC,       /**< sync */
    ASYN,       /**< asyn */

    RFSD,       /**< rf send data*/
    S_197,      /**< license */
    S_198,      /**< device type */
    S_199,      /**< id */
    S_200,      /**< IP */
    AT_END,
} at_type_e;

enum
{
    AT_CMD_RECV,
    AT_DATA_RECV,
} at_state_e;
static volatile uint8_t at_state = AT_CMD_RECV;
static volatile uint16_t at_data_length = 0;
#pragma pack(1)
typedef struct
{
    at_interface interface;
    select_callback select;
    config_callback config;
    uint8_t cmd[20];
    uint8_t len;
} at_object_t;
#pragma pack()
static const uint8_t *version[3]  =
{
    "WSN Design: sensor-network\r\n",
    "Hardware Version: 0.1\r\n",
    "Software Version: 0.1\r\n",
};

uint16_t uart_recv_index = 0;
uint8_t uart_recv[AT_SIZE];
char uart_send[AT_SIZE];
at_object_t at_object[AT_END];
at_send_cb at_send;
at_recv_cb at_recv;

static void exter_ver(uint8_t *buf, uint8_t len, void *object)
{
    uint8_t length = 0;
    for(uint8_t i = 0; i < 3; i++)
    {
        length = my_strlen(version[i]);
        at_send((char *)version[i], length);
    }
    at_send(CMD_OK, sizeof(CMD_OK) - 1);
}

static void exter_rfpwr(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    uint8_t length = 0;
    uint8_t temp;
    if(*buf == '=')
    {
        buf++;len--;
        if(ob->config == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        char rfpwr[1];
        rfpwr[0] = *buf;
        temp = atoi(rfpwr);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config(&temp,1);
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select(&temp,&length);
        length = sprintf(uart_send,"%d",temp);
        at_send(uart_send, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else{
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void exter_factory_reset(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    ob->config(NULL,0);
    at_send(CMD_OK, sizeof(CMD_OK) - 1);
}

static void exter_restart(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    at_send(CMD_OK, sizeof(CMD_OK) - 1);
    ob->config(NULL,0);
}

static void exter_rfo(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    uint8_t length = 0;
    uint8_t temp;
    if(*buf == '\r')
    {
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config(NULL,0);
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select(&temp,&length);
        length = sprintf(uart_send,"%d",temp);
        at_send(uart_send, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else{
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void exter_rfs(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    uint8_t length = 0;
    uint8_t temp;
    if(*buf == '\r')
    {
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config(NULL,0);
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select(&temp,&length);
        length = sprintf(uart_send,"%d",temp);
        at_send(uart_send, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else{
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void exter_mac(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    exter_mac_t info;
    osel_memset(&info, 0, sizeof(exter_mac_t));
    char *temp[20];
    osel_memset(temp,0,20);
    uint8_t length = 0;
    if(*buf == '=')
    {
        buf++;len--;
        if(ob->config == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        length = ustrtok((char *)buf,temp,",");
        if(atoi(temp[0]))
        {
            info.type = atoi(temp[0]);
            info.parameter.supf_cfg_arg.beacon_interv_order = atoi(temp[1]);
            info.parameter.supf_cfg_arg.beacon_duration_order = atoi(temp[2]);
            info.parameter.supf_cfg_arg.down_link_slot_edge = atoi(temp[3]);
            info.parameter.supf_cfg_arg.down_link_slot_length = atoi(temp[4]);
            info.parameter.supf_cfg_arg.gts_duration = atoi(temp[5]);
            info.parameter.supf_cfg_arg.intra_channel = atoi(temp[6]);
            info.parameter.supf_cfg_arg.cluster_number = atoi(temp[7]);
            info.parameter.supf_cfg_arg.intra_gts_number = atoi(temp[8]);
            info.parameter.supf_cfg_arg.inter_channel = atoi(temp[9]);
            info.parameter.supf_cfg_arg.inter_unit_number = atoi(temp[10]);
            info.parameter.supf_cfg_arg.intra_cap_number = atoi(temp[11]);

            for(uint8_t i = 0; i < MAX_HOP_NUM; i++)
                info.parameter.supf_cfg_arg.inter_gts_number[i] = atoi(temp[12+i]);

            at_send(CMD_OK, sizeof(CMD_OK) - 1);
            ob->config((uint8_t *)&info,sizeof(exter_mac_t));
        }
        else
        {
            info.type = atoi(temp[0]);
            info.parameter.asyn_cfg_arg.duration = atoi(temp[1]);
            info.parameter.asyn_cfg_arg.auery_duration = atoi(temp[2]);
            info.parameter.asyn_cfg_arg.auery_order = atoi(temp[3]);
            info.parameter.asyn_cfg_arg.asyn_cycle = atoi(temp[4]);
            at_send(CMD_OK, sizeof(CMD_OK) - 1);
            ob->config((uint8_t *)&info,sizeof(asyn_cfg_t)+1);
        }
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select((uint8_t *)&info,&length);
        if(info.type)
        {
            length = sprintf(uart_send,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            info.type,info.parameter.supf_cfg_arg.beacon_interv_order,info.parameter.supf_cfg_arg.beacon_duration_order,
            info.parameter.supf_cfg_arg.down_link_slot_edge,info.parameter.supf_cfg_arg.down_link_slot_length,
            info.parameter.supf_cfg_arg.gts_duration,info.parameter.supf_cfg_arg.intra_channel,info.parameter.supf_cfg_arg.cluster_number,
            info.parameter.supf_cfg_arg.intra_gts_number,info.parameter.supf_cfg_arg.inter_channel,info.parameter.supf_cfg_arg.inter_unit_number,
            info.parameter.supf_cfg_arg.intra_cap_number);
            for(uint8_t i = 0; i < MAX_HOP_NUM; i++)
                length = sprintf(uart_send,"%s,%d",uart_send,info.parameter.supf_cfg_arg.inter_gts_number[i]);
        }
        else
            length = sprintf(uart_send,"%d,%d,%d,%d,%d",
                         info.type,info.parameter.asyn_cfg_arg.duration,info.parameter.asyn_cfg_arg.auery_duration,
                         info.parameter.asyn_cfg_arg.auery_order,info.parameter.asyn_cfg_arg.asyn_cycle);
        at_send(uart_send, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else{
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void exter_rfsd(uint8_t *buf, uint8_t len, void *object)
{
    char num[1] = {0};
    osel_memset(uart_send,0,AT_SIZE);
    if(*buf == '=')
    {
        buf++;
        len--;
        uint16_t length = 0;
        osel_memcpy(num, buf, len);
        length = atoi(num);
        if(length > 64)
            at_send(CMD_ERROR02, sizeof(CMD_ERROR02) - 1);
        at_state = AT_DATA_RECV;
        at_data_length = length;
        length = sprintf(uart_send,"\r\n>\r\n");
        at_send(uart_send, length);

    }
    else if(*buf == '?')
    {
        at_send(CMD_ERROR01, sizeof(CMD_ERROR01) - 1);
    }
    else
    {
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void exter_ch(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    uint8_t ch[16];
    uint8_t length = 0;
    char *temp[16];
    osel_memset(temp,0,16);
    if(*buf == '=')
    {
        buf++;
        len--;
        if(ob->config == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        length = ustrtok((char *)buf, temp, ",");
        for(int i=0; i<length; i++)
        {
            ch[i] = atoi(temp[i]);
        }
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config(ch,length);
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select(ch,&length);
        for (uint8_t i = 0; i < length; i++)
        {
            if(i == 0)
                sprintf(uart_send,"%d",ch[i]);
            else
                sprintf(uart_send,"%s,%d",uart_send,ch[i]);
        }
        at_send(uart_send, length*2);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else
    {
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void intra_ip(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    intra_ip_t info;
    osel_memset(&info, 0, sizeof(intra_ip_t));
    char *temp[8];
    osel_memset(temp,0,8);
    char *temp_info[4];
    osel_memset(temp_info,0,4);
    uint8_t length = 0;
    if(*buf == '=')
    {
        buf++;
        len--;
        if(ob->config == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        length = ustrtok((char *)buf,temp,",");
        if(length == 4)
        {
            length = ustrtok(temp[0],temp_info,".");
            for(int i=0; i<length; i++)
            {
                info.ip[i] = atoi(temp_info[i]);
            }
            length = ustrtok(temp[1],temp_info,".");
            for(int i=0; i<length; i++)
            {
                info.gateway_ip[i] = atoi(temp_info[i]);
            }
            length = ustrtok(temp[2],temp_info,".");
            for(int i=0; i<length; i++)
            {
                info.dip[i] = atoi(temp_info[i]);
            }
            info.port = atoi(temp[3]);
        }
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config((uint8_t *)&info,sizeof(intra_ip_t));
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select((uint8_t *)&info,&length);
        length = sprintf(uart_send,"IP:%d.%d.%d.%d\r\n",info.ip[0],info.ip[1],info.ip[2],info.ip[3]);
        at_send(uart_send, length);
        length = sprintf(uart_send,"GATE_IP:%d.%d.%d.%d\r\n",info.gateway_ip[0],info.gateway_ip[1],info.gateway_ip[2],info.gateway_ip[3]);
        at_send(uart_send, length);
        length = sprintf(uart_send,"DIP:%d.%d.%d.%d\r\n",info.dip[0],info.dip[1],info.dip[2],info.dip[3]);
        at_send(uart_send, length);
        length = sprintf(uart_send,"PORT:%d",info.port);
        at_send(uart_send, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else
    {
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void intra_license(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    uint8_t license[32];
    uint8_t length = 0;
    if(*buf == '=')
    {
        buf++;
        len--;
        if(ob->config == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        length = len;
        osel_memcpy(license, buf, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config(license,length);
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select(license,&length);
        osel_memcpy(uart_send, license, length);
        at_send(uart_send, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else
    {
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void intra_device(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    uint8_t temp;
    uint8_t length = 0;
    if(*buf == '=')
    {
        buf++;
        len--;
        if(ob->config == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        char type[1];
        type[0] = *buf;
        temp = atoi(type);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config(&temp,1);
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select(&temp,&length);
        length = sprintf(uart_send,"%d",temp);
        at_send(uart_send, length);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else
    {
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void intra_id(uint8_t *buf, uint8_t len, void *object)
{
    at_object_t *ob = (at_object_t*)object;
    osel_memset(uart_send,0,AT_SIZE);
    uint8_t id[32];
    uint8_t length = 0;
    if(*buf == '=')
    {
        buf++;
        len--;
        if(ob->config == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        for (uint8_t i = 0; i < len; i += 2)
        {
            if (!ascii_to_hex(buf[i], buf[i+1],
                              &id[i>>1]))
            {
                at_send(CMD_ERROR02, sizeof(CMD_ERROR02) - 1);
                return;
            }
        }
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        ob->config(id,(len>>1));
    }
    else if(*buf == '?')
    {
        if(ob->select == NULL)
        {
            at_send(CMD_ERROR06, sizeof(CMD_ERROR06) - 1);
            return;
        }
        ob->select(id,&length);
        for (uint8_t i = 0; i < length; i++)
        {
            hex_to_ascii((uint8_t *)&uart_send[i * 2],
                         id[length - 1 - i]);
        }
        at_send(uart_send, length * 2);
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
    }
    else
    {
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
    }
}

static void at_interface_guide(uint8_t *buf, uint8_t len)
{
    bool_t mark = FALSE;
    for(uint16_t i=0; i<AT_END ; i++)
    {
        if(at_object[i].interface != NULL)
        {
            if(osel_memcmp(buf, at_object[i].cmd, at_object[i].len))
            {
                mark = TRUE;
                at_object[i].interface(buf + at_object[i].len, len - at_object[i].len, &at_object[i]);
            }
        }
    }
    if(!mark)
        at_send(CMD_ERROR01, sizeof(CMD_ERROR01) - 1);
}

void ver_cmd_register(config_callback config, select_callback select)
{
    at_object[VER].config = config;
    at_object[VER].select = select;
    at_object[VER].interface = exter_ver;
    at_object[VER].len = sizeof(CMD_EXTER_V_TYPE)-1;
    osel_memcpy(at_object[VER].cmd, CMD_EXTER_V_TYPE, at_object[VER].len);
}

void rfpwr_cmd_register(config_callback config, select_callback select)
{
    at_object[RFPWR].config = config;
    at_object[RFPWR].select = select;
    at_object[RFPWR].interface = exter_rfpwr;
    at_object[RFPWR].len = sizeof(CMD_EXTER_RFPWR_TYPE)-1;
    osel_memcpy(at_object[RFPWR].cmd, CMD_EXTER_RFPWR_TYPE, at_object[RFPWR].len);
}

void factory_reset_cmd_register(config_callback config, select_callback select)
{
    at_object[FR].config = config;
    at_object[FR].select = select;
    at_object[FR].interface = exter_factory_reset;
    at_object[FR].len = sizeof(CMD_EXTER_F_TYPE)-1;
    osel_memcpy(at_object[FR].cmd, CMD_EXTER_F_TYPE, at_object[FR].len);
}

void restart_cmd_register(config_callback config, select_callback select)
{
    at_object[RS].config = config;
    at_object[RS].select = select;
    at_object[RS].interface = exter_restart;
    at_object[RS].len = sizeof(CMD_EXTER_RS_TYPE)-1;
    osel_memcpy(at_object[RS].cmd, CMD_EXTER_RS_TYPE, at_object[RS].len);
}

void rfo_cmd_register(config_callback config, select_callback select)
{
    at_object[RFO].config = config;
    at_object[RFO].select = select;
    at_object[RFO].interface = exter_rfo;
    at_object[RFO].len = sizeof(CMD_EXTER_RFO_TYPE)-1;
    osel_memcpy(at_object[RFO].cmd, CMD_EXTER_RFO_TYPE, at_object[RFO].len);
}

void rfs_cmd_register(config_callback config, select_callback select)
{
    at_object[RFS].config = config;
    at_object[RFS].select = select;
    at_object[RFS].interface = exter_rfs;
    at_object[RFS].len = sizeof(CMD_EXTER_RFS_TYPE)-1;
    osel_memcpy(at_object[RFS].cmd, CMD_EXTER_RFS_TYPE, at_object[RFS].len);
}

void sync_cmd_register(config_callback config, select_callback select)
{
    at_object[SYNC].config = config;
    at_object[SYNC].select = select;
    at_object[SYNC].interface = exter_mac;
    at_object[SYNC].len = sizeof(CMD_EXTER_SYNC_TYPE)-1;
    osel_memcpy(at_object[SYNC].cmd, CMD_EXTER_SYNC_TYPE, at_object[SYNC].len);
}

void asyn_cmd_register(config_callback config, select_callback select)
{
    at_object[ASYN].config = config;
    at_object[ASYN].select = select;
    at_object[ASYN].interface = exter_mac;
    at_object[ASYN].len = sizeof(CMD_EXTER_ASYN_TYPE)-1;
    osel_memcpy(at_object[ASYN].cmd, CMD_EXTER_ASYN_TYPE, at_object[ASYN].len);
}

void rfsd_cmd_register(config_callback config, select_callback select)
{
    at_object[RFSD].config = config;
    at_object[RFSD].select = select;
    at_object[RFSD].interface = exter_rfsd;
    at_object[RFSD].len = sizeof(CMD_EXTER_RFSD_TYPE)-1;
    osel_memcpy(at_object[RFSD].cmd, CMD_EXTER_RFSD_TYPE, at_object[RFSD].len);
}

void ch_cmd_register(config_callback config, select_callback select)
{
    at_object[RFCH].config = config;
    at_object[RFCH].select = select;
    at_object[RFCH].interface = exter_ch;
    at_object[RFCH].len = sizeof(CMD_EXTER_CH_TYPE)-1;
    osel_memcpy(at_object[RFCH].cmd, CMD_EXTER_CH_TYPE, at_object[RFCH].len);
}

void license_cmd_register(config_callback config, select_callback select)
{
    at_object[S_197].config = config;
    at_object[S_197].select = select;
    at_object[S_197].interface = intra_license;
    at_object[S_197].len = sizeof(CMD_INTRA_LICENSE_TYPE)-1;
    osel_memcpy(at_object[S_197].cmd, CMD_INTRA_LICENSE_TYPE, at_object[S_197].len);
}

void device_cmd_register(config_callback config, select_callback select)
{
    at_object[S_198].config = config;
    at_object[S_198].select = select;
    at_object[S_198].interface = intra_device;
    at_object[S_198].len = sizeof(CMD_INTRA_DEVICE_TYPE)-1;
    osel_memcpy(at_object[S_198].cmd, CMD_INTRA_DEVICE_TYPE, at_object[S_198].len);
}

void id_cmd_register(config_callback config, select_callback select)
{
    at_object[S_199].config = config;
    at_object[S_199].select = select;
    at_object[S_199].interface = intra_id;
    at_object[S_199].len = sizeof(CMD_INTRA_ID_TYPE)-1;
    osel_memcpy(at_object[S_199].cmd, CMD_INTRA_ID_TYPE, at_object[S_199].len);
}

void ip_cmd_register(config_callback config, select_callback select)
{
    at_object[S_200].config = config;
    at_object[S_200].select = select;
    at_object[S_200].interface = intra_ip;
    at_object[S_200].len = sizeof(CMD_INTRA_IP_TYPE)-1;
    osel_memcpy(at_object[S_200].cmd, CMD_INTRA_IP_TYPE, at_object[S_200].len);
}

void at_mod_parse(uint8_t *buf, uint8_t len)
{
    switch (*buf)
    {
    case CMD_INTRA_TYPE:
    case CMD_EXTER_TYPE:
        at_interface_guide(buf + 1, len - 1);
        break;
    case CMD_TEST_TYPE:
        at_send(CMD_OK, sizeof(CMD_OK) - 1);
        break;
    default:
        at_send(CMD_ERROR00, sizeof(CMD_ERROR00) - 1);
        break;
    }
}

void at_mod_init(at_send_cb send_cb, at_recv_cb recv_cb)
{
    osel_memset(at_object,0,AT_END);
    at_send = send_cb;
    at_recv = recv_cb;
    at_send(CMD_SYSSTART, sizeof(CMD_SYSSTART) - 1);
    osel_memset(uart_recv,0,AT_SIZE);
}

void at_uart_recv(uint8_t ch)
{
    if(uart_recv_index == AT_SIZE)
        uart_recv_index = 0;
    if(uart_recv_index == 0)
        osel_memset(uart_recv,0,AT_SIZE);
    uart_recv[uart_recv_index] = ch;

    switch(at_state)
    {
    case AT_CMD_RECV:
    {
        if(uart_recv_index == 0 && ch == 'A')
        {
            uart_recv_index++;
        }
        else if(uart_recv_index == 1 && ch == 'T')
        {
            uart_recv_index++;
        }
        else if(ch == '\r')
        {
            at_mod_parse(&uart_recv[2], uart_recv_index-2);   /**< todo:后期用消息替换 */
            uart_recv_index = 0;
        }
        else if(uart_recv_index >= 2)
        {
            uart_recv_index++;
        }
        else
        {
            uart_recv_index = 0;
        }
        break;
    }

    case AT_DATA_RECV:
    {
        uart_recv_index++;
        if(uart_recv_index == at_data_length)
        {
            uart_recv_index = 0;
            at_state = AT_CMD_RECV;
            if(at_recv!=NULL)
                at_recv(uart_recv, at_data_length);
        }
        break;
    }
    default:
        break;
    }
}


#pragma vector = USCI_A0_VECTOR
__interrupt void uart0_rx_isr(void)
{
    OSEL_ISR_ENTRY();
    at_uart_recv(UCA0RXBUF);
    OSEL_ISR_EXIT();
    LPM3_EXIT;
}

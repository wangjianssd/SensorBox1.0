#include "general.h"

uint8_t license[22]= {0x01,0x02,0xff};

uint16_t mac_short_addr_get(uint64_t mac_addr)
{
    uint16_t addr = mac_addr;
    return addr;
}

uint8_t get_addr(pbuf_t *pbuf, mac_addr_mode_e mode, uint64_t *addr)
{
    if(mode == MAC_MHR_ADDR_MODE_SHORT)
    {
        memcpy(addr, pbuf->data_p, MAC_ADDR_SHORT_SIZE);
        pbuf->data_p += MAC_ADDR_SHORT_SIZE;
        return MAC_ADDR_SHORT_SIZE;
    }
    else if(mode == MAC_MHR_ADDR_MODE_LONG)
    {
        memcpy(addr, pbuf->data_p, MAC_ADDR_LONG_SIZE);
        pbuf->data_p += MAC_ADDR_LONG_SIZE;
        return MAC_ADDR_LONG_SIZE;
    }
    else
    {
    }
    return 0;
}
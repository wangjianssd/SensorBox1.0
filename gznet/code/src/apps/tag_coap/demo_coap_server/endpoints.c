#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "common/hal/hal.h"
#include "common/coap/coap_server.h"
#include "stack/custom/mac/mac.h"
#include <node_cfg.h>

uint8_t route_led[] = "led";
coap_endpoint_path_t path_led;

void endpoint_setup(void)
{
    uint64_t nui = mac_self_saddr();
    // LED控制路由
    path_led.count = 1;
    memcpy(path_led.route[0].name, route_led, 3);
    path_led.route[0].len = 3;
}

// LED状态，0x00代表LED关闭，0x01代表LED打开
uint8_t led = 0x00;
int handle_get_led(coap_rw_buffer_t *rw_buf,
                      const coap_packet_t *inpkt, coap_packet_t *outpkt,
                      uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(rw_buf, inpkt, outpkt,
                             (const uint8_t *)&led, 1,    // 负载内容和长度
                             id_hi, id_lo,                  // Message ID
                             &inpkt->tok,                   // CoAP ToKen
                             COAP_RSPCODE_CONTENT,          // 2.05 Content
                             COAP_CONTENTTYPE_OCTECT_STREAM);  // 负载类型文本格式
}

int handle_put_led(coap_rw_buffer_t *rw_buf,
                     const coap_packet_t *inpkt, coap_packet_t *outpkt,
                     uint8_t id_hi, uint8_t id_lo)
{
    if (inpkt->payload.len == 0)
    {
        return coap_make_response(rw_buf, inpkt, outpkt,
                                  NULL, 0,
                                  id_hi, id_lo,
                                  &inpkt->tok,
                                  COAP_RSPCODE_BAD_REQUEST,
                                  COAP_CONTENTTYPE_OCTECT_STREAM);
    }

    if (inpkt->payload.p[0] == 0x01)
    {
        led = 0x01;
        hal_led_open(GREEN);
        return coap_make_response(rw_buf, inpkt, outpkt,
                                 (const uint8_t *)&led, 1,
                                 id_hi, id_lo, &inpkt->tok,
                                 COAP_RSPCODE_CHANGED,
                                 COAP_CONTENTTYPE_OCTECT_STREAM);
    }
    else
    {
        led = 0x00;
        hal_led_close(GREEN);
        return coap_make_response(rw_buf, inpkt, outpkt,
                                 (const uint8_t *)&led, 1,
                                 id_hi, id_lo, &inpkt->tok,
                                 COAP_RSPCODE_CHANGED,
                                 COAP_CONTENTTYPE_OCTECT_STREAM);
    }
}

const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_PUT, handle_put_led, &path_led, "ct=42"},
    {COAP_METHOD_GET, handle_get_led, &path_led, "ct=42"},
    {(coap_method_t)0, NULL, NULL, NULL}
};
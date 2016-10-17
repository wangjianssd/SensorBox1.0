#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "coap_server.h"

/**
 * @breaf CoAP Frame.
 
    0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |Ver| T |  TKL  |      Code     |           Message ID          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Token (if any, TKL bytes) ...
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Options (if any) ...
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |1 1 1 1 1 1 1 1|   Payload (if any) ...
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
 
extern void endpoint_setup(void);   
extern const coap_endpoint_t endpoints[];

#ifdef DEBUG
/**
 * @brief 16进制方式打印coap首部结构体
 * @param  [in] coap_header_t *hdr coap首部结构体
 * @return 无
 * @note 不需要进行单元测试
 */
void coap_dump_header(coap_header_t hdr)
{
    printf("Header:\n");
    printf("  ver  0x%02X\n", hdr.ver);
    printf("  t    0x%02X\n", hdr.t);
    printf("  tkl  0x%02X\n", hdr.tkl);
    printf("  code 0x%02X\n", hdr.code);
    printf("  id   0x%02X%02X\n", hdr.id[0], hdr.id[1]);
}
#endif

#ifdef DEBUG
/**
 * @brief 16进制方式打印coap请求或者响应
 * @param  [in] buf coap请求或者响应缓冲区
 * @param  [in] buf_len coap请求或者响应缓冲区长度
 * @param  [in] bare
 * @return 无
 */
void coap_dump(const uint8_t *buf, size_t buf_len)
{
	while (buf_len--)
	{
		printf("%02X%s", *buf++, (buf_len > 0) ? " " : "");
	}
}
#endif

/**
 * @brief coap首部解析
 * @param  [out] hdr CoAP首部
 * @param  [in] buf CoAP请求或者响应缓冲区
 * @param  [in] buf_len CoAP请求或者响应缓冲区长度
 * @return 无
 * @note 可不进行单元测试
 */
int coap_parse_header(coap_header_t *hdr, const uint8_t *buf, size_t buf_len)
{
    if (buf_len < 4)
        return COAP_ERR_HEADER_TOO_SHORT;
    
    hdr->ver = (buf[0] & 0xC0) >> 6;
    if (hdr->ver != 1)
        return COAP_ERR_VERSION_NOT_1;
    
    hdr->t = (buf[0] & 0x30) >> 4;
    
    hdr->tkl = buf[0] & 0x0F;
    
    hdr->code = buf[1];
    
    hdr->id[0] = buf[2];
    hdr->id[1] = buf[3];
    
    return 0;
}

/**
 * @brief coap token部分解析
 * @param  [out] tokbuf Token区域
 * @param  [in] hdr CoAP首部，从首部中获得Token长度信息
 * @param  [in] buf CoAP请求或者响应缓冲区
 * @param  [in] buf_len CoAP请求或者响应缓冲区长度
 * @return 错误码
 * @note 可不进行单元测试
 */
int coap_parse_token(coap_buffer_t *tokbuf, const coap_header_t *hdr, const uint8_t *buf, size_t buf_len)
{
    if (hdr->tkl == 0)
    {
        tokbuf->p = NULL;
        tokbuf->len = 0;
        return 0;
    }
    else if (hdr->tkl <= 8)
    {
        if (4U + hdr->tkl > buf_len)
            return COAP_ERR_TOKEN_TOO_SHORT;    // tok bigger than packet
        tokbuf->p = buf + 4;                    // past header
        tokbuf->len = hdr->tkl;
        return 0;
    }
    else
    {
        return COAP_ERR_TOKEN_TOO_SHORT;
    }
}

/**
 * @brief coap token部分解析
 * @param  [out] option Option具体内容
 * @param  [in] running_delta 上一个Option的Num值
 * @param  [in] buf 该段Option的起始位置
 * @param  [in] buf_len ??
 * @return 错误码
 * @note 可不进行单元测试，比较复杂，可尝试
 */
int coap_parse_option(coap_option_t *option, uint16_t *running_delta, const uint8_t **buf, size_t buf_len)
{
    const uint8_t *p = *buf;
    uint8_t headlen = 1;
    uint16_t len, delta;

    if (buf_len < headlen) // too small
        return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;

    delta = (p[0] & 0xF0) >> 4;
    len = p[0] & 0x0F;

    // These are untested and may be buggy
    if (delta == 13)
    {
        headlen++;
        if (buf_len < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        delta = p[1] + 13;
        p++;
    }
    else if (delta == 14)
    {
        headlen += 2;
        if (buf_len < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        delta = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    }
    else if (delta == 15)
        return COAP_ERR_OPTION_DELTA_INVALID;

    if (len == 13)
    {
        headlen++;
        if (buf_len < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        len = p[1] + 13;
        p++;
    }
    else if (len == 14)
    {
        headlen += 2;
        if (buf_len < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        len = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    }
    else if (len == 15)
    {
        return COAP_ERR_OPTION_LEN_INVALID;
    }
        
    if ((p + 1 + len) > (*buf + buf_len))
        return COAP_ERR_OPTION_TOO_BIG;

    option->num = delta + *running_delta;
    option->buf.p = p+1;
    option->buf.len = len;

    *buf = p + 1 + len;
    *running_delta += delta;

    return 0;
}

/**
 * @brief 获得所有Option和payload
 * @param  [out] options 所有Options内容
 * @param  [out] num_option Option数量
 * @param  [out] payload 负载
 * @param  [out] hdr 首部
 * @param  [in] buf CoAP请求或者响应缓冲区
 * @param  [in] buf_len CoAP请求或者响应缓冲区长度
 * @return 错误码
 * @note 可不进行单元测试，比较复杂，可尝试
 */
int coap_parse_options_and_payload(coap_option_t *options, uint8_t *num_option, 
                                   coap_buffer_t *payload, const coap_header_t *hdr, 
                                   const uint8_t *buf, size_t buf_len)
{
    size_t option_index = 0;
    uint16_t delta = 0;
    const uint8_t *p = buf + 4 + hdr->tkl;
    const uint8_t *end = buf + buf_len;
    int rc;
    
    if (p > end)
        return COAP_ERR_OPTION_OVERRUNS_PACKET;   // out of bounds

    // 0xFF is payload marker
    while((option_index < *num_option) && (p < end) && (*p != 0xFF))
    {
        if (0 != (rc = coap_parse_option(&options[option_index], &delta, &p, end-p)))
            return rc;
        option_index++;
    }
    *num_option = option_index;

    if (p + 1 < end && *p == 0xFF)  // payload marker
    {
        payload->p = p + 1;
        payload->len = end - (p + 1);
    }
    else
    {
        payload->p = NULL;
        payload->len = 0;
    }

    return 0;
}

#ifdef DEBUG
void coap_dump_options(coap_option_t *opts, size_t numopt)
{
    size_t i;
    printf(" Options:\n");
    for (i = 0; i<numopt; i++)
    {
        printf("  0x%02X [ ", opts[i].num);
        coap_dump(opts[i].buf.p, opts[i].buf.len);
        printf(" ]\n");
    }
}
#endif

#ifdef DEBUG
/**
 * @brief 16进制方式打印coap首部结构体
 * @param  [in] pkt CoAP报文结构体，包含CoAP所有部分
 * @return 无
 * @note 不需要进行单元测试
 */
void coap_dump_packet(coap_packet_t pkt)
{
    // 打印CoAP首部
    coap_dump_header(pkt.hdr);
    // 打印CoAP选项
    coap_dump_options(pkt.opts, pkt.numopts);
    
    printf("Payload: ");
        coap_dump(pkt.payload.p, pkt.payload.len);
    printf("\n");
}
#endif

/**
 * @brief 二进制内容转化为CoAP报文结构体
 * @param  [out] pkt CoAP报文结构体
 * @param  [in] buf 二进制内容首地址
 * @param  [in] buf_len 二进制内容长度
 * @return 错误码
 * @note 非常重要的函数
 */
int coap_parse(coap_packet_t *pkt, const uint8_t *buf, size_t buf_len)
{
    int rc;

    if (0 != (rc = coap_parse_header(&pkt->hdr, buf, buf_len)))
        return rc;

    if (0 != (rc = coap_parse_token(&pkt->tok, &pkt->hdr, buf, buf_len)))
        return rc;

    pkt->numopts = MAXOPT;
    if (0 != (rc = coap_parse_options_and_payload(pkt->opts, &(pkt->numopts), &(pkt->payload), &pkt->hdr, buf, buf_len)))
        return rc;

    return 0;
}

/**
 * @brief 从CoAP Option区域中找出合适
 * @param  [in] pkt CoAP报文结构体
 * @param  [in] num 期望需要到的Option 编号
 * @param  [out] count 期望Option个数
 * @return 指向期望Option的首地址
 * @note 非常重要的函数
 */
const coap_option_t *coap_find_options(const coap_packet_t *pkt, uint8_t num, uint8_t *count)
{
    size_t i;
    const coap_option_t *first = NULL;
    *count = 0;
    for (i = 0; i < pkt->numopts; i++)
    {
        if (pkt->opts[i].num == num)
        {
            if (NULL == first)
                first = &pkt->opts[i];
            (*count)++;
        }
        else
        {
            if (NULL != first)
                break;
        }
    }
    return first;
}

/**
 * @brief coap_buffer_t 复制到 字符串缓冲区中
 * @param  [out] str_buf 字符串缓冲区首地址
 * @param  [in] str_len 字符串缓冲区长度
 * @param  [in] buf coap缓冲区
 * @return 错误码
 * @note 打印option内容比较有用
 */
int coap_buffer_to_string(char *str_buf, size_t str_len, const coap_buffer_t *buf)
{
    if (buf->len + 1 > str_len)
        return COAP_ERR_BUFFER_TOO_SMALL;

    memcpy(str_buf, buf->p, buf->len);
    str_buf[buf->len] = 0;
    return 0;
}

/**
 * @brief 把CoAP结构体转化为二进制内容
 * @param  [out] buf 二进制缓冲区
 * @param  [out] buf_len 二进制缓冲区长度（输入时为最大长度，输出时为实际长度）
 * @param  [in] pkt 将被序列化的CoAP结构体
 * @return 错误码
 * @note 非常重要的函数
 */
int coap_build(uint8_t *buf, size_t *buf_len, const coap_packet_t *pkt)
{
    size_t opts_len = 0;
    size_t i;
    uint8_t *p;
    uint16_t running_delta = 0;

    if (*buf_len < (4U + pkt->hdr.tkl))
        return COAP_ERR_BUFFER_TOO_SMALL;

    // 组装CoAP首部
    buf[0] = (pkt->hdr.ver & 0x03) << 6;
    buf[0] |= (pkt->hdr.t & 0x03) << 4;
    buf[0] |= (pkt->hdr.tkl & 0x0F);
    buf[1] = pkt->hdr.code;
    buf[2] = pkt->hdr.id[0];
    buf[3] = pkt->hdr.id[1];

    // 组装CoAP Token部分
    p = buf + 4;
    if ((pkt->hdr.tkl > 0) && (pkt->hdr.tkl != pkt->tok.len))
        return COAP_ERR_UNSUPPORTED;
    
    if (pkt->hdr.tkl > 0)
        memcpy(p, pkt->tok.p, pkt->hdr.tkl);

    p += pkt->hdr.tkl;

    // 组装CoAP选项
    for (i = 0; i < pkt->numopts; i++)
    {
        uint32_t optDelta;
        uint8_t len, delta = 0;

        if (((size_t)(p-buf)) > *buf_len)
             return COAP_ERR_BUFFER_TOO_SMALL;

        optDelta = pkt->opts[i].num - running_delta;
        coap_option_nibble(optDelta, &delta);
        coap_option_nibble((uint32_t)pkt->opts[i].buf.len, &len);

        *p++ = (0xFF & (delta << 4 | len));
        if (delta == 13)
        {
            *p++ = (optDelta - 13);
        }
        else if (delta == 14)
        {
            *p++ = ((optDelta-269) >> 8);
            *p++ = (0xFF & (optDelta-269));
        }
        if (len == 13)
        {
            *p++ = (pkt->opts[i].buf.len - 13);
        }
        else if (len == 14)
  	    {
            *p++ = (pkt->opts[i].buf.len >> 8);
            *p++ = (0xFF & (pkt->opts[i].buf.len-269));
        }

        memcpy(p, pkt->opts[i].buf.p, pkt->opts[i].buf.len);
        p += pkt->opts[i].buf.len;
        running_delta = pkt->opts[i].num;
    }

    opts_len = (p - buf) - 4;   // number of bytes used by options

    if (pkt->payload.len > 0)
    {
        if (*buf_len < 4 + 1 + pkt->payload.len + opts_len)
            return COAP_ERR_BUFFER_TOO_SMALL;
        buf[4 + opts_len] = 0xFF;   // CoAP首部和CoAP负载分隔符
        // CoAP负载拷贝到缓冲区中
        memcpy(buf + 5 + opts_len, pkt->payload.p, pkt->payload.len);
        *buf_len = opts_len + 5 + pkt->payload.len;
    }
    else
    {
        *buf_len = opts_len + 4;
    }

    return 0;
}

/**
 * @brief
 * @note 没搞懂
 */
void coap_option_nibble(uint32_t value, uint8_t *nibble)
{
    if (value < 13)
    {
        *nibble = (0xFF & value);
    }
    else if (value <= 0xFF + 13)
    {
        *nibble = 13;
    }
    else if (value <= 0xFFFF + 269)
    {
        *nibble = 14;
    }
}

/**
 * @brief 组装一个CoAP完整结构体
 * @param  [out] scratch 临时区域，此处用于装载Option
 * @param  [out] pkt 二进制内容首地址
 * @param  [in] content CoAP 负载
 * @param  [in] content_len CoAP 负载长度
 * @param  [in] msgid_hi message id高字节
 * @param  [in] msgid_hi message id高字节
 * @param  [in] rspcode 响应码
 * @param  [in] content_type 负载媒体格式
 * @return 错误码
 * @note 待补充
 */
int coap_make_response(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *pkt,
                        const uint8_t *content, size_t content_len, 
                        uint8_t msgid_hi, uint8_t msgid_lo, 
                        const coap_buffer_t* tok, 
                        coap_responsecode_t rspcode, 
                        coap_content_type_t content_type)
{
    // CoAP首部四字节操作
    pkt->hdr.ver = 0x01;
    pkt->hdr.t = COAP_TYPE_ACK;
    pkt->hdr.tkl = 0;
    pkt->hdr.code = rspcode;
    pkt->hdr.id[0] = msgid_hi;
    pkt->hdr.id[1] = msgid_lo;

    // CoAP Token区域操作
    if (tok)
    {
        pkt->hdr.tkl = tok->len;
        pkt->tok = *tok;
    }

    // 从输入pkt中查找URI Path
    const coap_option_t *opt;
    uint8_t counter = 0;
    opt = coap_find_options(inpkt, COAP_OPTION_URI_PATH, &counter);

    // CoAP Option区域操作
    pkt->numopts = counter + 1;

    // 把URI Path加入到响应pkt中
    for (int i = 0; i < counter; i++)
    {
        pkt->opts[i].num = COAP_OPTION_URI_PATH;
        pkt->opts[i].buf.p = opt[i].buf.p;
        pkt->opts[i].buf.len = opt[i].buf.len;
    }

    scratch->p[0] = (uint8_t)content_type;						// content_type 修改为一个字节
    pkt->opts[counter].num = COAP_OPTION_CONTENT_FORMAT;
    pkt->opts[counter].buf.p = scratch->p;                      // 临时区域scratch已经装载Option，直接复制给第一个Option
    if (scratch->len < 2)
        return COAP_ERR_BUFFER_TOO_SMALL;
    pkt->opts[counter].buf.len = 1;							    // content_type 修改为一个字节

    // CoAP 负载区域操作
    pkt->payload.p = content;
    pkt->payload.len = content_len;

    return 0;
}

/**
 * @brief 处理CoAP请求
 * @param  [out] scratch 临时区域，此处用于装载Option
 * @param  [in] inpkt CoAP完整结构体 CoAP请求（输入）
 * @param  [out] outpkt CoAP完整结构题 CoAP响应（输出）
 * @return 总是return 0
 * @note
 */
int coap_handle_req(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt)
{
    const coap_option_t *opt;
    uint8_t count;
    int i;
    const coap_endpoint_t *ep = endpoints;      // endpoints为全局变量，单元测试需要提前组装

    while(NULL != ep->handler)
    {
        // 第一步，匹配
        if (ep->method != inpkt->hdr.code)
            goto next;

        // 第二步，从option中找出 URI Path，需要获得两个要素，URI的个数和具体内容
        if (NULL != (opt = coap_find_options(inpkt, COAP_OPTION_URI_PATH, &count)))
        {
            // 比较URI个数，URI一般为1个或者2个
            if (count != ep->path->count)
                goto next;

            // 组个比较URI的长度和具体内容
            for (i = 0; i < count; i++)
            {
                //if (opt[i].buf.len != strlen(ep->path->elems[i]))
                if (opt[i].buf.len != ep->path->route[i].len)
                    goto next;

                // if (0 != memcmp(ep->path->elems[i], opt[i].buf.p, opt[i].buf.len))
                if (0 != memcmp(ep->path->route[i].name, opt[i].buf.p, opt[i].buf.len))
                    goto next;
            }

            // 匹配成功，立刻运行回调函数
            return ep->handler(scratch, inpkt, outpkt,
                               inpkt->hdr.id[0], inpkt->hdr.id[1]);
        }
next:
        ep++;
    }

    // 匹配不成功，返回404
    coap_make_response(scratch, inpkt, outpkt, NULL, 0,
                       inpkt->hdr.id[0], inpkt->hdr.id[1],
                       &inpkt->tok,
                       COAP_RSPCODE_NOT_FOUND,
                       COAP_CONTENTTYPE_NONE);

    return 0;
}

void coap_setup(void)
{

}


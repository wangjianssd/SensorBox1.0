#ifndef __COAP_H
#define __COAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include "sys_arch/osel_arch.h"
	
/**< Option最大个数 */
#define MAXOPT 16

/**
 * @brief CoAP首部结构体
 */
typedef struct
{
    uint8_t ver;                /**< CoAP版本编号 */
    uint8_t t;                  /**< CoAP 报文类型 */
    uint8_t tkl;                /**< CoAP Token区域长度 */
    uint8_t code;               /**< CoAP 功能码，请求和响应有所不同 */
    uint8_t id[2];              /**< CoAP 报文编号 2字节 */
} coap_header_t;

/**
 * @brief CoAP首部加负载二进制缓冲区
 */
typedef struct
{
    const uint8_t *p;           /**< 指向缓冲区的首地址指针 */
    size_t len;                 /**< 缓冲区长度 */
} coap_buffer_t;

/**
 * @brief CoAP首部加负载二进制缓冲区，一般用于输出，可修改
 */
typedef struct
{
    uint8_t *p;                 /**< 指向缓冲区的首地址指针 */
    size_t len;                 /**< 缓冲区长度 */
} coap_rw_buffer_t;

/**
 * @brief CoAP Option结构体，单个
 */
typedef struct
{
    uint8_t num;                /**< CoAP Option Num */ 
    coap_buffer_t buf;          /**< CoAP Option Value */ 
} coap_option_t;

/**
 * @brief CoAP请求或响应结构题
 */
typedef struct
{
    coap_header_t hdr;          /**< CoAP首部 */
    coap_buffer_t tok;          /**< CoAP Token区域 */
    uint8_t numopts;            /**< CoAP Option个数 */
    coap_option_t opts[MAXOPT]; /**< CoAP Option具体内容，数组结构存放 */
    coap_buffer_t payload;      /**< CoAP 负载 */
} coap_packet_t;


/**
 * @brief CoAP Option 编号枚举体
 */
typedef enum
{
    COAP_OPTION_IF_MATCH = 1,
    COAP_OPTION_URI_HOST = 3,
    COAP_OPTION_ETAG = 4,
    COAP_OPTION_IF_NONE_MATCH = 5,
    COAP_OPTION_OBSERVE = 6,
    COAP_OPTION_URI_PORT = 7,
    COAP_OPTION_LOCATION_PATH = 8,
    COAP_OPTION_URI_PATH = 11,
    COAP_OPTION_CONTENT_FORMAT = 12,
    COAP_OPTION_MAX_AGE = 14,
    COAP_OPTION_URI_QUERY = 15,
    COAP_OPTION_ACCEPT = 17,
    COAP_OPTION_LOCATION_QUERY = 20,
    COAP_OPTION_PROXY_URI = 35,
    COAP_OPTION_PROXY_SCHEME = 39
} coap_option_num_t;

/**
 * @brief CoAP Method
 */
typedef enum
{
    COAP_METHOD_GET = 1,
    COAP_METHOD_POST = 2,
    COAP_METHOD_PUT = 3,
    COAP_METHOD_DELETE = 4
} coap_method_t;

/**
 * @brief CoAP 报文类型枚举体
 */
typedef enum
{
    COAP_TYPE_CON = 0,
    COAP_TYPE_NONCON = 1,
    COAP_TYPE_ACK = 2,
    COAP_TYPE_RESET = 3
} coap_msgtype_t;

#define MAKE_RSPCODE(clas, det) ((clas << 5) | (det))
/**
 * @brief CoAP 响应码
 */
typedef enum
{
    COAP_RSPCODE_CONTENT = MAKE_RSPCODE(2, 5),      /**< 成功响应 */
    COAP_RSPCODE_NOT_FOUND = MAKE_RSPCODE(4, 4),    /**< 资源没有被发现 */
    COAP_RSPCODE_BAD_REQUEST = MAKE_RSPCODE(4, 0),  /**< 错误请求 */
    COAP_RSPCODE_CHANGED = MAKE_RSPCODE(2, 4)       /**< 资源被改变 */
} coap_responsecode_t;

/**
 * @brief CoAP 负载内容媒体类型枚举体
 */
typedef enum
{
    COAP_CONTENTTYPE_NONE = -1, 
    COAP_CONTENTTYPE_TEXT_PLAIN = 0,                /**< 文本类型 */
    COAP_CONTENTTYPE_OCTECT_STREAM = 42,            /**< 二进制类型 */
    COAP_CONTENTTYPE_APPLICATION_JSON = 50,         /**< JSON类型 */
    COAP_CONTENTTYPE_APPLICATION_LINKFORMAT = 40,   /**< 连接格式 */
    COAP_CONTENTTYPE_APPLICATION_CBOR = 60,         /**< 二进制JSON格式 */
} coap_content_type_t;

/**
 * @brief 错误码
 */
typedef enum
{
    COAP_ERR_NONE = 0,
    COAP_ERR_HEADER_TOO_SHORT = 1,
    COAP_ERR_VERSION_NOT_1 = 2,
    COAP_ERR_TOKEN_TOO_SHORT = 3,
    COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER = 4,
    COAP_ERR_OPTION_TOO_SHORT = 5,
    COAP_ERR_OPTION_OVERRUNS_PACKET = 6,
    COAP_ERR_OPTION_TOO_BIG = 7,
    COAP_ERR_OPTION_LEN_INVALID = 8,
    COAP_ERR_BUFFER_TOO_SMALL = 9,
    COAP_ERR_UNSUPPORTED = 10,
    COAP_ERR_OPTION_DELTA_INVALID = 11,
} coap_error_t;

/**
 * @brief CoAP终端回调函数
 * @para [out] scratch 作用未知
 * @para [in] inpkt CoAP请求
 * @para [out] outpkt CoAP响应
 * @para [in] id_hi Message ID高字节
 * @para [in] id_lo Message ID低字节
 * @note 在回调函数最后调用 coap_make_response()
 */
typedef int (*coap_endpoint_func)
            (coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt,
             uint8_t id_hi, uint8_t id_lo);

/**
 * @brief 路由名称描述，暂时不使用
 * @note /foo/bar
 */
typedef struct
{
    uint8_t name[32];
    uint8_t len;
} coap_route_t;

#define MAX_SEGMENTS 2
/**
 * @brief CoAP终端路径
 * @note /foo/bar 
 */
typedef struct
{
    int count;
    // const char *elems[MAX_SEGMENTS];
    coap_route_t route[MAX_SEGMENTS];
} coap_endpoint_path_t;

/**
 * @brief CoAP终端结构体
 */
typedef struct
{
    coap_method_t method;               /**< 访问方法 */
    coap_endpoint_func handler;         /**< 回调函数 */
    const coap_endpoint_path_t *path;   /**< 路径 例如 /hello /light */
    const char *core_attr;              /**< 类型指示 例如 ct=40, ct=50 */
} coap_endpoint_t;

void coap_dump_packet(coap_packet_t pkt);

int coap_parse(coap_packet_t *pkt, const uint8_t *buf, size_t buflen);

int coap_buffer_to_string(char *strbuf, size_t strbuflen, const coap_buffer_t *buf);
const coap_option_t *coap_find_options(const coap_packet_t *pkt, uint8_t num, uint8_t *count);

int coap_build(uint8_t *buf, size_t *buflen, const coap_packet_t *pkt);
void coap_dump(const uint8_t *buf, size_t buflen);

int coap_make_response(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *pkt, const uint8_t *content, size_t content_len, uint8_t msgid_hi, uint8_t msgid_lo, const coap_buffer_t* tok, coap_responsecode_t rspcode, coap_content_type_t content_type);
int coap_handle_req(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt);

void coap_option_nibble(uint32_t value, uint8_t *nibble);
void coap_setup(void);
void endpoint_setup(void);

#ifdef __cplusplus
}
#endif

#endif

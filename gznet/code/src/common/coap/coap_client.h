/**
 * @brief       : CoAP Client客户端
 * @file        : coap.h
 * @version     : v0.0.1
 * @author      : hongpeng.cui
 * @date        : 2015-07-07
 * change logs  :
 * Date       Version     Author        Note
 * 2015-07-07  v0.0.1     hongpeng.cui  first version
 * 2015-10-13  v0.0.2     xukai         修改部分内容
 *
 * @details This file provides functions for parsing and building CoAP message packets
 *          using only the actual binary of the message, not needing additional memory
 *          for secondary data structures.
 */

#ifndef _COAP_H_
#define _COAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_arch/osel_arch.h"
    
/**
 * @breaf CoAP Defined Parameters.
 * #   +-------------------+---------------+
 * #   | name              | default value |
 * #   +-------------------+---------------+
 * #   | ACK_TIMEOUT       | 2 seconds     |
 * #   | ACK_RANDOM_FACTOR | 1.5           |
 * #   | MAX_RETRANSMIT    | 4             |
 * #   | NSTART            | 1             |
 * #   | DEFAULT_LEISURE   | 5 seconds     |
 * #   | PROBING_RATE      | 1 Byte/second |
 * 
 * #   | MAX_TRANSMIT_SPAN | 45 s          |
 * #   | MAX_TRANSMIT_WAIT | 93 s          |
 * #   | MAX_LATENCY       | 100 s         |
 * #   | PROCESSING_DELAY  | 2 s           |
 * #   | MAX_RTT           | 202 s         |
 * #   | EXCHANGE_LIFETIME | 247 s         |
 * #   | NON_LIFETIME      | 145 s         |
 * #   +-------------------+---------------+
 */
#define COAP_ACK_TIMEOUT          2
#define COAP_ACK_RANDOM_FACTOR    1.5   
#define COAP_MAX_RETRANSMIT       4
#define COAP_NSTART               1
#define COAP_DEFAULT_LEISURE      5
#define COAP_PROBING_RATE         1

#define COAP_MAX_TRANSMIT_SPAN   45
#define COAP_MAX_TRANSMIT_WAIT   93
#define COAP_MAX_LATENCY        100
#define COAP_PROCESSING_DELAY     2
#define COAP_MAX_RTT            202
#define COAP_EXCHANGE_LIFETIME  247
#define COAP_NON_LIFETIME       145

/**
 * @breaf Status Codes
 * These codes represent the possible errors that functions in this library can return.
 */
typedef enum
{
    CE_NONE = 0,
    CE_INVALID_PACKET,
    CE_BAD_VERSION,
    CE_TOKEN_LENGTH_OUT_OF_RANGE,
    CE_UNKNOWN_CODE,
    CE_TOO_MANY_OPTIONS,
    CE_OUT_OF_ORDER_OPTIONS_LIST,
    CE_INSUFFICIENT_BUFFER,
    CE_FOUND_PAYLOAD_MARKER,
    CE_END_OF_PACKET
} coap_error_t;

/**
 * @breaf Protocol Versions
 * All known version of the protocol.
 */
typedef enum
{
    COAP_V1 = 1
} coap_version_t;

/**
 * @breaf Message Types
 * The four types of messages possible.
 */
typedef enum
{
    CT_CON = 0,                             /**< 'CON' */
    CT_NON = 1,                             /**< 'NON' */
    CT_ACK = 2,                             /**< 'ACK' */
    CT_RST = 3                              /**< 'RST' */
} coap_type_t;

/**
 * @breaf Message Codes
 * All known message request/response codes.
 */
typedef enum 
{
    CC_EMPTY = 0,                           /**< 'EMPTY' */
    CC_GET = 1,                             /**< 'GET' */
    CC_POST = 2,                            /**< 'POST' */
    CC_PUT = 3,                             /**< 'PUT' */
    CC_DELETE = 4,                          /**< 'DELETE' */
    CC_CREATED = 65,                        /**< '2.01 Created' */ 
    CC_DELETED = 66,                        /**< '2.02 Deleted' */
    CC_VALID = 67,                          /**< '2.03 Valid' */
    CC_CHANGED = 68,                        /**< '2.04 Changed' */
    CC_CONTENT = 69,                        /**< '2.05 Content' */
    CC_CONTINUE = 95,                       /**< '2.31 Continue' */
    CC_BAD_REQUEST = 128,                   /**< '4.00 Bad Request' */
    CC_UNAUTHORIZED = 129,                  /**< '4.01 Unauthorized' */
    CC_BAD_OPTION = 130,                    /**< '4.02 Bad Option' */
    CC_FORBIDDEN = 131,                     /**< '4.03 Forbidden' */
    CC_NOT_FOUND = 132,                     /**< '4.04 Not Found' */
    CC_METHOD_NOT_ALLOWED = 133,            /**< '4.05 Method Not Allowed' */
    CC_NOT_ACCEPTABLE = 134,                /**< '4.06 Not Acceptable' */
    CC_REQUEST_ENTITY_INCOMPLETE = 136,     /**< '4.08 Request Entity Incomplete' */ 
    CC_PRECONDITION_FAILED = 140,           /**< '4.12 Precondition Failed' */
    CC_REQUEST_ENTITY_TOO_LARGE = 141,      /**< '4.13 Request Entity Too Large' */
    CC_UNSUPPORTED_CONTENT  = 143,          /**< '4.15 Unsupported Content-Format' */
    CC_INTERNAL_SERVER_ERROR = 160,         /**< '5.00 Internal Server Error' */
    CC_NOT_IMPLEMENTED = 161,               /**< '5.01 Not Implemented' */
    CC_BAD_GATEWAY = 162,                   /**< '5.02 Bad Gateway' */
    CC_SERVICE_UNAVAILABLE = 163,           /**< '5.03 Service Unavailable' */
    CC_GATEWAY_TIMEOUT = 164,               /**< '5.04 Gateway Timeout' */
    CC_PROXYING_NOT_SUPPORTED = 165         /**< '5.05 Proxying Not Supported' */
} coap_code_t;

/**
 * @breaf Option Numbers
 * All known option numbers.
 */
typedef enum 
{
    CON_IF_MATCH = 1,
    CON_URI_HOST = 3,
    CON_ETAG = 4,
    CON_IF_NONE_MATCH = 5,
    CON_OBSERVE = 6,
    CON_URI_PORT = 7,
    CON_LOCATION_PATH = 8,
    CON_URI_PATH = 11,
    CON_CONTENT_FORMAT = 12,
    CON_MAX_AGE = 14,
    CON_URI_QUERY = 15,
    CON_ACCEPT = 17,
    CON_LOCATION_QUERY = 20,
    CON_BLOCK2 = 23,
    CON_BLOCK1 = 27,
    CON_PROXY_URI = 35,
    CON_PROXY_SCHEME = 39,
    CON_SIZE1 = 60
} coap_option_number_t;

typedef enum coap_content_format
{
    CCF_TEXT = 0,
    CCF_LINK_FORMAT = 40,
    CCF_XML = 41,
    CCF_OCTECT_STREAM = 42,
    CCF_EXI = 47,
    CCF_JSON = 50
} coap_content_format;

/**
 * @breaf Packet Data Unit
 * This contains all information about the message buffer.
 */
typedef struct 
{
    uint8_t *buf;       /**< pointer to buffer */
    size_t len;         /**< length of current message */
    size_t max;         /**< size of buffer */
    uint8_t *opt_ptr;   /**< Internal Pointer for Option Iterator */
} coap_pdu_t;

/**
 * @breaf CoAP Option
 * One option in a CoAP message.
 */
typedef struct 
{
    uint16_t num;   /**< size of buffer */
    size_t len;     /**< length of the value */
    uint8_t *val;   /**< pointer value */
} coap_option_t;

/**
 * @breaf CoAP Payload
 * Payload container.
 */
typedef struct 
{
    size_t len;     /**< length of current message */
    uint8_t *val;   /**< pointer to buffer */
} coap_payload_t;


/**
 * @brief Parses the given packet to check if it is a valid CoAP message.
 * @param  [in] pdu pointer to the coap message struct.
 * @return error code (CE_NONE == 0 == no error).
 * @see coap_error_t | coap_init_pdu
 */
coap_error_t coap_validate_pkt(coap_pdu_t *pdu);

/**
 * @brief Extracts the CoAP version from the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @return version.
 * @see coap_version_t.
 */
coap_version_t  coap_get_version(coap_pdu_t *pdu);

/**
 * @brief Extracts the message type from the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @return type.
 * @see coap_type_t.
 */
coap_type_t coap_get_type(coap_pdu_t *pdu);

/**
 * @brief Extracts the token length from the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @return length.
 * @see coap_type_t.
 */
uint8_t coap_get_tkl(coap_pdu_t *pdu);

/**
 * @brief Extracts the message code from the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @return code.
 * @see coap_code_t.
 */
coap_code_t coap_get_code(coap_pdu_t *pdu);

/**
 * @brief Gets the class portion of the message code.
 * @param  [in] pdu pointer to the coap message struct.
 * @see coap_get_code.
 */
uint8_t coap_get_code_class(coap_pdu_t *pdu);

/**
 * @brief Gets the detail portion of the message code.
 * @param  [in] pdu pointer to the coap message struct.
 * @see coap_get_code.
 */
uint8_t coap_get_code_detail(coap_pdu_t *pdu);

/**
 * @brief Extracts the message ID from the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @see mid.
 */
uint16_t coap_get_mid(coap_pdu_t *pdu);

/**
 * @brief Extracts the token from the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @return token
 */
uint64_t  coap_get_token(coap_pdu_t *pdu);

/**
 * @brief Iterates over the options in the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in]  pointer to the last/next option, pass 0 for the first option.
 * @return coap_option_t.
 */
coap_option_t coap_get_option(coap_pdu_t *pdu, coap_option_t *last);

/**
 * @brief Gets a single specified by the option number and index of which occurrence of that option number you'd like.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] num option number to get.
 * @param  [in] occ occurrence of to get (0th, 1st, 2nd, etc)
 * @return coap_option_t.
 */
coap_option_t coap_get_option_by_num(coap_pdu_t *pdu, coap_option_number_t num, uint8_t occ);

/**
 * @brief Extracts the option with the given index in the given message.
 * @param  [in] pdu pointer to the coap message struct.
 * @return coap_payload.
 */
coap_payload_t coap_get_payload(coap_pdu_t *pdu);

/**
 * @brief Decoding Functions (Intended for Internal Use)
 * @return coap_error_t.
 */
coap_error_t coap_decode_option(uint8_t *pkt_ptr, size_t pkt_len,
                              uint16_t *option_number, size_t *option_length, uint8_t **value);

/**
 * @brief Initializes on an empty buffer for creating new CoAP packets.
 * @param  [in] pdu pointer to the coap message struct.
 * @return coap_error_t (0 == no error).
 */
coap_error_t coap_init_pdu(coap_pdu_t *pdu);

/**
 * @brief Sets the version number header field.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] ver version to set. Must be COAP_V1.
 * @return coap_error_t (0 == no error).
 * @see coap_version_t.
 */
coap_error_t coap_set_version(coap_pdu_t *pdu, coap_version_t ver);

/**
 * @brief Sets the message type header field.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] mtype type to set.
 * @return coap_error_t (0 == no error).
 * @see coap_type_t.
 */
coap_error_t coap_set_type(coap_pdu_t *pdu, coap_type_t mtype);

/**
 * @brief Sets the message type header field.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] code code to set.
 * @return coap_error_t (0 == no error).
 * @see coap_code_t
 */
coap_error_t coap_set_code(coap_pdu_t *pdu, coap_code_t code);

/**
 * @brief Sets the message ID header field.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] mid message ID to set.
 * @return coap_error_t (0 == no error).
 */
coap_error_t coap_set_mid(coap_pdu_t *pdu, uint16_t mid);

/**
 * @brief Sets the message token header field.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] token token value to set.
 * @return coap_error_t (0 == no error).
 */
coap_error_t coap_set_token(coap_pdu_t *pdu, uint64_t token, uint8_t tkl);

/**
 * @brief Adds an option to the existing message.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] opt option container.
 * @return coap_error_t (0 == no error).
 * @node Options SHOULD be added in order of option number. 
 * In the case of multiple options of the same type, they are 
 * sorted in the order that they are added.
 */
coap_error_t coap_add_option(coap_pdu_t *pdu, int32_t opt_num, uint8_t* value, uint16_t opt_len);

/**
 * @brief Sets the payload of the given message to the value in `payload`.
 * @param  [in] pdu pointer to the coap message struct.
 * @param  [in] pl payload container.
 * @return coap_error_t (0 == no error).
 */
coap_error_t coap_set_payload(coap_pdu_t *pdu, uint8_t *payload, size_t payload_len);

/**
 * @brief Gets the class portion of the message code.
 * @param [in] class  the code class.
 * @param [in] detail the code detail.
 * @see coap_get_code
 */
static inline uint8_t coap_build_code(uint8_t _class, uint8_t detail) { return (_class << 5) | detail; }

/**
 * @brief Adjust the option deltas for the rest of the options.
 * Internal Method.
 */
coap_error_t coap_adjust_option_deltas(uint8_t *opts, size_t *opts_len, size_t max_len, int32_t offset);

/**
 * @brief Insert the Header
 * Internal Method.
 */
int8_t coap_build_option_header(uint8_t *buf, size_t max_len, int32_t opt_delta, int32_t opt_len);

/**
 * @brief Build New Header
 * Internal Method.
 */
int8_t coap_compute_option_header_len(int32_t opt_delta, int32_t opt_len);

#ifdef __cplusplus
}
#endif

#endif /*_COAP_H_*/


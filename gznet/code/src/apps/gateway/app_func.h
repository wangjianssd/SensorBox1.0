#ifndef __APP_FUNC_H__
#define __APP_FUNC_H__

void uart_frame_cfg(void);

void north_register_func(void);

bool_t platform_state(void);

void cnn_platform(void);

void discnn_platform(void);

void reverse_id_endian(uint8_t *id);

void uart_event_handle(void);

bool_t judge_self_id(uint8_t *judge_id);
#endif
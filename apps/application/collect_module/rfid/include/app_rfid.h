 /**
 * @brief       : rfid的业务处理
 *
 * @file        : app_rfid.h
 * @author      : zhangzhan
 * @version     : v0.0.1
 * @date        : 2015/09/15
 *
 * Date        Version      Author      Notes
 * 2015/09/15  v0.0.1       zhangzhan   first version
 */
#ifndef _APP_RFID_H_
#define _APP_RFID_H_

void box_nfc_init(void);
void m24lr64e_int_proc(void);
void box_nfc_int_handle(void *arg);
void box_gprs_stop_timer_handle(void *arg);

void fingerprints_int_proc(void);
void box_fingerprints_int_handle(void *arg);
void nfc_no_lock_timeout_timer_cb(void);
void stop_nfc_no_lock_timeout(void);
#endif
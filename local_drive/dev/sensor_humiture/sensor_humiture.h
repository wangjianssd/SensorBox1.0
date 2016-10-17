#ifndef __SENSOR_HUMITURE_H__
#define __SENSOR_HUMITURE_H__

#define HUMITURE_SENSOR_RH	0
#define HUMITURE_SENSOR_T	1

bool_t humiture_sensor_init(void);
bool_t humiture_sensor_read_h(uint16_t *pval);
bool_t humiture_sensor_read_t(uint16_t *pval);

#endif
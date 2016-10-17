#ifndef _APP_HUMITURE_H_
#define _APP_HUMITURE_H_

#define BOX_HUMITURE_PERIOD      600000//1800000//60000
#define BOX_HUMITURE_MAX         80
#define BOX_HUMITURE_MIN         0
#define BOX_TEMPERTURE_MAX         0
#define BOX_TEMPERTURE_MIN         0

float SHT2x_CalcRH(uint16_t u16sRH);
float SHT2x_CalcTemperatureC(uint16_t u16sT);

void box_humiture_timer_handle(void *arg);
void box_humiture_init(void);
void box_humiture_set_period(uint32_t period);
void humiture_stop_timer();
#endif
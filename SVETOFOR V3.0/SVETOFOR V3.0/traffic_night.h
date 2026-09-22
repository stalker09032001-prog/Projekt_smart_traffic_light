#ifndef TRAFFIC_NIGHT_H
#define TRAFFIC_NIGHT_H

#include <stdint.h>
#include "types.h"

extern uint8_t night_led_on;
extern uint32_t night_state_start;
extern uint8_t night_state_initialized;

void Process_Night(void);
void Process_Auto_Night(void);
uint8_t Is_Night_Time(const RTC_Time *time);

#endif
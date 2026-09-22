#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#include <stdint.h>
#include "types.h"

extern TrafficMode current_mode;
extern uint16_t auto_stage_time[4];
extern uint16_t night_on_time;
extern uint16_t night_off_time;
extern uint8_t night_start_hour;
extern uint8_t night_start_minute;
extern uint8_t night_start_second;
extern uint8_t night_end_hour;
extern uint8_t night_end_minute;
extern uint8_t night_end_second;

uint8_t EEPROM_Load(void);
void EEPROM_Save(void);

#endif
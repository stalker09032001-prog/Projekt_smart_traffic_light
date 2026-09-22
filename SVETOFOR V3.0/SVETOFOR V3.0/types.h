#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef enum
{
	MODE_OFF = 0,
	MODE_AUTO,
	MODE_NIGHT,
	MODE_AUTO_NIGHT
} TrafficMode;

typedef enum
{
	STAGE_1 = 0,
	STAGE_2,
	STAGE_3,
	STAGE_4
} AutoStage;

typedef struct
{
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} RTC_Time;

typedef struct
{
	uint16_t magic;
	uint8_t version;
	uint8_t mode;
	uint16_t auto_stage_time[4];
	uint16_t night_on_time;
	uint16_t night_off_time;
	uint8_t night_start_hour;
	uint8_t night_start_minute;
	uint8_t night_start_second;
	uint8_t night_end_hour;
	uint8_t night_end_minute;
	uint8_t night_end_second;
	uint8_t crc;
} EEPROM_Config;

#endif

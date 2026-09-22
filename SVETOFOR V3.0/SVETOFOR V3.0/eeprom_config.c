#include "config.h"
#include "types.h"
#include "eeprom_config.h"

#include <avr/eeprom.h>

TrafficMode current_mode = MODE_OFF;

uint16_t auto_stage_time[4] = {30, 5, 20, 5};

uint16_t night_on_time = 2;
uint16_t night_off_time = 2;

uint8_t night_start_hour = 22;
uint8_t night_start_minute = 0;
uint8_t night_start_second = 0;

uint8_t night_end_hour = 6;
uint8_t night_end_minute = 0;
uint8_t night_end_second = 0;

static EEPROM_Config EEMEM eeprom_config;

static void EEPROM_SetDefaults(void)
{
	current_mode = MODE_OFF;

	auto_stage_time[0] = 30;
	auto_stage_time[1] = 5;
	auto_stage_time[2] = 20;
	auto_stage_time[3] = 5;

	night_on_time = 2;
	night_off_time = 2;

	night_start_hour = 22;
	night_start_minute = 0;
	night_start_second = 0;

	night_end_hour = 6;
	night_end_minute = 0;
	night_end_second = 0;
}

static uint8_t EEPROM_CRC8(const uint8_t *data, uint8_t length)
{
	uint8_t crc = 0;
	uint8_t i;
	uint8_t bit;

	for (i = 0; i < length; i++)
	{
		crc ^= data[i];

		for (bit = 0; bit < 8; bit++)
		{
			if (crc & 0x80)
			crc = (uint8_t)((crc << 1) ^ 0x07);
			else
			crc <<= 1;
		}
	}

	return crc;
}

static void EEPROM_BuildConfig(EEPROM_Config *config)
{
	uint8_t i;

	config->magic = EEPROM_CONFIG_MAGIC;
	config->version = EEPROM_CONFIG_VERSION;
	config->mode = (uint8_t)current_mode;

	for (i = 0; i < 4; i++)
	config->auto_stage_time[i] = auto_stage_time[i];

	config->night_on_time = night_on_time;
	config->night_off_time = night_off_time;

	config->night_start_hour = night_start_hour;
	config->night_start_minute = night_start_minute;
	config->night_start_second = night_start_second;

	config->night_end_hour = night_end_hour;
	config->night_end_minute = night_end_minute;
	config->night_end_second = night_end_second;

	config->crc = EEPROM_CRC8(
	(const uint8_t *)config,
	sizeof(EEPROM_Config) - 1
	);
}

static uint8_t EEPROM_IsValid(const EEPROM_Config *config)
{
	uint8_t crc;

	if (config->magic != EEPROM_CONFIG_MAGIC)
	return 0;

	if (config->version != EEPROM_CONFIG_VERSION)
	return 0;

	if (config->mode > MODE_AUTO_NIGHT)
	return 0;

	if (config->auto_stage_time[0] == 0 &&
	config->auto_stage_time[1] == 0 &&
	config->auto_stage_time[2] == 0 &&
	config->auto_stage_time[3] == 0)
	{
		return 0;
	}

	if (config->night_on_time == 0 ||
	config->night_off_time == 0)
	{
		return 0;
	}

	if (config->night_start_hour > 23 ||
	config->night_start_minute > 59 ||
	config->night_start_second > 59)
	{
		return 0;
	}

	if (config->night_end_hour > 23 ||
	config->night_end_minute > 59 ||
	config->night_end_second > 59)
	{
		return 0;
	}

	crc = EEPROM_CRC8(
	(const uint8_t *)config,
	sizeof(EEPROM_Config) - 1
	);

	if (crc != config->crc)
	return 0;

	return 1;
}

void EEPROM_Save(void)
{
	EEPROM_Config config;

	EEPROM_BuildConfig(&config);

	eeprom_update_block(
	&config,
	&eeprom_config,
	sizeof(EEPROM_Config)
	);
}

uint8_t EEPROM_Load(void)
{
	EEPROM_Config config;
	uint8_t i;

	eeprom_read_block(
	&config,
	&eeprom_config,
	sizeof(EEPROM_Config)
	);

	if (!EEPROM_IsValid(&config))
	{
		EEPROM_SetDefaults();
		EEPROM_Save();
		return 0;
	}

	current_mode = (TrafficMode)config.mode;

	for (i = 0; i < 4; i++)
	auto_stage_time[i] = config.auto_stage_time[i];

	night_on_time = config.night_on_time;
	night_off_time = config.night_off_time;

	night_start_hour = config.night_start_hour;
	night_start_minute = config.night_start_minute;
	night_start_second = config.night_start_second;

	night_end_hour = config.night_end_hour;
	night_end_minute = config.night_end_minute;
	night_end_second = config.night_end_second;

	return 1;
}

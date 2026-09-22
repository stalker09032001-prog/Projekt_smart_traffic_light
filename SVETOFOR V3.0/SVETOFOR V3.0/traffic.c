#include "config.h"
#include "types.h"
#include "traffic.h"
#include "display.h"
#include "eeprom_config.h"
#include "traffic_auto.h"
#include "traffic_night.h"

#include <avr/io.h>

static void Reset_Mode_State(void)
{
	pedestrian_request = 0;
	pedestrian_request_time = 0;
	auto_state_initialized = 0;
	night_state_initialized = 0;
	night_led_on = 0;
	stage_start_time = 0;
	night_state_start = 0;
}

void Traffic_AllOff(void)
{
	PORTD &= (uint8_t)~(
	(1 << CAR_RED) |
	(1 << CAR_YELLOW) |
	(1 << CAR_GREEN) |
	(1 << PED_RED) |
	(1 << PED_GREEN)
	);
}

void Traffic_Stage1(void)
{
	Traffic_AllOff();
	PORTD |= (1 << CAR_GREEN) | (1 << PED_RED);
}

void Traffic_Stage2(void)
{
	Traffic_AllOff();
	PORTD |= (1 << CAR_YELLOW) | (1 << PED_RED);
}

void Traffic_Stage3(void)
{
	Traffic_AllOff();
	PORTD |= (1 << CAR_RED) | (1 << PED_GREEN);
}

void Traffic_Stage4(void)
{
	Traffic_AllOff();
	PORTD |= (1 << CAR_YELLOW) | (1 << PED_RED);
}

void Traffic_Night(uint8_t yellow_on)
{
	Traffic_AllOff();

	if (yellow_on)
	PORTD |= (1 << CAR_YELLOW);
}

void Set_Mode(TrafficMode mode)
{
	Reset_Mode_State();
	current_mode = mode;

	current_stage = STAGE_1;

	switch (current_mode)
	{
		case MODE_OFF:
		Traffic_AllOff();
		Display_Off();
		break;

		case MODE_AUTO:
		Traffic_Stage1();
		Display_SetNumber(auto_stage_time[0]);
		break;

		case MODE_NIGHT:
		Traffic_Night(0);
		Display_Off();
		break;

		case MODE_AUTO_NIGHT:
		Traffic_AllOff();
		Display_Off();
		break;

		default:
		Traffic_AllOff();
		Display_Off();
		break;
	}
}

#include "config.h"
#include "types.h"
#include "traffic_auto.h"
#include "traffic.h"
#include "display.h"
#include "rtc.h"
#include "eeprom_config.h"

AutoStage current_stage = STAGE_1;

uint8_t pedestrian_request = 0;
uint32_t pedestrian_request_time = 0;

uint32_t stage_start_time = 0;
uint8_t auto_state_initialized = 0;

static uint8_t Auto_FindNextActiveStage(
AutoStage current,
AutoStage *next_stage)
{
	uint8_t step;
	uint8_t index;

	for (step = 1; step <= 4; step++)
	{
		index = ((uint8_t)current + step) % 4;

		if (auto_stage_time[index] != 0)
		{
			*next_stage = (AutoStage)index;
			return 1;
		}
	}

	return 0;
}

static AutoStage Auto_FirstActiveStage(void)
{
	uint8_t i;

	for (i = 0; i < 4; i++)
	{
		if (auto_stage_time[i] != 0)
		return (AutoStage)i;
	}

	return STAGE_1;
}

void Auto_EnterStage(AutoStage stage, uint32_t now)
{
	uint8_t attempts;

	for (attempts = 0; attempts < 4; attempts++)
	{
		if (auto_stage_time[stage] != 0)
		break;

		stage = (AutoStage)(((uint8_t)stage + 1) % 4);
	}

	if (auto_stage_time[stage] == 0)
	{
		Traffic_AllOff();
		Display_Off();
		return;
	}

	current_stage = stage;
	stage_start_time = now;
	pedestrian_request = 0;

	switch (stage)
	{
		case STAGE_1:
		Traffic_Stage1();
		Display_SetNumber(auto_stage_time[0]);
		break;

		case STAGE_2:
		Traffic_Stage2();
		Display_SetNumber(auto_stage_time[1]);
		break;

		case STAGE_3:
		Traffic_Stage3();
		Display_SetNumber(auto_stage_time[2]);
		break;

		case STAGE_4:
		Traffic_Stage4();
		Display_SetNumber(auto_stage_time[3]);
		break;
	}
}

void Auto_Initialize(uint32_t now)
{
	AutoStage first_stage = Auto_FirstActiveStage();

	current_stage = first_stage;
	stage_start_time = now;
	pedestrian_request = 0;
	pedestrian_request_time = 0;
	auto_state_initialized = 1;

	Auto_EnterStage(first_stage, now);
}

void Process_Auto(void)
{
	uint32_t now;
	uint32_t elapsed;
	uint32_t remaining;
	uint16_t duration;
	AutoStage next_stage;

	now = RTC_GetSecondsFromMidnight(&rtc);

	if (!auto_state_initialized)
	{
		Auto_Initialize(now);
		return;
	}

	elapsed = Seconds_Difference(now, stage_start_time);
	duration = auto_stage_time[current_stage];

	if (current_stage == STAGE_1 && pedestrian_request)
	{
		uint32_t request_elapsed;

		request_elapsed =
		Seconds_Difference(now, pedestrian_request_time);

		if (request_elapsed >= PEDESTRIAN_DELAY_SEC)
		{
			pedestrian_request = 0;
			Auto_EnterStage(STAGE_2, now);
			return;
		}

		Display_SetNumber(
		(uint16_t)(PEDESTRIAN_DELAY_SEC - request_elapsed)
		);

		return;
	}

	if (elapsed >= duration)
	{
		if (Auto_FindNextActiveStage(current_stage, &next_stage))
		{
			Auto_EnterStage(next_stage, now);
		}
		else
		{
			Traffic_AllOff();
			Display_Off();
		}

		return;
	}

	remaining = (uint32_t)duration - elapsed;
	Display_SetNumber((uint16_t)remaining);
}

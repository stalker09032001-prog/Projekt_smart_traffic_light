#include "config.h"
#include "types.h"
#include "traffic_night.h"
#include "traffic.h"
#include "display.h"
#include "rtc.h"
#include "eeprom_config.h"
#include "traffic_auto.h"

uint8_t night_led_on = 0;
uint32_t night_state_start = 0;
uint8_t night_state_initialized = 0;

uint8_t Is_Night_Time(const RTC_Time *time)
{
	uint32_t now;
	uint32_t start;
	uint32_t end;

	now = Time_To_Seconds(
	time->hours,
	time->minutes,
	time->seconds
	);

	start = Time_To_Seconds(
	night_start_hour,
	night_start_minute,
	night_start_second
	);

	end = Time_To_Seconds(
	night_end_hour,
	night_end_minute,
	night_end_second
	);

	if (start == end)
	return 1;

	if (start < end)
	return (now >= start && now < end);

	return (now >= start || now < end);
}

static void Night_Initialize(uint32_t now)
{
	night_state_initialized = 1;
	night_led_on = 1;
	night_state_start = now;

	Traffic_Night(1);
	Display_Off();
}

void Process_Night(void)
{
	uint32_t now;
	uint32_t elapsed;
	uint16_t required_time;

	now = RTC_GetSecondsFromMidnight(&rtc);

	if (!night_state_initialized)
	{
		Night_Initialize(now);
		return;
	}

	if (night_led_on)
	required_time = night_on_time;
	else
	required_time = night_off_time;

	if (required_time == 0)
	required_time = 1;

	elapsed = Seconds_Difference(now, night_state_start);

	if (elapsed >= required_time)
	{
		night_led_on = !night_led_on;
		night_state_start = now;
		Traffic_Night(night_led_on);
	}

	Display_Off();
}

void Process_Auto_Night(void)
{
	uint8_t night;

	night = Is_Night_Time(&rtc);

	if (night)
	{
		if (!night_state_initialized)
		auto_state_initialized = 0;

		Process_Night();
		return;
	}

	if (night_state_initialized)
	{
		uint32_t now;

		night_state_initialized = 0;
		auto_state_initialized = 0;
		pedestrian_request = 0;

		now = RTC_GetSecondsFromMidnight(&rtc);
		Auto_Initialize(now);
		return;
	}

	Process_Auto();
}

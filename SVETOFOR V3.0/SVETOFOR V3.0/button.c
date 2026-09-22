#include "config.h"
#include "button.h"
#include "types.h"
#include "rtc.h"
#include "traffic_auto.h"
#include "eeprom_config.h"
#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t button_event = 0;
static volatile uint8_t button_debounce_counter = 0;

void Button_Init(void)
{
	DDRD &= (uint8_t)~(1 << BUTTON);
	PORTD |= (1 << BUTTON);

	EICRA |= (1 << ISC01);
	EICRA &= (uint8_t)~(1 << ISC00);
	EIMSK |= (1 << INT0);
}

void Button_TimerTick(void)
{
	if (button_debounce_counter > 0)
	button_debounce_counter--;
}

ISR(INT0_vect)
{
	if (button_debounce_counter != 0)
	return;

	button_debounce_counter = BUTTON_DEBOUNCE_TICKS;
	button_event = 1;
}

void Process_Button_Event(void)
{
	uint8_t event;
	uint32_t now;
	uint32_t elapsed;
	uint32_t remaining;

	uint8_t sreg = SREG;
	cli();
	event = button_event;
	button_event = 0;
	SREG = sreg;

	if (!event)
	return;

	if (current_mode != MODE_AUTO &&
	current_mode != MODE_AUTO_NIGHT)
	return;

	if (current_stage != STAGE_1)
	return;

	if (auto_stage_time[STAGE_1] == 0)
	return;

	if (pedestrian_request)
	return;

	now = RTC_GetSecondsFromMidnight(&rtc);
	elapsed = Seconds_Difference(now, stage_start_time);

	if (elapsed >= auto_stage_time[STAGE_1])
	return;

	remaining = (uint32_t)auto_stage_time[STAGE_1] - elapsed;

	if ((remaining < ((uint32_t)auto_stage_time[STAGE_1] / 2UL)) && (remaining > PEDESTRIAN_DELAY_SEC))
	{
		pedestrian_request = 1;
		pedestrian_request_time = now;
		UART_SendString_P(str_ok_request);
	}
	else
	{
		UART_SendString_P(str_request_ignored);
	}
}

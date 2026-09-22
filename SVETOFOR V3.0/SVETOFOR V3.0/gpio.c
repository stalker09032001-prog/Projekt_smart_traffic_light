#include "config.h"
#include "gpio.h"
#include "traffic.h"
#include "display.h"

#include <avr/io.h>

void GPIO_Init(void)
{
	DDRD |= (1 << CAR_RED) | (1 << CAR_YELLOW) | (1 << CAR_GREEN) |
	(1 << PED_RED) | (1 << PED_GREEN);

	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) |
	(1 << PB3) | (1 << PB4) | (1 << PB5);

	DDRC |= (1 << DIGIT_UNITS) | (1 << DIGIT_TENS) |
	(1 << DIGIT_HUNDREDS) | (1 << SEGMENT_G);

	Traffic_AllOff();
	Display_DisableAll();
	Display_ClearSegments();
}

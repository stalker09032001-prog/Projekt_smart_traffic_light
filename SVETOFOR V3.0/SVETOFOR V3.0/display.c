#include "config.h"
#include "display.h"
#include "button.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static volatile uint16_t display_value = 0;
static volatile uint8_t display_digit = 0;
static volatile uint8_t display_enabled = 0;

static const uint8_t segment_table[10] PROGMEM =
{
	0,
	(1 << PB0) | (1 << PB3) | (1 << PB4) | (1 << PB5),
	(1 << PB2) | (1 << PB5),
	(1 << PB4) | (1 << PB5),
	(1 << PB0) | (1 << PB3) | (1 << PB4),
	(1 << PB1) | (1 << PB4),
	(1 << PB1),
	(1 << PB3) | (1 << PB4) | (1 << PB5),
	0,
	(1 << PB4)
};

static const uint8_t segment_g_table[10] PROGMEM =
{
	1, 1, 0, 0, 0, 0, 0, 1, 0, 0
};

void Display_DisableAll(void)
{
	PORTC &= (uint8_t)~(
	(1 << DIGIT_UNITS) |
	(1 << DIGIT_TENS) |
	(1 << DIGIT_HUNDREDS)
	);
}

void Display_ClearSegments(void)
{
	PORTB &= (uint8_t)~(
	(1 << PB0) | (1 << PB1) | (1 << PB2) |
	(1 << PB3) | (1 << PB4) | (1 << PB5)
	);

	PORTC &= (uint8_t)~(1 << SEGMENT_G);
}

void Display_Off(void)
{
	uint8_t sreg = SREG;
	cli();

	display_enabled = 0;
	display_value = 0;

	Display_DisableAll();
	Display_ClearSegments();

	SREG = sreg;
}

static void Display_SetSegments(uint8_t digit)
{
	uint8_t pattern;
	uint8_t g_pattern;

	if (digit > 9)
	digit = 0;

	pattern = pgm_read_byte(&segment_table[digit]);
	g_pattern = pgm_read_byte(&segment_g_table[digit]);

	PORTB = (PORTB & 0xC0) | pattern;

	if (g_pattern)
	PORTC |= (1 << SEGMENT_G);
	else
	PORTC &= (uint8_t)~(1 << SEGMENT_G);
}

void Display_SetNumber(uint16_t number)
{
	uint8_t sreg = SREG;

	if (number > 999)
	number = 999;

	cli();
	display_value = number;
	display_enabled = 1;
	SREG = sreg;
}

void Display_Refresh(void)
{
	uint16_t number;
	uint8_t units;
	uint8_t tens;
	uint8_t hundreds;

	Display_DisableAll();

	if (!display_enabled)
	{
		Display_ClearSegments();
		display_digit = 0;
		return;
	}

	number = display_value;

	units = number % 10;
	tens = (number / 10) % 10;
	hundreds = number / 100;

	if (display_digit == 0)
	{
		Display_SetSegments(units);
		PORTC |= (1 << DIGIT_UNITS);
	}
	else if (display_digit == 1)
	{
		if (number >= 10)
		{
			Display_SetSegments(tens);
			PORTC |= (1 << DIGIT_TENS);
		}
	}
	else
	{
		if (number >= 100)
		{
			Display_SetSegments(hundreds);
			PORTC |= (1 << DIGIT_HUNDREDS);
		}
	}

	display_digit++;

	if (display_digit >= 3)
	display_digit = 0;
}

void Timer1_Init(void)
{
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
	OCR1A = TIMER1_OCR_VALUE;
	TIMSK1 = (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
	Display_Refresh();
	Button_TimerTick();
}

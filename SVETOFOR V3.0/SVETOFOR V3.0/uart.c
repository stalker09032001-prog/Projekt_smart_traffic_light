#include "config.h"
#include "types.h"
#include "uart.h"
#include "rtc.h"
#include "eeprom_config.h"
#include "traffic.h"
#include "display.h"
#include "traffic_auto.h"
#include "traffic_night.h"
#include "reset.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_index = 0;
volatile uint8_t uart_command_ready = 0;

const char str_reset_power[] PROGMEM =
"POWER-ON\r\n";

const char str_reset_software[] PROGMEM =
"RESET CAUSE: SOFTWARE RESET\r\n";

static const char str_startup[] PROGMEM =
"\r\n"
"============================================\r\n"
" ATMEGA328P SVETOFOR v2.1\r\n"
"============================================\r\n"
"CPU: 16 MHz external crystal\r\n"
"UART: 9600 baud\r\n"
"RTC: DS3231\r\n"
"EEPROM: INTERNAL ATMEGA328P\r\n"
"============================================\r\n"
"Type HELP for commands.\r\n";

const char str_rtc_error[] PROGMEM =
"\r\nERROR: RTC NOT RESPONDING\r\n";

const char str_rtc_disabled[] PROGMEM =
"\r\nTRAFFIC LIGHTS DISABLED\r\n";

const char str_rtc_ok[] PROGMEM =
"RTC: OK\r\n";

const char str_ok_request[] PROGMEM =
"\r\nOK: PEDESTRIAN REQUEST\r\n";

const char str_request_ignored[] PROGMEM =
"\r\nINFO: PEDESTRIAN REQUEST IGNORED - "
"STAGE TOO CLOSE TO END\r\n";

static const char str_ok_time[] PROGMEM =
"\r\nOK: TIME SET\r\n";

static const char str_ok_date[] PROGMEM =
"\r\nOK: DATE SET\r\n";

static const char str_ok_auto[] PROGMEM =
"\r\nOK: AUTO TIMES UPDATED AND SAVED\r\n";

static const char str_ok_night[] PROGMEM =
"\r\nOK: NIGHT TIMES UPDATED AND SAVED\r\n";

static const char str_ok_auto_night[] PROGMEM =
"\r\nOK: AUTO NIGHT TIME UPDATED AND SAVED\r\n";

static const char str_mode_off[] PROGMEM =
"\r\nOK: MODE_OFF - SAVED\r\n";

static const char str_mode_auto[] PROGMEM =
"\r\nOK: MODE_AUTO - SAVED\r\n";

static const char str_mode_night[] PROGMEM =
"\r\nOK: MODE_NIGHT - SAVED\r\n";

static const char str_mode_auto_night[] PROGMEM =
"\r\nOK: MODE_AUTO_NIGHT - SAVED\r\n";

static const char str_unknown[] PROGMEM =
"\r\nERROR: UNKNOWN COMMAND\r\n";

static const char str_invalid_time[] PROGMEM =
"\r\nERROR: INVALID TIME\r\n";

static const char str_invalid_date[] PROGMEM =
"\r\nERROR: INVALID DATE\r\n";

static const char str_invalid_auto[] PROGMEM =
"\r\nERROR: INVALID AUTO PARAMETER\r\n";

static const char str_expected_stages[] PROGMEM =
"\r\nERROR: EXPECTED 4 STAGES\r\n";

static const char str_too_many[] PROGMEM =
"\r\nERROR: TOO MANY PARAMETERS\r\n";

static const char str_expected_on[] PROGMEM =
"\r\nERROR: EXPECTED ON_XXX\r\n";

static const char str_expected_off[] PROGMEM =
"\r\nERROR: EXPECTED OFF_XXX\r\n";

static const char str_invalid_on[] PROGMEM =
"\r\nERROR: INVALID ON TIME\r\n";

static const char str_invalid_off[] PROGMEM =
"\r\nERROR: INVALID OFF TIME\r\n";

static const char str_expected_on_word[] PROGMEM =
"\r\nERROR: EXPECTED ON\r\n";

static const char str_expected_off_word[] PROGMEM =
"\r\nERROR: EXPECTED OFF\r\n";

static const char str_invalid_night_start[] PROGMEM =
"\r\nERROR: INVALID NIGHT START\r\n";

static const char str_invalid_night_end[] PROGMEM =
"\r\nERROR: INVALID NIGHT END\r\n";

static const char str_rtc_unavailable[] PROGMEM =
"\r\nERROR: RTC NOT AVAILABLE\r\n";

static const char str_all_auto_zero[] PROGMEM =
"\r\nERROR: ALL AUTO STAGES ARE ZERO\r\n";

static const char str_ok_reset[] PROGMEM =
"\r\nOK: MCU RESETTING...\r\n";

void UART_Init(void)
{
	UBRR0H = (uint8_t)(UART_UBRR >> 8);
	UBRR0L = (uint8_t)UART_UBRR;

	UCSR0A = 0;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendChar(char c)
{
	while (!(UCSR0A & (1 << UDRE0)))
	{
	}

	UDR0 = c;
}

void UART_SendString(const char *text)
{
	while (*text)
	UART_SendChar(*text++);
}

void UART_SendString_P(const char *text)
{
	char c;

	while ((c = (char)pgm_read_byte(text++)) != '\0')
	UART_SendChar(c);
}

void UART_SendNumber(uint16_t value)
{
	char buffer[6];
	uint8_t i = 0;

	if (value == 0)
	{
		UART_SendChar('0');
		return;
	}

	while (value > 0)
	{
		buffer[i++] = (char)('0' + (value % 10));
		value /= 10;
	}

	while (i > 0)
	UART_SendChar(buffer[--i]);
}

void UART_Send2(uint8_t value)
{
	UART_SendChar((char)('0' + (value / 10)));
	UART_SendChar((char)('0' + (value % 10)));
}

ISR(USART_RX_vect)
{
	char c = UDR0;

	if (c == '\r' || c == '\n')
	{
		if (uart_index > 0)
		{
			uart_buffer[uart_index] = '\0';
			uart_command_ready = 1;
		}

		return;
	}

	if (uart_command_ready)
	return;

	if (uart_index < UART_BUFFER_SIZE - 1)
	{
		uart_buffer[uart_index++] = c;
	}
	else
	{
		uart_index = 0;
		uart_buffer[0] = '\0';
		uart_command_ready = 0;
	}
}

static uint8_t Str_Equals(const char *a, const char *b)
{
	while (*a && *b)
	{
		if (*a != *b)
		return 0;

		a++;
		b++;
	}

	return (*a == '\0' && *b == '\0');
}

static uint8_t Str_StartsWith(
const char *str,
const char *prefix)
{
	while (*prefix)
	{
		if (*str++ != *prefix++)
		return 0;
	}

	return 1;
}

static uint8_t Str_Length(const char *str)
{
	uint8_t n = 0;

	while (*str++)
	n++;

	return n;
}

static uint8_t Parse_Time(
const char *str,
uint8_t *hour,
uint8_t *minute,
uint8_t *second)
{
	uint8_t i;

	if (Str_Length(str) != 8)
	return 0;

	if (str[2] != ':' || str[5] != ':')
	return 0;

	for (i = 0; i < 8; i++)
	{
		if (i == 2 || i == 5)
		continue;

		if (str[i] < '0' || str[i] > '9')
		return 0;
	}

	*hour = (uint8_t)((str[0] - '0') * 10 +
	(str[1] - '0'));

	*minute = (uint8_t)((str[3] - '0') * 10 +
	(str[4] - '0'));

	*second = (uint8_t)((str[6] - '0') * 10 +
	(str[7] - '0'));

	if (*hour > 23 || *minute > 59 || *second > 59)
	return 0;

	return 1;
}

static uint8_t Parse_Date(
const char *str,
uint8_t *day,
uint8_t *month,
uint8_t *year)
{
	uint8_t i;

	if (Str_Length(str) != 8)
	return 0;

	if (str[2] != ':' || str[5] != ':')
	return 0;

	for (i = 0; i < 8; i++)
	{
		if (i == 2 || i == 5)
		continue;

		if (str[i] < '0' || str[i] > '9')
		return 0;
	}

	*day = (uint8_t)((str[0] - '0') * 10 +
	(str[1] - '0'));

	*month = (uint8_t)((str[3] - '0') * 10 +
	(str[4] - '0'));

	*year = (uint8_t)((str[6] - '0') * 10 +
	(str[7] - '0'));

	if (*day < 1 || *day > 31 ||
	*month < 1 || *month > 12)
	return 0;

	return 1;
}

static uint8_t Parse_XXX(
const char *str,
uint16_t *value)
{
	uint8_t i;

	if (Str_Length(str) != 3)
	return 0;

	for (i = 0; i < 3; i++)
	{
		if (str[i] < '0' || str[i] > '9')
		return 0;
	}

	*value = (uint16_t)(
	(str[0] - '0') * 100 +
	(str[1] - '0') * 10 +
	(str[2] - '0')
	);

	return (*value != 0);
}

static char *Get_Token(char **text)
{
	char *p = *text;
	char *start;

	while (*p == ' ')
	p++;

	if (*p == '\0')
	{
		*text = p;
		return 0;
	}

	start = p;

	while (*p && *p != ' ')
	p++;

	if (*p)
	{
		*p = '\0';
		p++;
	}

	*text = p;
	return start;
}

static uint8_t Parse_Auto_Parameter(
const char *str,
uint8_t expected_stage,
uint16_t *value)
{
	if (Str_Length(str) != 5)
	return 0;

	if (str[0] != ('0' + expected_stage))
	return 0;

	if (str[1] != '_')
	return 0;

	if (str[2] == '0' &&
	str[3] == '0' &&
	str[4] == '0')
	{
		*value = 0;
		return 1;
	}

	return Parse_XXX(&str[2], value);
}

static void Command_HELP(void)
{
	UART_SendString_P(PSTR(
	"\r\n"
	"============================================\r\n"
	"ATMEGA328P SVETOFOR V2.1\r\n"
	"============================================\r\n"
	"RESET\r\n"
	"HELP\r\n"
	"READ_STATUS\r\n"
	"SET_TIME HH:MM:SS\r\n"
	"SET_DATE DD:MM:YY\r\n"
	"MODE_OFF\r\n"
	"MODE_AUTO\r\n"
	"MODE_NIGHT\r\n"
	"MODE_AUTO_NIGHT\r\n"
	"SET_MODE_AUTO 1_XXX 2_XXX 3_XXX 4_XXX\r\n"
	"SET_MODE_NIGHT ON_XXX OFF_XXX\r\n"
	"SET_MODE_AUTO_NIGHT_TIME ON HH:MM:SS OFF HH:MM:SS\r\n"
	"============================================\r\n"
	));
}

static void UART_SendModeName(TrafficMode mode)
{
	switch (mode)
	{
		case MODE_OFF:
		UART_SendString("MODE_OFF");
		break;

		case MODE_AUTO:
		UART_SendString("MODE_AUTO");
		break;

		case MODE_NIGHT:
		UART_SendString("MODE_NIGHT");
		break;

		case MODE_AUTO_NIGHT:
		UART_SendString("MODE_AUTO_NIGHT");
		break;

		default:
		UART_SendString("UNKNOWN");
		break;
	}
}

static void Command_READ_STATUS(void)
{
	RTC_Time temp;

	if (!RTC_ReadTime(&temp))
	{
		rtc_error = 1;
		Traffic_AllOff();
		Display_Off();
		UART_SendString_P(str_rtc_error);
		return;
	}

	rtc = temp;
	rtc_error = 0;

	UART_SendString_P(PSTR(
	"\r\n"
	"============================================\r\n"
	"STATUS\r\n"
	"============================================\r\n"
	));

	UART_SendString("DATE: ");
	UART_Send2(temp.day);
	UART_SendChar('.');
	UART_Send2(temp.month);
	UART_SendString(".20");
	UART_Send2(temp.year);

	UART_SendString("\r\nTIME: ");
	UART_Send2(temp.hours);
	UART_SendChar(':');
	UART_Send2(temp.minutes);
	UART_SendChar(':');
	UART_Send2(temp.seconds);

	UART_SendString("\r\nMODE: ");
	UART_SendModeName(current_mode);

	UART_SendString_P(PSTR(
	"\r\n\r\nAUTO MODE:\r\n"
	"Stage 1: CAR GREEN / PED RED - "
	));
	UART_SendNumber(auto_stage_time[0]);

	UART_SendString_P(PSTR(
	" s\r\nStage 2: CAR YELLOW / PED RED - "
	));
	UART_SendNumber(auto_stage_time[1]);

	UART_SendString_P(PSTR(
	" s\r\nStage 3: CAR RED / PED GREEN - "
	));
	UART_SendNumber(auto_stage_time[2]);

	UART_SendString_P(PSTR(
	" s\r\nStage 4: CAR YELLOW / PED RED - "
	));
	UART_SendNumber(auto_stage_time[3]);

	UART_SendString_P(PSTR("\r\nCurrent stage: "));
	UART_SendNumber((uint16_t)current_stage + 1);

	UART_SendString_P(PSTR(
	"\r\n\r\nNIGHT MODE:\r\n"
	"Yellow ON: "
	));
	UART_SendNumber(night_on_time);

	UART_SendString_P(PSTR(
	" s\r\nYellow OFF: "
	));
	UART_SendNumber(night_off_time);

	UART_SendString_P(PSTR(
	" s\r\n\r\nAUTO_NIGHT:\r\n"
	"Night start: "
	));

	UART_Send2(night_start_hour);
	UART_SendChar(':');
	UART_Send2(night_start_minute);
	UART_SendChar(':');
	UART_Send2(night_start_second);

	UART_SendString_P(PSTR("\r\nNight end: "));
	UART_Send2(night_end_hour);
	UART_SendChar(':');
	UART_Send2(night_end_minute);
	UART_SendChar(':');
	UART_Send2(night_end_second);

	UART_SendString_P(PSTR(
	"\r\n\r\nRTC: OK\r\n"
	"EEPROM: SETTINGS ACTIVE\r\n"
	"============================================\r\n"
	));
}

static void Command_SET_TIME(char *command)
{
	char *p = command;
	char *token;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	Get_Token(&p);
	token = Get_Token(&p);

	if (!token || !Parse_Time(token, &hour, &minute, &second))
	{
		UART_SendString_P(str_invalid_time);
		return;
	}

	if (!RTC_SetTime(hour, minute, second))
	{
		rtc_error = 1;
		Traffic_AllOff();
		Display_Off();
		UART_SendString_P(str_rtc_error);
		return;
	}

	rtc_error = 0;
	UART_SendString_P(str_ok_time);
}

static void Command_SET_DATE(char *command)
{
	char *p = command;
	char *token;
	uint8_t day;
	uint8_t month;
	uint8_t year;

	Get_Token(&p);
	token = Get_Token(&p);

	if (!token || !Parse_Date(token, &day, &month, &year))
	{
		UART_SendString_P(str_invalid_date);
		return;
	}

	if (!RTC_SetDate(day, month, year))
	{
		rtc_error = 1;
		Traffic_AllOff();
		Display_Off();
		UART_SendString_P(str_rtc_error);
		return;
	}

	rtc_error = 0;
	UART_SendString_P(str_ok_date);
}

static void Command_SET_MODE_AUTO(char *command)
{
	char *p = command;
	char *token;
	uint16_t new_time[4];
	uint8_t i;

	Get_Token(&p);

	for (i = 0; i < 4; i++)
	{
		token = Get_Token(&p);

		if (!token)
		{
			UART_SendString_P(str_expected_stages);
			return;
		}

		if (!Parse_Auto_Parameter(
		token, i + 1, &new_time[i]))
		{
			UART_SendString_P(str_invalid_auto);
			return;
		}
	}

	if (Get_Token(&p))
	{
		UART_SendString_P(str_too_many);
		return;
	}

	if (new_time[0] == 0 &&
	new_time[1] == 0 &&
	new_time[2] == 0 &&
	new_time[3] == 0)
	{
		UART_SendString_P(str_all_auto_zero);
		return;
	}

	for (i = 0; i < 4; i++)
	auto_stage_time[i] = new_time[i];

	EEPROM_Save();
	UART_SendString_P(str_ok_auto);
}

static void Command_SET_MODE_NIGHT(char *command)
{
	char *p = command;
	char *token;
	uint16_t on_time;
	uint16_t off_time;

	Get_Token(&p);

	token = Get_Token(&p);

	if (!token ||
	token[0] != 'O' ||
	token[1] != 'N' ||
	token[2] != '_')
	{
		UART_SendString_P(str_expected_on);
		return;
	}

	if (!Parse_XXX(&token[3], &on_time))
	{
		UART_SendString_P(str_invalid_on);
		return;
	}

	token = Get_Token(&p);

	if (!token ||
	token[0] != 'O' ||
	token[1] != 'F' ||
	token[2] != 'F' ||
	token[3] != '_')
	{
		UART_SendString_P(str_expected_off);
		return;
	}

	if (!Parse_XXX(&token[4], &off_time))
	{
		UART_SendString_P(str_invalid_off);
		return;
	}

	if (Get_Token(&p))
	{
		UART_SendString_P(str_too_many);
		return;
	}

	night_on_time = on_time;
	night_off_time = off_time;

	EEPROM_Save();
	UART_SendString_P(str_ok_night);
}

static void Command_SET_MODE_AUTO_NIGHT_TIME(char *command)
{
	char *p = command;
	char *token;

	uint8_t sh;
	uint8_t sm;
	uint8_t ss;

	uint8_t eh;
	uint8_t em;
	uint8_t es;

	Get_Token(&p);

	token = Get_Token(&p);

	if (!token || !Str_Equals(token, "ON"))
	{
		UART_SendString_P(str_expected_on_word);
		return;
	}

	token = Get_Token(&p);

	if (!token || !Parse_Time(token, &sh, &sm, &ss))
	{
		UART_SendString_P(str_invalid_night_start);
		return;
	}

	token = Get_Token(&p);

	if (!token || !Str_Equals(token, "OFF"))
	{
		UART_SendString_P(str_expected_off_word);
		return;
	}

	token = Get_Token(&p);

	if (!token || !Parse_Time(token, &eh, &em, &es))
	{
		UART_SendString_P(str_invalid_night_end);
		return;
	}

	if (Get_Token(&p))
	{
		UART_SendString_P(str_too_many);
		return;
	}

	night_start_hour = sh;
	night_start_minute = sm;
	night_start_second = ss;

	night_end_hour = eh;
	night_end_minute = em;
	night_end_second = es;

	EEPROM_Save();
	UART_SendString_P(str_ok_auto_night);
}

static void Command_MODE_AUTO(void)
{
	if (rtc_error)
	{
		UART_SendString_P(str_rtc_unavailable);
		return;
	}

	Set_Mode(MODE_AUTO);
	EEPROM_Save();
	UART_SendString_P(str_mode_auto);
}

static void Command_MODE_NIGHT(void)
{
	if (rtc_error)
	{
		UART_SendString_P(str_rtc_unavailable);
		return;
	}

	Set_Mode(MODE_NIGHT);
	EEPROM_Save();
	UART_SendString_P(str_mode_night);
}

static void Command_MODE_AUTO_NIGHT(void)
{
	if (rtc_error)
	{
		UART_SendString_P(str_rtc_unavailable);
		return;
	}

	Set_Mode(MODE_AUTO_NIGHT);
	EEPROM_Save();
	UART_SendString_P(str_mode_auto_night);
}

static void Command_MODE_OFF(void)
{
	Set_Mode(MODE_OFF);
	EEPROM_Save();
	UART_SendString_P(str_mode_off);
}

void Process_Command(void)
{
	uint8_t sreg = SREG;
	cli();
	uart_command_ready = 0;
	SREG = sreg;

	if (Str_Equals((char *)uart_buffer, "RESET"))
	{
		UART_SendString_P(str_ok_reset);
		MCU_Reset();
		return;
	}

	if (Str_Equals((char *)uart_buffer, "HELP"))
	{
		Command_HELP();
		uart_index = 0;
		return;
	}

	if (Str_Equals((char *)uart_buffer, "READ_STATUS"))
	{
		Command_READ_STATUS();
		uart_index = 0;
		return;
	}

	if (Str_Equals((char *)uart_buffer, "MODE_OFF"))
	{
		Command_MODE_OFF();
		uart_index = 0;
		return;
	}

	if (Str_Equals((char *)uart_buffer, "MODE_AUTO"))
	{
		Command_MODE_AUTO();
		uart_index = 0;
		return;
	}

	if (Str_Equals((char *)uart_buffer, "MODE_NIGHT"))
	{
		Command_MODE_NIGHT();
		uart_index = 0;
		return;
	}

	if (Str_Equals((char *)uart_buffer, "MODE_AUTO_NIGHT"))
	{
		Command_MODE_AUTO_NIGHT();
		uart_index = 0;
		return;
	}

	if (Str_StartsWith((char *)uart_buffer, "SET_TIME "))
	{
		Command_SET_TIME((char *)uart_buffer);
		uart_index = 0;
		return;
	}

	if (Str_StartsWith((char *)uart_buffer, "SET_DATE "))
	{
		Command_SET_DATE((char *)uart_buffer);
		uart_index = 0;
		return;
	}

	if (Str_StartsWith(
	(char *)uart_buffer,
	"SET_MODE_AUTO "))
	{
		Command_SET_MODE_AUTO((char *)uart_buffer);
		uart_index = 0;
		return;
	}

	if (Str_StartsWith(
	(char *)uart_buffer,
	"SET_MODE_NIGHT "))
	{
		Command_SET_MODE_NIGHT((char *)uart_buffer);
		uart_index = 0;
		return;
	}

	if (Str_StartsWith(
	(char *)uart_buffer,
	"SET_MODE_AUTO_NIGHT_TIME "))
	{
		Command_SET_MODE_AUTO_NIGHT_TIME(
		(char *)uart_buffer
		);
		uart_index = 0;
		return;
	}

	UART_SendString_P(str_unknown);
	uart_index = 0;
}

void UART_StartupMessage(void)
{
	UART_SendString_P(str_startup);
}

#include "config.h"
#include "rtc.h"
#include "twi.h"

#include <stdint.h>

RTC_Time rtc;
uint8_t rtc_error = 0;

static uint8_t BCD_To_Decimal(uint8_t value)
{
	return (uint8_t)(((value >> 4) * 10) + (value & 0x0F));
}

static uint8_t Decimal_To_BCD(uint8_t value)
{
	return (uint8_t)(((value / 10) << 4) | (value % 10));
}

uint32_t Time_To_Seconds(uint8_t hour, uint8_t minute, uint8_t second)
{
	return ((uint32_t)hour * 3600UL) +
	((uint32_t)minute * 60UL) + second;
}

uint32_t RTC_GetSecondsFromMidnight(const RTC_Time *time)
{
	return Time_To_Seconds(time->hours, time->minutes, time->seconds);
}

uint32_t Seconds_Difference(uint32_t now, uint32_t previous)
{
	if (now >= previous)
	return now - previous;

	return (86400UL - previous) + now;
}

static uint8_t RTC_WriteRegister(uint8_t reg, uint8_t value)
{
	if (!TWI_Start((RTC_ADDRESS << 1) | 0))
	{
		TWI_Stop();
		return 0;
	}

	if (!TWI_Write(reg))
	{
		TWI_Stop();
		return 0;
	}

	if (!TWI_Write(value))
	{
		TWI_Stop();
		return 0;
	}

	TWI_Stop();
	return 1;
}

uint8_t RTC_ReadTime(RTC_Time *time)
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t month;
	uint8_t year;

	if (!TWI_Start((RTC_ADDRESS << 1) | 0))
	{
		TWI_Stop();
		return 0;
	}

	if (!TWI_Write(0x00))
	{
		TWI_Stop();
		return 0;
	}

	if (!TWI_Start((RTC_ADDRESS << 1) | 1))
	{
		TWI_Stop();
		return 0;
	}

	seconds = TWI_ReadACK();
	minutes = TWI_ReadACK();
	hours   = TWI_ReadACK();

	TWI_ReadACK();

	day   = TWI_ReadACK();
	month = TWI_ReadACK();
	year  = TWI_ReadNACK();

	TWI_Stop();

	time->seconds = BCD_To_Decimal(seconds & 0x7F);
	time->minutes = BCD_To_Decimal(minutes & 0x7F);
	time->hours   = BCD_To_Decimal(hours & 0x3F);
	time->day     = BCD_To_Decimal(day);
	time->month   = BCD_To_Decimal(month & 0x1F);
	time->year    = BCD_To_Decimal(year);

	if (time->hours > 23 || time->minutes > 59 || time->seconds > 59 ||
	time->day < 1 || time->day > 31 ||
	time->month < 1 || time->month > 12)
	{
		return 0;
	}

	return 1;
}

uint8_t RTC_SetTime(uint8_t hour, uint8_t minute, uint8_t second)
{
	if (!RTC_WriteRegister(0x00, Decimal_To_BCD(second)))
	return 0;

	if (!RTC_WriteRegister(0x01, Decimal_To_BCD(minute)))
	return 0;

	if (!RTC_WriteRegister(0x02, Decimal_To_BCD(hour)))
	return 0;

	return 1;
}

uint8_t RTC_SetDate(uint8_t day, uint8_t month, uint8_t year)
{
	if (!RTC_WriteRegister(0x03, 1))
	return 0;

	if (!RTC_WriteRegister(0x04, Decimal_To_BCD(day)))
	return 0;

	if (!RTC_WriteRegister(0x05, Decimal_To_BCD(month)))
	return 0;

	if (!RTC_WriteRegister(0x06, Decimal_To_BCD(year)))
	return 0;

	return 1;
}

uint8_t RTC_Update(void)
{
	RTC_Time temp;

	if (!RTC_ReadTime(&temp))
	{
		rtc_error = 1;
		return 0;
	}

	rtc = temp;
	rtc_error = 0;
	return 1;
}

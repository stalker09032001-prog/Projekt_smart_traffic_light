#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include "types.h"

extern RTC_Time rtc;
extern uint8_t rtc_error;

uint8_t RTC_ReadTime(RTC_Time *time);
uint8_t RTC_SetTime(uint8_t hour, uint8_t minute, uint8_t second);
uint8_t RTC_SetDate(uint8_t day, uint8_t month, uint8_t year);
uint8_t RTC_Update(void);
uint32_t RTC_GetSecondsFromMidnight(const RTC_Time *time);
uint32_t Time_To_Seconds(uint8_t hour, uint8_t minute, uint8_t second);
uint32_t Seconds_Difference(uint32_t now, uint32_t previous);

#endif
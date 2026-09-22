#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/pgmspace.h>

extern volatile char uart_buffer[];
extern volatile uint8_t uart_index;
extern volatile uint8_t uart_command_ready;

extern const char str_reset_power[] PROGMEM;
extern const char str_reset_software[] PROGMEM;
extern const char str_rtc_error[] PROGMEM;
extern const char str_rtc_disabled[] PROGMEM;
extern const char str_rtc_ok[] PROGMEM;
extern const char str_ok_request[] PROGMEM;
extern const char str_request_ignored[] PROGMEM;

void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char *text);
void UART_SendString_P(const char *text);
void UART_SendNumber(uint16_t value);
void UART_Send2(uint8_t value);
void UART_StartupMessage(void);

void Process_Command(void);

#endif
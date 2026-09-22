#include "config.h"
#include "reset.h"
#include "traffic.h"
#include "display.h"
#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t reset_cause __attribute__((section(".noinit")));
volatile uint8_t software_reset_flag __attribute__((section(".noinit")));

void Reset_Early_Init(void) __attribute__((naked, used, section(".init3")));
void Reset_Early_Init(void)
{
	reset_cause = MCUSR;
	MCUSR = 0;

	WDTCSR = (1 << WDCE) | (1 << WDE);
	WDTCSR = 0;
}

void Reset_Report(void)
{
	if (software_reset_flag == 0xA5)
	{
		UART_SendString_P(str_reset_software);
		software_reset_flag = 0;
	}
	else
	{
		UART_SendString_P(str_reset_power);
	}
}

void MCU_Reset(void)
{
	Traffic_AllOff();
	Display_Off();

	software_reset_flag = 0xA5;

	UCSR0A |= (1 << TXC0);
	while (!(UCSR0A & (1 << TXC0)))
	{
	}

	cli();
	asm volatile ("jmp 0");

	while (1)
	{
	}
}

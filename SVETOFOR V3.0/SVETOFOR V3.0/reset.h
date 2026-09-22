#ifndef RESET_H
#define RESET_H

#include <stdint.h>

extern volatile uint8_t reset_cause;
extern volatile uint8_t software_reset_flag;

void Reset_Early_Init(void);
void Reset_Report(void);
void MCU_Reset(void) __attribute__((noreturn));

#endif
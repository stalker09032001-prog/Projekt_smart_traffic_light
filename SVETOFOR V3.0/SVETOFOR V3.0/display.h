#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void Display_DisableAll(void);
void Display_ClearSegments(void);
void Display_Off(void);
void Display_SetNumber(uint16_t number);
void Display_Refresh(void);
void Timer1_Init(void);

#endif
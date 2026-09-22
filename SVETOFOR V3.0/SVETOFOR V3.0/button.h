#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

extern volatile uint8_t button_event;

void Button_Init(void);
void Button_TimerTick(void);
void Process_Button_Event(void);

#endif

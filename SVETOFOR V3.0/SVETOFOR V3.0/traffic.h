#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <stdint.h>
#include "types.h"

void Traffic_AllOff(void);
void Traffic_Stage1(void);
void Traffic_Stage2(void);
void Traffic_Stage3(void);
void Traffic_Stage4(void);
void Traffic_Night(uint8_t yellow_on);

void Set_Mode(TrafficMode mode);

#endif
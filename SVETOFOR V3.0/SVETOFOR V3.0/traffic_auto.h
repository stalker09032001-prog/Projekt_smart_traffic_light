#ifndef TRAFFIC_AUTO_H
#define TRAFFIC_AUTO_H

#include <stdint.h>
#include "types.h"

extern AutoStage current_stage;
extern uint8_t pedestrian_request;
extern uint32_t pedestrian_request_time;
extern uint32_t stage_start_time;
extern uint8_t auto_state_initialized;

void Auto_EnterStage(AutoStage stage, uint32_t now);
void Auto_Initialize(uint32_t now);
void Process_Auto(void);

#endif
#ifndef TWI_H
#define TWI_H

#include <stdint.h>

void TWI_Init(void);
uint8_t TWI_Start(uint8_t address);
uint8_t TWI_Write(uint8_t data);
uint8_t TWI_ReadACK(void);
uint8_t TWI_ReadNACK(void);
void TWI_Stop(void);

#endif

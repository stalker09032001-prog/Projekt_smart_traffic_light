#include "config.h"
#include "twi.h"

#include <avr/io.h>

void TWI_Init(void)
{
	TWSR = 0;
	TWBR = (uint8_t)TWI_TWBR_VALUE;
	TWCR = (1 << TWEN);
}

uint8_t TWI_Start(uint8_t address)
{
	uint8_t status;

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

	while (!(TWCR & (1 << TWINT)))
	{
	}

	status = TWSR & 0xF8;
	if (status != 0x08 && status != 0x10)
	return 0;

	TWDR = address;
	TWCR = (1 << TWINT) | (1 << TWEN);

	while (!(TWCR & (1 << TWINT)))
	{
	}

	status = TWSR & 0xF8;

	if (address & 1)
	return status == 0x40;

	return status == 0x18;
}

uint8_t TWI_Write(uint8_t data)
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);

	while (!(TWCR & (1 << TWINT)))
	{
	}

	return ((TWSR & 0xF8) == 0x28);
}

uint8_t TWI_ReadACK(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

	while (!(TWCR & (1 << TWINT)))
	{
	}

	return TWDR;
}

uint8_t TWI_ReadNACK(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN);

	while (!(TWCR & (1 << TWINT)))
	{
	}

	return TWDR;
}

void TWI_Stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

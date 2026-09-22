#ifndef CONFIG_H
#define CONFIG_H

#define F_CPU 16000000UL

#define UART_BAUD       9600UL
#define UART_UBRR       ((F_CPU / (16UL * UART_BAUD)) - 1UL)

#define TWI_FREQUENCY   100000UL
#define TWI_TWBR_VALUE  ((F_CPU / TWI_FREQUENCY - 16UL) / 2UL)

#define RTC_ADDRESS     0x68
#define UART_BUFFER_SIZE 64

#define TIMER1_OCR_VALUE        832
#define PEDESTRIAN_DELAY_SEC    5
#define BUTTON_DEBOUNCE_TICKS   30

#define EEPROM_CONFIG_MAGIC     0x5346
#define EEPROM_CONFIG_VERSION   1

#define CAR_RED         PD3
#define CAR_YELLOW      PD4
#define CAR_GREEN       PD5
#define PED_RED         PD6
#define PED_GREEN       PD7

#define BUTTON          PD2

#define DIGIT_UNITS     PC0
#define DIGIT_TENS      PC1
#define DIGIT_HUNDREDS  PC2
#define SEGMENT_G       PC3

#endif

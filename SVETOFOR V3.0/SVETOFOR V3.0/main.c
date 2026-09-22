#include "config.h"
#include "types.h"
#include "gpio.h"
#include "uart.h"
#include "twi.h"
#include "rtc.h"
#include "eeprom_config.h"
#include "display.h"
#include "traffic.h"
#include "button.h"
#include "traffic_auto.h"
#include "traffic_night.h"
#include "reset.h"

#include <avr/interrupt.h>

int main(void)
{
    uint8_t eeprom_valid;

    GPIO_Init();
    UART_Init();
    TWI_Init();
    Timer1_Init();
    Button_Init();

    sei();

    UART_StartupMessage();
    Reset_Report();

    eeprom_valid = EEPROM_Load();

    if (eeprom_valid)
        UART_SendString_P(PSTR("EEPROM: SETTINGS LOADED\r\n"));
    else
        UART_SendString_P(
            PSTR("EEPROM: INVALID/EMPTY - DEFAULT SETTINGS LOADED\r\n")
        );

    if (!RTC_Update())
    {
        rtc_error = 1;
        Traffic_AllOff();
        Display_Off();

        UART_SendString_P(str_rtc_error);
        UART_SendString_P(str_rtc_disabled);
    }
    else
    {
        rtc_error = 0;

        Traffic_AllOff();
        Display_Off();

        UART_SendString_P(str_rtc_ok);

        Set_Mode(current_mode);
    }

    while (1)
    {
        if (uart_command_ready)
            Process_Command();

        if (!RTC_Update())
        {
            rtc_error = 1;
            Traffic_AllOff();
            Display_Off();
            continue;
        }

        rtc_error = 0;

        Process_Button_Event();

        switch (current_mode)
        {
            case MODE_OFF:
                Traffic_AllOff();
                Display_Off();
                break;

            case MODE_AUTO:
                Process_Auto();
                break;

            case MODE_NIGHT:
                Process_Night();
                break;

            case MODE_AUTO_NIGHT:
                Process_Auto_Night();
                break;

            default:
                Traffic_AllOff();
                Display_Off();
                break;
        }
    }
}

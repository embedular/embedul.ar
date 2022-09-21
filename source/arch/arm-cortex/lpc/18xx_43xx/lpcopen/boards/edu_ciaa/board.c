/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpcopen board - EDU-CIAA-NXP + RETRO-CIAA Poncho.

  Copyright 2018-2022 Santiago Germino
  <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/edu_ciaa/board.h"


// CHIP module configuration. EDU-CIAA clock is based on a 12 Mhz crystal.
const   uint32_t        ExtRateIn       = 0;
const   uint32_t        OscRateIn       = 12000000;


// Cortex-M specific interrupt handler
CC_Weak void HardFault_Handler (void)
{
    while (1)
    {
        BOARD_LED_TOGGLE (M4_HARDFAULT);
        for (uint32_t i = 0; i < (SystemCoreClock / 20); ++i);
    }
}


#ifdef BOARD_USE_ADC
static  ADC_CHANNEL_T curADCChannel = ADC_CH4;
#endif

// -----------------------------------------------------------------------------
// This board code does not fully adhere to LPCOpen board_api.h
// -----------------------------------------------------------------------------
// Board_LED_Set, Board_LED_Test, Board_LED_Toggle:
//      Use BOARD_LED_ON/OFF/TOGGLE/GET_STATUS macros instead.
//
// Board_Debug_Init, Board_UARTGetChar, Board_UARTPutSTR:
//      Not implemented.
// -----------------------------------------------------------------------------


void Board_UARTPutChar (char ch)
{
    while (!(Chip_UART_ReadLineStatus(BOARD_DEBUG_INTERFACE) & UART_LSR_THRE))
    {
        Chip_UART_SendByte(BOARD_DEBUG_INTERFACE, (uint8_t) ch);
    }
}


void Board_UARTPutSTR (const char *str)
{
    while (*str != '\0') 
    {
        Board_UARTPutChar (*str++);
    }
}


void Board_SetLCDBacklight (uint8_t Intensity)
{
    #ifndef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    (void) Intensity;
    #else
    if (Intensity)
    {
        BOARD_LED_ON (Y);
        BOARD_LED_ON (Z);
    }
    else
    {
        BOARD_LED_OFF (Y);
        BOARD_LED_OFF (Z);
    }
    #endif
}


#ifdef BOARD_USE_ADC
void Board_ADC_ReadBegin (ADC_CHANNEL_T channel)
{
    if (channel > ADC_CH3)
    {
        return;
    }

    curADCChannel = channel;

    Chip_ADC_EnableChannel      (LPC_ADC0, curADCChannel, DISABLE);
    Chip_SCU_ADC_Channel_Config (0, (uint8_t)curADCChannel);
    Chip_ADC_EnableChannel      (LPC_ADC0, curADCChannel, ENABLE);
    Chip_ADC_SetBurstCmd        (LPC_ADC0, DISABLE);
    Chip_ADC_SetStartMode       (LPC_ADC0, ADC_START_NOW,
                                 ADC_TRIGGERMODE_RISING);
}


bool Board_ADC_ReadWait (void)
{
    if (curADCChannel > ADC_CH3)
    {
        return false;
    }

    return (Chip_ADC_ReadStatus (LPC_ADC0, (uint8_t)curADCChannel,
                                 ADC_DR_DONE_STAT) == RESET);
}


uint16_t Board_ADC_ReadEnd (void)
{
    if (curADCChannel > ADC_CH3)
    {
        return (uint16_t) -1;
    }

    uint16_t data;

    if (Chip_ADC_ReadValue (LPC_ADC0, (uint8_t)curADCChannel, &data)
        != SUCCESS)
    {
        data = (uint16_t) -1;
    }

    Chip_ADC_EnableChannel (LPC_ADC0, curADCChannel, DISABLE);
    curADCChannel = ADC_CH4;

    return data;
}
#endif


// Internal board initialization functions

static void initPeripherals (void)
{
    Chip_I2C_Init           (BOARD_I2C_ID);
    Chip_SCU_I2C0PinConfig  (BOARD_I2C_MODE);
    Chip_I2C_SetClockRate   (BOARD_I2C_ID, BOARD_I2C_SPEED);

    Chip_SSP_Init           (BOARD_SPI_INTERFACE);
    Chip_SSP_Set_Mode       (BOARD_SPI_INTERFACE, BOARD_SPI_MODE);
    Chip_SSP_SetFormat      (BOARD_SPI_INTERFACE, BOARD_SPI_BITS,
                             BOARD_SPI_FORMAT, BOARD_SPI_POLARITY);
    Chip_SSP_SetBitRate     (BOARD_SPI_INTERFACE, BOARD_SPI_SPEED);
    Chip_SSP_Enable         (BOARD_SPI_INTERFACE);

    Chip_UART_Init          (BOARD_DEBUG_INTERFACE);
    Chip_UART_SetBaudFDR    (BOARD_DEBUG_INTERFACE, BOARD_DEBUG_BAUD_RATE);
    Chip_UART_ConfigData    (BOARD_DEBUG_INTERFACE, BOARD_DEBUG_CONFIG);
    Chip_UART_TXEnable      (BOARD_DEBUG_INTERFACE);

    Chip_UART_Init          (BOARD_EXT_USART_INTERFACE);
    Chip_UART_SetBaudFDR    (BOARD_EXT_USART_INTERFACE,
                             BOARD_EXT_USART_BAUD_RATE);
    Chip_UART_ConfigData    (BOARD_EXT_USART_INTERFACE, BOARD_EXT_USART_CONFIG);
    Chip_UART_TXEnable      (BOARD_EXT_USART_INTERFACE);

    #ifdef BOARD_USE_ADC
    ADC_CLOCK_SETUP_T cs;

    Chip_ADC_Init           (LPC_ADC0, &cs);
    Chip_ADC_SetSampleRate  (LPC_ADC0, &cs, BOARD_ADC_SAMPLE_RATE);
    Chip_ADC_SetResolution  (LPC_ADC0, &cs, BOARD_ADC_RESOLUTION);
    #endif

    #ifdef BOARD_USE_RTC
    RTC_TIME_T rtcTime;
    Chip_RTC_Init (LPC_RTC);

    /* Set current time for RTC */
    /* Current time is 8:00:00PM, 2013-01-31 */
    rtcTime.time[RTC_TIMETYPE_SECOND]     = 0;
    rtcTime.time[RTC_TIMETYPE_MINUTE]     = 0;
    rtcTime.time[RTC_TIMETYPE_HOUR]       = 20;
    rtcTime.time[RTC_TIMETYPE_DAYOFMONTH] = 31;
    rtcTime.time[RTC_TIMETYPE_MONTH]      = 1;
    rtcTime.time[RTC_TIMETYPE_YEAR]       = 2013;
    Chip_RTC_SetFullAlarmTime (LPC_RTC, &rtcTime);

    /* Enable rtc (starts increasing the tick counter and second counter register) */
    Chip_RTC_Enable (LPC_RTC, ENABLE);
    #endif

    #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    // Enable pin buffers
    BOARD_GPIO_SET_STATE (OUT_SYNC_JOY_OE, ENABLED);
    BOARD_GPIO_SET_STATE (OUT_RGB_OE, ENABLED);
    #endif
}


// Standard API board init
void Board_Init (void)
{
    Chip_GPIO_Init (LPC_GPIO_PORT);

    initPeripherals ();

    Board_SetLCDBacklight (255);
}

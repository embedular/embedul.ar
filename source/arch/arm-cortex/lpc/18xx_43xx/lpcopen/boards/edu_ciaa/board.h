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

#pragma once

// LPCOpen chip functions and CMSIS
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/chip_18xx_43xx/chip.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/api.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/gpio.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/seed.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/i2cm.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/panic.h"


// GPIO definitions
#define BOARD_GPIO_OUT_LED_RGB_RED_PORT     5
#define BOARD_GPIO_OUT_LED_RGB_RED_PIN      0
#define BOARD_GPIO_OUT_LED_RGB_RED_EN       BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_LED_RGB_GREEN_PORT   5
#define BOARD_GPIO_OUT_LED_RGB_GREEN_PIN    1
#define BOARD_GPIO_OUT_LED_RGB_GREEN_EN     BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_LED_RGB_BLUE_PORT    5
#define BOARD_GPIO_OUT_LED_RGB_BLUE_PIN     2
#define BOARD_GPIO_OUT_LED_RGB_BLUE_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_LED_1_PORT           0
#define BOARD_GPIO_OUT_LED_1_PIN            14
#define BOARD_GPIO_OUT_LED_1_EN             BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_LED_2_PORT           1
#define BOARD_GPIO_OUT_LED_2_PIN            11
#define BOARD_GPIO_OUT_LED_2_EN             BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_LED_3_PORT           1
#define BOARD_GPIO_OUT_LED_3_PIN            12
#define BOARD_GPIO_OUT_LED_3_EN             BOARD_GPIO_HIGH

#define BOARD_GPIO_IN_SWITCH_TEC_1_PORT     0
#define BOARD_GPIO_IN_SWITCH_TEC_1_PIN      4
#define BOARD_GPIO_IN_SWITCH_TEC_1_EN       BOARD_GPIO_LOW

#define BOARD_GPIO_IN_SWITCH_TEC_2_PORT     0
#define BOARD_GPIO_IN_SWITCH_TEC_2_PIN      8
#define BOARD_GPIO_IN_SWITCH_TEC_2_EN       BOARD_GPIO_LOW

#define BOARD_GPIO_IN_SWITCH_TEC_3_PORT     0
#define BOARD_GPIO_IN_SWITCH_TEC_3_PIN      9
#define BOARD_GPIO_IN_SWITCH_TEC_3_EN       BOARD_GPIO_LOW

#define BOARD_GPIO_IN_SWITCH_TEC_4_PORT     1
#define BOARD_GPIO_IN_SWITCH_TEC_4_PIN      9
#define BOARD_GPIO_IN_SWITCH_TEC_4_EN       BOARD_GPIO_LOW

#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
#define BOARD_GPIO_OUT_LED_Z_PORT           5
#define BOARD_GPIO_OUT_LED_Z_PIN            15
#define BOARD_GPIO_OUT_LED_Z_EN             BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_LED_Y_PORT           5
#define BOARD_GPIO_OUT_LED_Y_PIN            16
#define BOARD_GPIO_OUT_LED_Y_EN             BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_SYNC_JOY_OE_PORT     5
#define BOARD_GPIO_OUT_SYNC_JOY_OE_PIN      13
#define BOARD_GPIO_OUT_SYNC_JOY_OE_EN       BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_RGB_OE_PORT          1
#define BOARD_GPIO_OUT_RGB_OE_PIN           8
#define BOARD_GPIO_OUT_RGB_OE_EN            BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_WIFI_EN_PORT         3
#define BOARD_GPIO_OUT_WIFI_EN_PIN          12
#define BOARD_GPIO_OUT_WIFI_EN_EN           BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_EDDC_EN_PORT         3
#define BOARD_GPIO_OUT_EDDC_EN_PIN          13
#define BOARD_GPIO_OUT_EDDC_EN_EN           BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_JOY_LATCH_PORT       3
#define BOARD_GPIO_OUT_JOY_LATCH_PIN        3
#define BOARD_GPIO_OUT_JOY_LATCH_EN         BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_JOY_CLOCK_PORT       3
#define BOARD_GPIO_OUT_JOY_CLOCK_PIN        6
#define BOARD_GPIO_OUT_JOY_CLOCK_EN         BOARD_GPIO_HIGH

#define BOARD_GPIO_IN_JOY1_DATA_PORT        3
#define BOARD_GPIO_IN_JOY1_DATA_PIN         5
#define BOARD_GPIO_IN_JOY1_DATA_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_IN_JOY2_DATA_PORT        3
#define BOARD_GPIO_IN_JOY2_DATA_PIN         7
#define BOARD_GPIO_IN_JOY2_DATA_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_SOUND_MUTE_PORT      3
#define BOARD_GPIO_OUT_SOUND_MUTE_PIN       15
#define BOARD_GPIO_OUT_SOUND_MUTE_EN        BOARD_GPIO_LOW

// For retrociaa-poncho v1.1.1 and v1.0.2 patched with sdcard reader pcb:
// SPI-CS1 is permanently connected to sdcard select.
#define BOARD_GPIO_OUT_SD_SELECT_PORT       0
#define BOARD_GPIO_OUT_SD_SELECT_PIN        12
#define BOARD_GPIO_OUT_SD_SELECT_EN         BOARD_GPIO_LOW

// For retrociaa-poncho v1.1.1 and v1.0.2 patched with sdcard reader pcb:
// SPI-CS2 is permanently connected to the sdcard presence switch.
#define BOARD_GPIO_IN_SWITCH_SD_DETECT_PORT 0
#define BOARD_GPIO_IN_SWITCH_SD_DETECT_PIN  13
#define BOARD_GPIO_IN_SWITCH_SD_DETECT_EN   BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_SPI_CS3_PORT         0
#define BOARD_GPIO_OUT_SPI_CS3_PIN          15
#define BOARD_GPIO_OUT_SPI_CS3_EN           BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_VSYNC_PORT           5
#define BOARD_GPIO_OUT_VSYNC_PIN            14
#define BOARD_GPIO_OUT_VSYNC_EN             BOARD_GPIO_LOW   // driver dependant

#define BOARD_GPIO_OUT_HSYNC_PORT           5
#define BOARD_GPIO_OUT_HSYNC_PIN            12
#define BOARD_GPIO_OUT_HSYNC_EN             BOARD_GPIO_LOW   // driver dependant

#define BOARD_GPIO_OUT_PIXEL_R2_PORT        2
#define BOARD_GPIO_OUT_PIXEL_R2_PIN         8
#define BOARD_GPIO_OUT_PIXEL_R2_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_R1_PORT        2
#define BOARD_GPIO_OUT_PIXEL_R1_PIN         6
#define BOARD_GPIO_OUT_PIXEL_R1_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_R0_PORT        2
#define BOARD_GPIO_OUT_PIXEL_R0_PIN         5
#define BOARD_GPIO_OUT_PIXEL_R0_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_G2_PORT        2
#define BOARD_GPIO_OUT_PIXEL_G2_PIN         4
#define BOARD_GPIO_OUT_PIXEL_G2_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_G1_PORT        2
#define BOARD_GPIO_OUT_PIXEL_G1_PIN         3
#define BOARD_GPIO_OUT_PIXEL_G1_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_G0_PORT        2
#define BOARD_GPIO_OUT_PIXEL_G0_PIN         2
#define BOARD_GPIO_OUT_PIXEL_G0_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_B1_PORT        2
#define BOARD_GPIO_OUT_PIXEL_B1_PIN         1
#define BOARD_GPIO_OUT_PIXEL_B1_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_B0_PORT        2
#define BOARD_GPIO_OUT_PIXEL_B0_PIN         0
#define BOARD_GPIO_OUT_PIXEL_B0_EN          BOARD_GPIO_HIGH
#endif // #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO

// Local (mainboard) I2C interfaces and device 7-bit addresses
#define BOARD_I2C_INTERFACE                 LPC_I2C0
#define BOARD_I2C_ID                        I2C0
#define BOARD_SPI_INTERFACE                 LPC_SSP1

// I2S
#define BOARD_I2S_INTERFACE                 LPC_I2S1
#define BOARD_I2S_GPDMA_CONN_TX             GPDMA_CONN_I2S1_Tx_Channel_0

#define BOARD_RS485_INTERFACE               LPC_USART0
#define BOARD_RS485_UART_IRQ                USART0_IRQn
#define BOARD_RS485_UART_IRQHANDLER         UART0_IRQHandler

#define BOARD_DEBUG_INTERFACE               LPC_USART2
#define BOARD_DEBUG_IRQ                     USART2_IRQn
#define BOARD_DEBUG_IRQHANDLER              UART2_IRQHandler

#define BOARD_EXT_USART_INTERFACE           LPC_USART3
#define BOARD_EXT_USART_IRQ                 USART3_IRQn
#define BOARD_EXT_USART_IRQHANDLER          UART3_IRQHandler


// I2C defaults to Standard/Fast mode, 400 Khz.
#ifndef BOARD_I2C_MODE
    #define BOARD_I2C_MODE                  I2C0_STANDARD_FAST_MODE
#endif

#ifndef BOARD_I2C_SPEED
    #define BOARD_I2C_SPEED                 400000
#endif


// SPI default config: Master, 8 Bits, SPI format, CPHA0/CPOL0 polarity.
#ifndef BOARD_SPI_MODE
    #define BOARD_SPI_MODE                  SSP_MODE_MASTER
#endif

#ifndef BOARD_SPI_BITS
    #define BOARD_SPI_BITS                  SSP_BITS_8
#endif

#ifndef BOARD_SPI_FORMAT
    #define BOARD_SPI_FORMAT                SSP_FRAMEFORMAT_SPI
#endif

#ifndef BOARD_SPI_POLARITY
    #define BOARD_SPI_POLARITY              SSP_CLOCK_CPHA0_CPOL0
#endif

#ifndef BOARD_SPI_SPEED
    #define BOARD_SPI_SPEED                 100000
#endif


// Debug UART defaults: 115200, 8N1.
#ifndef BOARD_DEBUG_BAUD_RATE
    #define BOARD_DEBUG_BAUD_RATE           115200
#endif

#ifndef BOARD_DEBUG_DATA_BITS
    #define BOARD_DEBUG_DATA_BITS           UART_LCR_WLEN8
#endif

#ifndef BOARD_DEBUG_PARITY
    #define BOARD_DEBUG_PARITY              UART_LCR_PARITY_DIS
#endif

#ifndef BOARD_DEBUG_STOP_BITS
    #define BOARD_DEBUG_STOP_BITS           UART_LCR_SBS_1BIT
#endif

#define BOARD_DEBUG_CONFIG                  (BOARD_DEBUG_DATA_BITS | \
                                                BOARD_DEBUG_PARITY | \
                                                BOARD_DEBUG_STOP_BITS)

// Expansion header UART settings
#define BOARD_EXT_USART_BAUD_RATE           115200
#define BOARD_EXT_USART_DATA_BITS           UART_LCR_WLEN8
#define BOARD_EXT_USART_PARITY              UART_LCR_PARITY_DIS
#define BOARD_EXT_USART_STOP_BITS           UART_LCR_SBS_1BIT
#define BOARD_EXT_USART_CONFIG              (BOARD_EXT_USART_DATA_BITS \
                                                | BOARD_EXT_USART_PARITY \
                                                | BOARD_EXT_USART_STOP_BITS)


// ADC maximum sampling rate: (4.5 Mhz / 11 bits) = ~400 Khz.
#ifndef BOARD_ADC_SAMPLE_RATE
    #define BOARD_ADC_SAMPLE_RATE           400000
#endif

#ifndef BOARD_ADC_RESOLUTION
    #define BOARD_ADC_RESOLUTION            ADC_10BITS
#endif


#ifdef BOARD_USE_ADC
void        Board_ADC_ReadBegin     (ADC_CHANNEL_T channel);
bool        Board_ADC_ReadWait      (void);
uint16_t    Board_ADC_ReadEnd       (void);
#endif


// LED used to signal an M4 Core (application) Hardfault
#define BOARD_GPIO_OUT_LED_M4_HARDFAULT_PORT    BOARD_GPIO_OUT_LED_1_PORT
#define BOARD_GPIO_OUT_LED_M4_HARDFAULT_PIN     BOARD_GPIO_OUT_LED_1_PIN
#define BOARD_GPIO_OUT_LED_M4_HARDFAULT_EN      BOARD_GPIO_OUT_LED_1_EN

// LED used to signal an M0 Core (video adapter) Hardfault
#define BOARD_GPIO_OUT_LED_M0_HARDFAULT_PORT    BOARD_GPIO_OUT_LED_2_PORT
#define BOARD_GPIO_OUT_LED_M0_HARDFAULT_PIN     BOARD_GPIO_OUT_LED_2_PIN
#define BOARD_GPIO_OUT_LED_M0_HARDFAULT_EN      BOARD_GPIO_OUT_LED_2_EN


// Default I2CM interface
#define BOARD_I2CM_TRANSFER_INTERFACE       BOARD_I2C_INTERFACE

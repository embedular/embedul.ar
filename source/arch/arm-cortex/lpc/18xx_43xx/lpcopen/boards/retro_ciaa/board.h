/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpcopen board - RETRO-CIAA standalone.

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
#define BOARD_GPIO_OUT_LED_WARN_PORT        3
#define BOARD_GPIO_OUT_LED_WARN_PIN         11
#define BOARD_GPIO_OUT_LED_WARN_EN          BOARD_GPIO_LOW

#define BOARD_GPIO_IN_SWITCH_ISP_PORT       0
#define BOARD_GPIO_IN_SWITCH_ISP_PIN        7
#define BOARD_GPIO_IN_SWITCH_ISP_EN         BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_SD_POW_PORT          1
#define BOARD_GPIO_OUT_SD_POW_PIN           8
#define BOARD_GPIO_OUT_SD_POW_EN            BOARD_GPIO_LOW

#define BOARD_GPIO_IO_STACKPORT_IO1_PORT    5
#define BOARD_GPIO_IO_STACKPORT_IO1_PIN     12
#define BOARD_GPIO_IO_STACKPORT_IO1_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_IO_STACKPORT_IO2_PORT    5
#define BOARD_GPIO_IO_STACKPORT_IO2_PIN     13
#define BOARD_GPIO_IO_STACKPORT_IO2_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_IO_STACKPORT_IO3_PORT    5
#define BOARD_GPIO_IO_STACKPORT_IO3_PIN     14
#define BOARD_GPIO_IO_STACKPORT_IO3_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_IO_STACKPORT_IO4_PORT    2
#define BOARD_GPIO_IO_STACKPORT_IO4_PIN     0
#define BOARD_GPIO_IO_STACKPORT_IO4_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_IO_STACKPORT_IO5_PORT    2
#define BOARD_GPIO_IO_STACKPORT_IO5_PIN     3
#define BOARD_GPIO_IO_STACKPORT_IO5_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_IO_STACKPORT_IO6_PORT    2
#define BOARD_GPIO_IO_STACKPORT_IO6_PIN     5
#define BOARD_GPIO_IO_STACKPORT_IO6_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_IO_STACKPORT_IO7_PORT    2
#define BOARD_GPIO_IO_STACKPORT_IO7_PIN     6
#define BOARD_GPIO_IO_STACKPORT_IO7_EN      BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_SOUND_MUTE_PORT      3
#define BOARD_GPIO_OUT_SOUND_MUTE_PIN       8
#define BOARD_GPIO_OUT_SOUND_MUTE_EN        BOARD_GPIO_LOW

// MCU: P5_4, U1_CTS, GPIO2[13] - WIFI: IO14(MTDI), RTS
#define BOARD_GPIO_OUT_WIFI_IO14_PORT       2
#define BOARD_GPIO_OUT_WIFI_IO14_PIN        13
#define BOARD_GPIO_OUT_WIFI_IO14_EN         BOARD_GPIO_LOW

// MCU: P5_2, U1_RTS, GPIO2[11] - WIFI: IO15(MTDI), CTS
// WIFI, Low: silences boot messages printed by the ROM bootloader
// (weak pull-up)
#define BOARD_GPIO_OUT_WIFI_IO15_PORT       2
#define BOARD_GPIO_OUT_WIFI_IO15_PIN        11
#define BOARD_GPIO_OUT_WIFI_IO15_EN         BOARD_GPIO_LOW

#define BOARD_GPIO_OUT_WIFI_EN_PORT         5
#define BOARD_GPIO_OUT_WIFI_EN_PIN          18
#define BOARD_GPIO_OUT_WIFI_EN_EN           BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_DVI_EN_PORT          4
#define BOARD_GPIO_OUT_DVI_EN_PIN           11
#define BOARD_GPIO_OUT_DVI_EN_EN            BOARD_GPIO_HIGH

// Cortex-M0 video driver pinout
#define BOARD_GPIO_OUT_HSYNC_PORT           5
#define BOARD_GPIO_OUT_HSYNC_PIN            2
#define BOARD_GPIO_OUT_HSYNC_EN             BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_VSYNC_PORT           0
#define BOARD_GPIO_OUT_VSYNC_PIN            5
#define BOARD_GPIO_OUT_VSYNC_EN             BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_RGB_EN_PORT          5
#define BOARD_GPIO_OUT_RGB_EN_PIN           15
#define BOARD_GPIO_OUT_RGB_EN_EN            BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_R2_PORT        3
#define BOARD_GPIO_OUT_PIXEL_R2_PIN         7
#define BOARD_GPIO_OUT_PIXEL_R2_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_R1_PORT        3
#define BOARD_GPIO_OUT_PIXEL_R1_PIN         6
#define BOARD_GPIO_OUT_PIXEL_R1_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_R0_PORT        3
#define BOARD_GPIO_OUT_PIXEL_R0_PIN         5
#define BOARD_GPIO_OUT_PIXEL_R0_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_G2_PORT        3
#define BOARD_GPIO_OUT_PIXEL_G2_PIN         4
#define BOARD_GPIO_OUT_PIXEL_G2_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_G1_PORT        3
#define BOARD_GPIO_OUT_PIXEL_G1_PIN         3
#define BOARD_GPIO_OUT_PIXEL_G1_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_G0_PORT        3
#define BOARD_GPIO_OUT_PIXEL_G0_PIN         2
#define BOARD_GPIO_OUT_PIXEL_G0_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_B1_PORT        3
#define BOARD_GPIO_OUT_PIXEL_B1_PIN         1
#define BOARD_GPIO_OUT_PIXEL_B1_EN          BOARD_GPIO_HIGH

#define BOARD_GPIO_OUT_PIXEL_B0_PORT        3
#define BOARD_GPIO_OUT_PIXEL_B0_PIN         0
#define BOARD_GPIO_OUT_PIXEL_B0_EN          BOARD_GPIO_HIGH

// Local (mainboard) I2C interfaces and device 7-bit addresses
#define BOARD_I2C_INTERFACE                 LPC_I2C1
#define BOARD_I2C_ID                        I2C1
#define BOARD_I2C_SLAVE_DVI_TRANSMITTER     0x38
#define BOARD_I2C_SLAVE_EEPROM_BANK1        0x56
#define BOARD_I2C_SLAVE_EEPROM_BANK2        0x57

// I2S
#define BOARD_I2S_INTERFACE                 LPC_I2S0
#define BOARD_I2S_GPDMA_CONN_TX             GPDMA_CONN_I2S_Tx_Channel_0

#define BOARD_DEBUG_INTERFACE               LPC_USART0
#define BOARD_DEBUG_IRQ                     USART0_IRQn
#define BOARD_DEBUG_IRQHANDLER              UART0_IRQHandler

#define BOARD_ESP_INTERFACE                 LPC_UART1
#define BOARD_ESP_IRQ                       UART1_IRQn
#define BOARD_ESP_IRQHANDLER                UART1_IRQHandler

// Local I2C
#ifndef BOARD_I2C_SPEED
    #define BOARD_I2C_SPEED                 400000
#endif

// Debug UART defaults: 115200, 8N1
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

// Fixed ESP module UART settings
#define BOARD_ESP_BAUD_RATE                 115200
#define BOARD_ESP_DATA_BITS                 UART_LCR_WLEN8
#define BOARD_ESP_PARITY                    UART_LCR_PARITY_DIS
#define BOARD_ESP_STOP_BITS                 UART_LCR_SBS_1BIT
#define BOARD_ESP_CONFIG                    (BOARD_ESP_DATA_BITS \
                                                | BOARD_ESP_PARITY \
                                                | BOARD_ESP_STOP_BITS)


#define BOARD_SP_GENESIS                    0x01
#define BOARD_SP_SSL                        0x02
#define BOARD_SP_ALL                        ((uint32_t) -1)
// -----------------------------------------------------------------------------
// StackPort interfaces
// -----------------------------------------------------------------------------
#define BOARD_STACKPORT_I2C_INTERFACE       LPC_I2C0
#define BOARD_STACKPORT_I2C_ID              I2C0

#define BOARD_STACKPORT_SPI_INTERFACE       LPC_SSP0

#define BOARD_STACKPORT_UART_INTERFACE      LPC_USART2
#define BOARD_STACKPORT_UART_IRQ            USART2_IRQn
#define BOARD_STACKPORT_UART_IRQHANDLER     UART2_IRQHandler

#define BOARD_STACKPORT_EIA485_INTERFACE    LPC_USART3
#define BOARD_STACKPORT_EIA485_IRQ          USART3_IRQn
#define BOARD_STACKPORT_EIA485_IRQHANDLER   UART3_IRQHandler

// StackPort I2C defaults to Standard/Fast mode, 400 Khz
#ifndef BOARD_STACKPORT_I2C_MODE
    #define BOARD_STACKPORT_I2C_MODE        I2C0_STANDARD_FAST_MODE
#endif

#ifndef BOARD_STACKPORT_I2C_SPEED
    #define BOARD_STACKPORT_I2C_SPEED       400000
#endif

// StackPort SPI default config: Master, 8 Bits, SPI format, CPHA0/CPOL0 polarity
#ifndef BOARD_STACKPORT_SPI_MODE
    #define BOARD_STACKPORT_SPI_MODE        SSP_MODE_MASTER
#endif

#ifndef BOARD_STACKPORT_SPI_BITS
    #define BOARD_STACKPORT_SPI_BITS        SSP_BITS_8
#endif

#ifndef BOARD_STACKPORT_SPI_FORMAT
    #define BOARD_STACKPORT_SPI_FORMAT      SSP_FRAMEFORMAT_SPI
#endif

#ifndef BOARD_STACKPORT_SPI_POLARITY
    #define BOARD_STACKPORT_SPI_POLARITY    SSP_CLOCK_CPHA0_CPOL0
#endif

#ifndef BOARD_STACKPORT_SPI_SPEED
    #define BOARD_STACKPORT_SPI_SPEED       100000
#endif

// StackPort UART defaults: 115200, 8N1
#ifndef BOARD_STACKPORT_UART_BAUD_RATE
    #define BOARD_STACKPORT_UART_BAUD_RATE  115200
#endif

#ifndef BOARD_STACKPORT_UART_DATA_BITS
    #define BOARD_STACKPORT_UART_DATA_BITS  UART_LCR_WLEN8
#endif

#ifndef BOARD_STACKPORT_UART_PARITY
    #define BOARD_STACKPORT_UART_PARITY     UART_LCR_PARITY_DIS
#endif

#ifndef BOARD_STACKPORT_UART_STOP_BITS
    #define BOARD_STACKPORT_UART_STOP_BITS  UART_LCR_SBS_1BIT
#endif

#define BOARD_STACKPORT_UART_CONFIG         (BOARD_STACKPORT_UART_DATA_BITS | \
                                                BOARD_STACKPORT_UART_PARITY | \
                                                BOARD_STACKPORT_UART_STOP_BITS)

// StackPort "Genesis" module
#define BOARD_SP_GENESIS_I2C_ADDR_PCA9673   0x24
#define BOARD_SP_GENESIS_I2C_ADDR_LP5036    0x30

#define BOARD_GPIO_SP_GENESIS_LP5036_PORT   BOARD_GPIO_IO_STACKPORT_IO1_PORT
#define BOARD_GPIO_SP_GENESIS_LP5036_PIN    BOARD_GPIO_IO_STACKPORT_IO1_PIN
#define BOARD_GPIO_SP_GENESIS_LP5036_EN     BOARD_GPIO_HIGH

// StackPort "SSL" module
#define BOARD_SP_SSL_I2C_ADDR_PCA9956B_DA   0x35
#define BOARD_SP_SSL_I2C_ADDR_PCA9956B_DB   0x33

#define BOARD_GPIO_SP_SSL_BACKLIGHT_PORT    BOARD_GPIO_IO_STACKPORT_IO2_PORT
#define BOARD_GPIO_SP_SSL_BACKLIGHT_PIN     BOARD_GPIO_IO_STACKPORT_IO2_PIN
#define BOARD_GPIO_SP_SSL_BACKLIGHT_EN      BOARD_GPIO_LOW


// LED used to signal an M4 Core (application) Hardfault
#define BOARD_GPIO_OUT_LED_M4_HARDFAULT_PORT    BOARD_GPIO_OUT_LED_WARN_PORT
#define BOARD_GPIO_OUT_LED_M4_HARDFAULT_PIN     BOARD_GPIO_OUT_LED_WARN_PIN
#define BOARD_GPIO_OUT_LED_M4_HARDFAULT_EN      BOARD_GPIO_OUT_LED_WARN_EN

// LED used to signal an M0 Core (video adapter) Hardfault
#define BOARD_GPIO_OUT_LED_M0_HARDFAULT_PORT    BOARD_GPIO_OUT_LED_WARN_PORT
#define BOARD_GPIO_OUT_LED_M0_HARDFAULT_PIN     BOARD_GPIO_OUT_LED_WARN_PIN
#define BOARD_GPIO_OUT_LED_M0_HARDFAULT_EN      BOARD_GPIO_OUT_LED_WARN_EN


// Default I2CM interface
#define BOARD_I2CM_TRANSFER_INTERFACE       BOARD_STACKPORT_I2C_INTERFACE


bool Board_DetectedStackPortModules (uint32_t mask);
void Board_SetSslModuleBacklight (bool on);
bool Board_GetSslModuleBacklight (void);
void Board_SetSdPower (bool on);
bool Board_GetSdPower (void);


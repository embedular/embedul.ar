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

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/retro_ciaa/board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/sysinit_util.h"


// CHIP module configuration. retrociaa clock is based on a 12 Mhz crystal.
const   uint32_t        ExtRateIn           = 0;
const   uint32_t        OscRateIn           = 12000000;

// Stack port modules detected and initialized
static  uint32_t        s_StackPortModules  = 0;


// Cortex-M specific interrupt handler
CC_Weak void HardFault_Handler (void)
{
    while (1)
    {
        BOARD_LED_TOGGLE (M4_HARDFAULT);
        for (uint32_t i = 0; i < (SystemCoreClock / 20); ++i);
    }
}


// -----------------------------------------------------------------------------
// This board code does not fully adhere to LPCOpen board_api.h
// -----------------------------------------------------------------------------
// Board_LED_Set, Board_LED_Test, Board_LED_Toggle:
//      Use BOARD_LED_ON/OFF/TOGGLE/GET_STATUS macros instead.
//
// Board_Debug_Init, Board_SetLCDBacklight, Board_UARTGetChar, Board_UARTPutSTR:
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


static uint32_t initGenesisModule (void)
{
    bool lp5036     = false;
    bool pca9673    = false;

    uint8_t txData[1];
    uint8_t rxData[1];

    // Store original IO1 value to use in case of LP5036 detection failure
    const bool Lp5036EnInitState = BOARD_GPIO_FAST_ACCESS (SP_GENESIS_LP5036);

    // Enable hardware
    BOARD_GPIO_SET_STATE (SP_GENESIS_LP5036, ENABLED);

    // Wait approximately 1 ms until LP5036 finishes initialization phase after
    // hardware enabled.
    for (volatile uint32_t i = 0; i < (SystemCoreClock / 500); ++i);

    // RGB Led driver LP5036, read register DEVICE_CONFIG0
    // (still in standby mode)
    txData[0] = 0x00;
    if (Board_I2CMTransfer (BOARD_STACKPORT_I2C_INTERFACE,
                            BOARD_SP_GENESIS_I2C_ADDR_LP5036,
                            txData, 1, rxData, 1)
        == I2CM_STATUS_OK)
    {
        lp5036 = true;
    }
    else
    {
        BOARD_GPIO_FAST_WRITE (SP_GENESIS_LP5036, Lp5036EnInitState);
    }

    // I/O expander PCA9673, set and read from Port 0.
    txData[0] = 0xFF;   // All pins LOW
    if (Board_I2CMTransfer (BOARD_STACKPORT_I2C_INTERFACE,
                            BOARD_SP_GENESIS_I2C_ADDR_PCA9673,
                            txData, 1, rxData, 1)
        == I2CM_STATUS_OK)
    {
        pca9673 = true;
    }

    return (lp5036 && pca9673)? BOARD_SP_GENESIS : 0;
}


static bool checkSslPca9956 (uint8_t addr)
{
    uint8_t txData[2];
    uint8_t rxData[2] = { 0, 0 };

    // PCA9956B, read MODE1 and MODE 2 registers (0x80 = auto increment)
    txData[0] = 0x80;
    if (Board_I2CMTransfer (BOARD_STACKPORT_I2C_INTERFACE, addr,
                            txData, 1, rxData, 2)
        != I2CM_STATUS_OK)
    {
        return false;
    }

    // Check MODE1 and MODE 2 reset default values to discard false positives
    // at the same i2c address
    if (rxData[0] != 0x89 || rxData[1] != 0x05)
    {
        return false;
    }

    return true;
}


static uint32_t initSslModule (void)
{
    bool    pca9956_A = false;
    bool    pca9956_B = false;

    pca9956_A = checkSslPca9956 (BOARD_SP_SSL_I2C_ADDR_PCA9956B_DA);
    pca9956_B = checkSslPca9956 (BOARD_SP_SSL_I2C_ADDR_PCA9956B_DB);

    if (pca9956_A && pca9956_B)
    {
        Board_SetSslModuleBacklight (false);

        return BOARD_SP_SSL;
    }

    return 0;
}


void initStackPortModules (void)
{
    s_StackPortModules |= initGenesisModule ();
    s_StackPortModules |= initSslModule     ();
}


// Internal board initialization functions


static void initLocalPeripherals ()
{   
    //Board_SetSdPower (true);

    // ASSERT (BOARD_I2C_INTERFACE == I2C1)
    Chip_I2C_Init               (BOARD_I2C_ID);
    Chip_I2C_SetClockRate       (BOARD_I2C_ID, BOARD_I2C_SPEED);
    // Local I2C bus events resolved by polling instead of interruption.
    Chip_I2C_SetMasterEventHandler
                                (BOARD_I2C_ID, Chip_I2C_EventHandlerPolling);

    Chip_UART_Init              (BOARD_DEBUG_INTERFACE);
    Chip_UART_SetBaudFDR        (BOARD_DEBUG_INTERFACE,
                                 BOARD_DEBUG_BAUD_RATE);
    Chip_UART_ConfigData        (BOARD_DEBUG_INTERFACE,
                                 BOARD_DEBUG_CONFIG);
    Chip_UART_TXEnable          (BOARD_DEBUG_INTERFACE);

    Chip_UART_Init              (BOARD_ESP_INTERFACE);
    Chip_UART_SetBaudFDR        (BOARD_ESP_INTERFACE, BOARD_ESP_BAUD_RATE);
    Chip_UART_ConfigData        (BOARD_ESP_INTERFACE, BOARD_ESP_CONFIG);
//    Chip_UART_SetModemControl   (BOARD_ESP_INTERFACE,
//                                 UART_MCR_AUTO_RTS_EN | UART_MCR_AUTO_CTS_EN);
    Chip_UART_TXEnable          (BOARD_ESP_INTERFACE);

//    Board_SetSdPower (false);

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

	/* Enable rtc (starts increase the tick counter and second counter register) */
	Chip_RTC_Enable (LPC_RTC, ENABLE);
    #endif
}


static void initStackPortPeripherals ()
{
    // ASSERT (BOARD_STACKPORT_I2C_INTERFACE == I2C0)
    Chip_I2C_Init               (BOARD_STACKPORT_I2C_ID);
    Chip_SCU_I2C0PinConfig      (BOARD_STACKPORT_I2C_MODE);
    Chip_I2C_SetClockRate       (BOARD_STACKPORT_I2C_ID,
                                 BOARD_STACKPORT_I2C_SPEED);

    Chip_SSP_Init               (BOARD_STACKPORT_SPI_INTERFACE);
    Chip_SSP_Set_Mode           (BOARD_STACKPORT_SPI_INTERFACE,
                                 BOARD_STACKPORT_SPI_MODE);
    Chip_SSP_SetFormat          (BOARD_STACKPORT_SPI_INTERFACE,
                                 BOARD_STACKPORT_SPI_BITS,
                                 BOARD_STACKPORT_SPI_FORMAT,
                                 BOARD_STACKPORT_SPI_POLARITY);
    Chip_SSP_SetBitRate         (BOARD_STACKPORT_SPI_INTERFACE,
                                 BOARD_STACKPORT_SPI_SPEED);
    Chip_SSP_Enable             (BOARD_STACKPORT_SPI_INTERFACE);

    Chip_UART_Init              (BOARD_STACKPORT_UART_INTERFACE);
    Chip_UART_SetBaudFDR        (BOARD_STACKPORT_UART_INTERFACE,
                                 BOARD_STACKPORT_UART_BAUD_RATE);
    Chip_UART_ConfigData        (BOARD_STACKPORT_UART_INTERFACE,
                                 BOARD_STACKPORT_UART_CONFIG);
    Chip_UART_TXEnable          (BOARD_STACKPORT_UART_INTERFACE);

    // TODO: EIA-485
}


bool Board_DetectedStackPortModules (uint32_t mask)
{
    return ((s_StackPortModules & mask) == mask)? true : false;
}


// For unknown reasons, using BOARD_GPIO_Out to output a high will not 
// completely turn OFF the corresponding BSS209PW PMOS; it will barely dim the 
// backlight on StackPort SSL module and the power led (the power itself,
// which leads to sdmmc init failures) on RETRO-CIAA-STANDALONE->SD_POW.
static void pmosDrive (bool on, uint8_t group, uint8_t pin, uint16_t scu_mode,
                       uint8_t gpio_port, uint8_t gpio_pin)
{
    if (on)
    {
        // GND -> ON
        BOARD_GPIO_Out (group, pin, scu_mode, gpio_port, gpio_pin, false);
    }
    else 
    {
        // VDD -> OFF
        BOARD_GPIO_In (group, pin, scu_mode, gpio_port, gpio_pin);
    }
}


void Board_SetSslModuleBacklight (bool on)
{
    pmosDrive (on, 4, 9, SCU_MODE_FUNC4,
               BOARD_GPIO_SP_SSL_BACKLIGHT_PORT,
               BOARD_GPIO_SP_SSL_BACKLIGHT_PIN);
}


bool Board_GetSslModuleBacklight (void)
{
    return false;
}


void Board_SetSdPower (bool on)
{
    pmosDrive (on, 1, 5, SCU_MODE_FUNC0 | SCU_MODE_PULLUP | SCU_MODE_14MA_DRIVESTR | SCU_MODE_INBUFF_EN,
               BOARD_GPIO_OUT_SD_POW_PORT,
               BOARD_GPIO_OUT_SD_POW_PIN);
}


bool Board_GetSdPower (void)
{
    return false;
}


// Standard API board init
void Board_Init (void)
{
    Chip_GPIO_Init (LPC_GPIO_PORT);

    initLocalPeripherals        ();
    initStackPortPeripherals    ();    
    initStackPortModules        ();
}

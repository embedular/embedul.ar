/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD driver] generic support for st nucleo-144 -based boards.

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

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/io_board_nucleo_144.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/stream_uart.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/random_rng.h"
// Selected nucleo-144 -based board
#include "cubemx/Core/Inc/gpio.h"
#include "cubemx/Core/Inc/eth.h"
#include "cubemx/Core/Inc/usart.h"
#include "cubemx/Core/Inc/usb_otg.h"
#include "cubemx/Core/Inc/rng.h"
// BSP for Nucleo-144 boards
#include "embedul.ar/source/arch/arm-cortex/stm32/nucleo_144_bsp.h"


#define BOARD_INFO_0_NAME           "stm32 nucleo-144 board"
#define BOARD_INFO_1_CHIP           EMBEDULAR_ARCH_CHIP

#define BOARD_INFO_FMT              "`M40`0`L1" \
                                    "`M40`1`L1"


struct BOARD_IO_PROFILES
{
    struct INPUT_PROFILE_MAIN   inMain;
    struct OUTPUT_PROFILE_SIGN  outSign;
};


struct BOARD_NUCLEO_144
{
    struct BOARD                device;
    struct IO_BOARD_NUCLEO_144  ioBoard;
    struct STREAM_UART          streamDebugUart;
    struct RANDOM_RNG           randomRng;
    uint8_t                     debugInBuffer[16];
    uint8_t                     debugOutBuffer[16];
    uint8_t                     tcpServerInBuffer[64];
    uint8_t                     tcpServerOutBuffer[1024];
};


#ifdef BSS_SECTION_BOARD
BSS_SECTION_BOARD
#endif
static struct BOARD_NUCLEO_144 s_board_nucleo_144;

#ifdef BSS_SECTION_IO_PROFILES
BSS_SECTION_IO_PROFILES
#endif
static struct BOARD_IO_PROFILES s_io_profiles;


static void *               stageChange     (struct BOARD *const B,
                                             const enum BOARD_Stage Stage);
static void                 assertFunc      (struct BOARD *const B, 
                                             const bool Condition);


static const struct BOARD_IFACE BOARD_NUCLEO_144_IFACE =
{
    .Description    = "st nucleo-144",
    .StageChange    = stageChange,
    .Assert         = assertFunc
};


// Required by the standard stm32 startup file
void _init (void) 
{
}


// Cortex-M specific interrupt handler
void HardFault_Handler (void)
{
    #ifdef DEBUG
    __BKPT (0);
    #endif

    while (1)
    {
        BSP_LED_Toggle (LED_RED);
        for (uint32_t i = 0; i < (SystemCoreClock / 20); ++i);
    }
}


static void sendString (const char *const Str)
{
    for (const char *p = Str; *p; ++p)
    {
        HAL_UART_Transmit (&huart3, (const uint8_t *)p, 1, 0);
    }
}


static void panic (const char *const ErrorMsg)
{
    if (ErrorMsg)
    {
        sendString ("BOARD PANIC.\r\n");
        sendString ("Error: ");
        sendString (ErrorMsg);
        sendString ("\r\n");
    }

    HardFault_Handler ();
}


struct BOARD * BOARD__boot (const int Argc, const char **const Argv,
                            struct BOARD_RIG *const R)
{
    struct BOARD *const B = (struct BOARD *)&s_board_nucleo_144;

    const char *const ErrorMsg = BOARD_Init (B, &BOARD_NUCLEO_144_IFACE,
                                             Argc, Argv, R);
    if (ErrorMsg)
    {
        panic (ErrorMsg);
    }

    return B;
}


static void greetings (struct STREAM *const B)
{
    STREAM_IN_FromParsedString (B, 0, 512, BOARD_INFO_FMT,
                                           BOARD_INFO_0_NAME,
                                           BOARD_INFO_1_CHIP);
}


static void stm32HalTickHook (const TIMER_Ticks Ticks)
{
    (void) Ticks;

    HAL_IncTick ();
}


void SystemClock_Config (void);


static void * stageChange (struct BOARD *const B, const enum BOARD_Stage Stage)
{
    struct BOARD_NUCLEO_144 *const N = (struct BOARD_NUCLEO_144 *) B;

    switch (Stage)
    {
        case BOARD_Stage_InitPreTicksHardware:
        {
            // Reset peripherals, Initializes Flash interface and Systick.
            HAL_Init ();

            // Configure the system clock (cubemx/Core/Inc/main.c)
            SystemClock_Config ();
            break;
        }

        case BOARD_Stage_InitPostTicksHardware:
        {
            TICKS_SetHook (stm32HalTickHook);

            // Configure hardware pins and peripherals
            MX_GPIO_Init ();
            MX_ETH_Init ();
            MX_USART3_UART_Init ();
            MX_USB_OTG_FS_PCD_Init ();
            MX_RNG_Init ();

            // Initialize minimum BSP functionality (HardFault led)
            BSP_LED_Init (LED_RED);
            break;
        }

        case BOARD_Stage_InitDebugStreamDriver:
        {
            STREAM_UART3_Init (&N->streamDebugUart, &huart3,
                               N->debugInBuffer, sizeof(N->debugInBuffer),
                               N->debugOutBuffer, sizeof(N->debugOutBuffer));
            return &N->streamDebugUart;
        }

        case BOARD_Stage_Greetings:
        {
            greetings ((struct STREAM *)&N->streamDebugUart);
            break;
        }

        case BOARD_Stage_InitIOProfiles:
        {
            INPUT_PROFILE_ATTACH    (MAIN, B, s_io_profiles.inMain);
            OUTPUT_PROFILE_ATTACH   (SIGN, B, s_io_profiles.outSign);
            break;
        }

        case BOARD_Stage_InitRandomDriver:
        {
            RANDOM_RNG_Init (&N->randomRng, &hrng);
            return &N->randomRng;
        }

        case BOARD_Stage_InitIOLevel1Drivers:
        {   
            IO_BOARD_NUCLEO_144_Init   (&N->ioBoard);
            IO_BOARD_NUCLEO_144_Attach (&N->ioBoard);    
            break;
        }

        case BOARD_Stage_InitCommDrivers:
        {
            break;
        }

        case BOARD_Stage_InitStorageDrivers:
        {
            break;
        }

        case BOARD_Stage_InitIOLevel2Drivers:
        {
            break;
        }

        case BOARD_Stage_InitScreenDrivers:
        {
            break;
        }

        case BOARD_Stage_InitSoundDriver:
        {
            break;
        }

        case BOARD_Stage_InitIOLevel3Drivers:
        {
            break;
        }

        case BOARD_Stage_Ready:
        {
            break;
        }

        case BOARD_Stage_ShutdownHardware:
        {
            while (1)
            {
                __WFI ();
            }
        }
    }

    return NULL;
}


void assertFunc (struct BOARD *const B, const bool Condition)
{
    (void) B;

    if (!Condition)
    {
        panic (NULL);
    }
}

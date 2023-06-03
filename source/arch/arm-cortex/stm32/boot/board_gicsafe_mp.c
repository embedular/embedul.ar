/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD driver] conicet-gicsafe "modulo de procesamiento" for trenes
                 argentinos operaciones.

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
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/io_board_gicsafe_mp.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/stream_uart.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/random_rng.h"
#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/rawstor_sd.h"
#include "cubemx/Core/Inc/adc.h"
#include "cubemx/Core/Inc/can.h"
#include "cubemx/Core/Inc/gpio.h"
#include "cubemx/Core/Inc/i2c.h"
#include "cubemx/Core/Inc/rng.h"
#include "cubemx/Core/Inc/rtc.h"
#include "cubemx/Core/Inc/sdio.h"
#include "cubemx/Core/Inc/spi.h"
#include "cubemx/Core/Inc/usart.h"


#define BOARD_INFO_0_NAME       "gicsafe \"modulo de procesamiento v1.62\""
#define BOARD_INFO_1_NAME       "for trenes argentinos operaciones"
#define BOARD_INFO_2_CHIP       EMBEDULAR_ARCH_CHIP
#define BOARD_INFO_FMT          "`M40`0`L1" \
                                "`M40`1`L1" \
                                "`M40`2`L1"


struct BOARD_IO_PROFILES
{
    struct INPUT_PROFILE_MAIN       inMain;
    struct INPUT_PROFILE_CONTROL    inControl;
    struct OUTPUT_PROFILE_SIGN      outSign;
};


struct BOARD_GICSAFE_MP
{
    struct BOARD                    device;
    struct IO_BOARD_GICSAFE_MP      ioBoard;
    struct STREAM_UART              streamDebugUart;
    struct RANDOM_RNG               randomRng;
    struct RAWSTOR_SD               rawstorSd;
    uint8_t                         debugInBuffer[16];
    uint8_t                         debugOutBuffer[16];
    uint8_t                         tcpServerInBuffer[64];
    uint8_t                         tcpServerOutBuffer[1024];
};


#ifdef BSS_SECTION_BOARD
BSS_SECTION_BOARD
#endif
static struct BOARD_GICSAFE_MP s_board_gicsafe_mp;

#ifdef BSS_SECTION_IO_PROFILES
BSS_SECTION_IO_PROFILES
#endif
static struct BOARD_IO_PROFILES s_io_profiles;


static void *               stageChange     (struct BOARD *const B,
                                             const enum BOARD_Stage Stage);
static void                 assertFunc      (struct BOARD *const B, 
                                             const bool Condition);


static const struct BOARD_IFACE BOARD_GICSAFE_MP_IFACE =
{
    .Description    = "gicsafe mp",
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
        // Check io_board_gicsafe_mp.c
        HAL_GPIO_TogglePin (GPIOB, LED1_Pin);
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
    struct BOARD *const B = (struct BOARD *)&s_board_gicsafe_mp;

    const char *const ErrorMsg = BOARD_Init (B, &BOARD_GICSAFE_MP_IFACE,
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
                                           BOARD_INFO_1_NAME,
                                           BOARD_INFO_2_CHIP);
}


static void stm32HalTickHook (const TIMER_Ticks Ticks)
{
    (void) Ticks;

    HAL_IncTick ();
}


void SystemClock_Config (void);


static void * stageChange (struct BOARD *const B, const enum BOARD_Stage Stage)
{
    struct BOARD_GICSAFE_MP *const G = (struct BOARD_GICSAFE_MP *) B;

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
            MX_USART6_UART_Init();
            MX_USART1_UART_Init();
            MX_USART2_UART_Init();
            MX_USART3_UART_Init();
            MX_UART4_Init();
            MX_ADC1_Init();
            MX_CAN1_Init();
            MX_I2C1_Init();
            MX_RNG_Init();
            MX_SPI1_Init();
            MX_RTC_Init();
            // MX_SDIO_SD_Init() will put the system in an unrecoverable error
            // state simply because there is no SD card in the slot (lame...)
            // This board will be using custom code in a RAWSTOR device that
            // does allow initializing the hardware first and mounting the sd
            // card on-demand later.
            break;
        }

        case BOARD_Stage_InitDebugStreamDriver:
        {
            STREAM_UART6_Init (&G->streamDebugUart, &huart6,
                               G->debugInBuffer, sizeof(G->debugInBuffer),
                               G->debugOutBuffer, sizeof(G->debugOutBuffer));
            return &G->streamDebugUart;
        }

        case BOARD_Stage_Greetings:
        {
            greetings ((struct STREAM *)&G->streamDebugUart);
            break;
        }

        case BOARD_Stage_InitIOProfiles:
        {
            INPUT_PROFILE_ATTACH    (MAIN, B, s_io_profiles.inMain);
            INPUT_PROFILE_ATTACH    (CONTROL, B, s_io_profiles.inControl);
            OUTPUT_PROFILE_ATTACH   (SIGN, B, s_io_profiles.outSign);
            break;
        }

        case BOARD_Stage_InitRandomDriver:
        {
            RANDOM_RNG_Init (&G->randomRng, &hrng);
            return &G->randomRng;
        }

        case BOARD_Stage_InitIOLevel1Drivers:
        {   
            IO_BOARD_GICSAFE_MP_Init   (&G->ioBoard);
            IO_BOARD_GICSAFE_MP_Attach (&G->ioBoard);    
            break;
        }

        case BOARD_Stage_InitCommDrivers:
        {
            break;
        }

        case BOARD_Stage_InitStorageDrivers:
        {
            // Unfortunately there is no way to use MX_SDIO_SD_Init(),
            // see above. SD card full-speed interface params must be replicated
            // here.
            RAWSTOR_SD_Init (&G->rawstorSd,
                             &hsd,
                             SDIO_CLOCK_EDGE_RISING,
                             SDIO_CLOCK_BYPASS_DISABLE,
                             SDIO_CLOCK_POWER_SAVE_DISABLE,
                             SDIO_BUS_WIDE_1B,
                             SDIO_HARDWARE_FLOW_CONTROL_DISABLE,
                             0);
            STORAGE_SetDevice ((struct RAWSTOR *)&G->rawstorSd);
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

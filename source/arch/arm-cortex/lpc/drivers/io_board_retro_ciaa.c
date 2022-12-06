/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] RETRO-CIAA standalone board.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/io_board_retro_ciaa.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/retro_ciaa/board.h"
#include "embedul.ar/source/core/device/board.h"


#define GET_SWITCH_STATE(_n) \
    B->inbData |= BOARD_SWITCH_GET_STATE(_n)? \
                        (1 << IO_BOARD_RETRO_CIAA_INB_##_n) : 0;

#define GET_GPIO_STATE(_n) \
    B->inbData |= BOARD_GPIO_GET_STATE(OUT_##_n)? \
                        (1 << IO_BOARD_RETRO_CIAA_INB_##_n) : 0;

#define SET_LED_STATE(_n) \
    (B->outbData & (1 << IO_BOARD_RETRO_CIAA_OUTB_LED_##_n))? \
                        BOARD_LED_ON(_n) : BOARD_LED_OFF(_n);

#define SET_GPIO_STATE(_n) \
    (B->outbData & (1 << IO_BOARD_RETRO_CIAA_OUTB_##_n))? \
                        BOARD_GPIO_SET_STATE(OUT_##_n, ENABLED) : \
                        BOARD_GPIO_SET_STATE(OUT_##_n, DISABLED);

/*
void EVRT_IRQHandler (void)
{
}
*/

static const char * s_InputBitNames[IO_BOARD_RETRO_CIAA_INB__COUNT] =
{
    [IO_BOARD_RETRO_CIAA_INB_ISP]               = "ISP",
    [IO_BOARD_RETRO_CIAA_INB_WAKEUP]            = "Wake up",
    [IO_BOARD_RETRO_CIAA_INB_BOARD_BACKLIGHT]   = "Backlight",
    [IO_BOARD_RETRO_CIAA_INB_SD_POW]            = "SD power",
    [IO_BOARD_RETRO_CIAA_INB_WIFI_EN]           = "WiFi enable",
    [IO_BOARD_RETRO_CIAA_INB_SOUND_MUTE]        = "Sound mute"
};


static const char * s_OutputBitNames[IO_BOARD_RETRO_CIAA_OUTB__COUNT] =
{
    [IO_BOARD_RETRO_CIAA_OUTB_LED_WARN]         = "Warning",
    [IO_BOARD_RETRO_CIAA_OUTB_BOARD_BACKLIGHT]  = "Backlight",
    [IO_BOARD_RETRO_CIAA_OUTB_SD_POW]           = "SD power",
    [IO_BOARD_RETRO_CIAA_OUTB_WIFI_EN]          = "WiFi enable",
    [IO_BOARD_RETRO_CIAA_OUTB_SOUND_MUTE]       = "Sound mute"
};


static void         update              (struct IO *const Io);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port,
                                         const uint32_t Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_BOARD_RETRO_CIAA_IFACE =
{
    IO_IFACE_DECLARE("retro-ciaa board io", BOARD_RETRO_CIAA),
    .Update         = update,
    .GetInput       = getInput,
    .SetOutput      = setOutput,
    .InputName      = inputName,
    .OutputName     = outputName
};


void IO_BOARD_RETRO_CIAA_Init (struct IO_BOARD_RETRO_CIAA *const B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    IO_INIT_STATIC_PORT_INFO (B, BOARD_RETRO_CIAA);

    // Switch wired to WAKEUP0 is active HIGH
    B->wakeupActive = EVRT_SRC_ACTIVE_RISING_EDGE;

    Chip_EVRT_Init ();
    Chip_EVRT_ConfigIntSrcActiveType (EVRT_SRC_WAKEUP0, B->wakeupActive);
    Chip_EVRT_SetUpIntSrc (EVRT_SRC_WAKEUP0, ENABLE);

    LPC_EVRT->CLR_STAT |= 1 << EVRT_SRC_WAKEUP0;

    // NVIC_EnableIRQ (EVENTROUTER_IRQn);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_RETRO_CIAA_IFACE, B->portInfo, 15);
}


void IO_BOARD_RETRO_CIAA_Attach (struct IO_BOARD_RETRO_CIAA *const B)
{
    BOARD_AssertParams (B);

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)B, 0);

    MIO_MAP_INPUT_BIT (MAIN, A, IO_BOARD_RETRO_CIAA_INB_ISP);
    MIO_MAP_INPUT_BIT (MAIN, B, IO_BOARD_RETRO_CIAA_INB_WAKEUP);
    MIO_MAP_INPUT_BIT (CONTROL, Backlight, IO_BOARD_RETRO_CIAA_INB_BOARD_BACKLIGHT);
    MIO_MAP_INPUT_BIT (CONTROL, StoragePower, IO_BOARD_RETRO_CIAA_INB_SD_POW);
    MIO_MAP_INPUT_BIT (CONTROL, WirelessEnable, IO_BOARD_RETRO_CIAA_INB_WIFI_EN);
    MIO_MAP_INPUT_BIT (CONTROL, SoundMute, IO_BOARD_RETRO_CIAA_INB_SOUND_MUTE);


    MIO_RegisterGateway (MIO_Dir_Output, (struct IO *)B, 0);

    MIO_MAP_OUTPUT_BIT (SIGN, Warning, IO_BOARD_RETRO_CIAA_OUTB_LED_WARN);
    // It will command the board backlight on the SSL module, if present
    MIO_MAP_OUTPUT_BIT (CONTROL, Backlight,
                    IO_BOARD_RETRO_CIAA_OUTB_BOARD_BACKLIGHT);
    MIO_MAP_OUTPUT_BIT (CONTROL, StoragePower, IO_BOARD_RETRO_CIAA_OUTB_SD_POW);
    MIO_MAP_OUTPUT_BIT (CONTROL, WirelessEnable, IO_BOARD_RETRO_CIAA_OUTB_WIFI_EN);
    MIO_MAP_OUTPUT_BIT (CONTROL, SoundMute, IO_BOARD_RETRO_CIAA_OUTB_SOUND_MUTE);
}


static void getWakeupState (struct IO_BOARD_RETRO_CIAA *const B)
{
    // Wakeup state transition
    if (LPC_EVRT->STATUS & 1)
    {
        B->wakeupActive = (B->wakeupActive == EVRT_SRC_ACTIVE_RISING_EDGE)?
                            EVRT_SRC_ACTIVE_FALLING_EDGE :
                            EVRT_SRC_ACTIVE_RISING_EDGE;

        Chip_EVRT_ConfigIntSrcActiveType (EVRT_SRC_WAKEUP0, B->wakeupActive);
        LPC_EVRT->CLR_STAT |= 1 << EVRT_SRC_WAKEUP0;
    }

    // Now detecting a falling edge, button is being pressed.
    if (B->wakeupActive == EVRT_SRC_ACTIVE_FALLING_EDGE)
    {
        B->inbData |= 1 << IO_BOARD_RETRO_CIAA_INB_WAKEUP;
    }
}


static inline void getBoardBacklight (struct IO_BOARD_RETRO_CIAA *const B)
{
    if (Board_DetectedStackPortModules (BOARD_SP_SSL))
    {
        B->inbData |= Board_GetSslModuleBacklight()? 
                            1 << IO_BOARD_RETRO_CIAA_INB_BOARD_BACKLIGHT : 0;
    }
}


static inline void getSdPower (struct IO_BOARD_RETRO_CIAA *const B)
{
    B->inbData |= Board_GetSdPower()? 
                        1 << IO_BOARD_RETRO_CIAA_INB_SD_POW : 0;
}


static inline void setBoardBacklight (struct IO_BOARD_RETRO_CIAA *const B)
{
    if (Board_DetectedStackPortModules (BOARD_SP_SSL))
    {
        Board_SetSslModuleBacklight (
                      B->outbData & (1 << IO_BOARD_RETRO_CIAA_OUTB_BOARD_BACKLIGHT));
    }
}


void update (struct IO *const Io)
{
    struct IO_BOARD_RETRO_CIAA *const B = (struct IO_BOARD_RETRO_CIAA *) Io;

    B->inbData = 0;

    // Input
    GET_SWITCH_STATE    (ISP);
    getWakeupState      (B);
    getBoardBacklight   (B);
    getSdPower          (B);
    GET_GPIO_STATE      (WIFI_EN);
    GET_GPIO_STATE      (SOUND_MUTE);

    // Output
    SET_LED_STATE       (WARN);
    setBoardBacklight   (B);
    Board_SetSdPower    (B->outbData & (1 << IO_BOARD_RETRO_CIAA_OUTB_SD_POW));
    SET_GPIO_STATE      (WIFI_EN);
    SET_GPIO_STATE      (SOUND_MUTE);
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) Port;
    (void) IoType;

    struct IO_BOARD_RETRO_CIAA *const B = (struct IO_BOARD_RETRO_CIAA *) Io;

    return (B->inbData & (1 << DriverCode));
}


void setOutput (struct IO *const Io, const enum IO_Type IoType,
                const IO_Code DriverCode, const IO_Port Port,
                const uint32_t Value)
{
    (void) Port;
    (void) IoType;

    struct IO_BOARD_RETRO_CIAA *const B = (struct IO_BOARD_RETRO_CIAA *) Io;

    if (Value)
    {
        B->outbData |= (1 << DriverCode);
    }
    else
    {
        B->outbData &= ~(1 << DriverCode);
    }
}


const char * inputName (struct IO *const Io, const enum IO_Type IoType,
                        const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_InputBitNames[DriverCode];
}


const char * outputName (struct IO *const Io, const enum IO_Type IoType,
                         const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_OutputBitNames[DriverCode];
}

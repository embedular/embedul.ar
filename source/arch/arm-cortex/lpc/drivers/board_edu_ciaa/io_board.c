/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] EDU-CIAA-NXP board.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_edu_ciaa/io_board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/edu_ciaa/board.h"
#include "embedul.ar/source/core/device/board.h"


#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    #define IO_BOARD_DESCRIPTION        "edu-ciaa w/retro board io"
#else
    #define IO_BOARD_DESCRIPTION        "edu-ciaa board io"
#endif


#define GET_SWITCH_STATE(_n) \
    B->inbData |= BOARD_SWITCH_GET_STATE(_n)? \
                            (1 << IO_BOARD_INB_##_n) : 0;

#define SET_GPIO_STATE(_n) \
    (B->outbData & (1 << IO_BOARD_OUTB_##_n))? \
                            BOARD_GPIO_SET_STATE(OUT_##_n, ENABLED) : \
                            BOARD_GPIO_SET_STATE(OUT_##_n, DISABLED);


static const char * s_InputBitNames[IO_BOARD_INB__COUNT] =
{
    [IO_BOARD_INB_TEC_1]            = "tec 1",
    [IO_BOARD_INB_TEC_2]            = "tec 2",
    [IO_BOARD_INB_TEC_3]            = "tec 3",
    [IO_BOARD_INB_TEC_4]            = "tec 4"
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    , 
    [IO_BOARD_INB_SD_DETECT]        = "sd_detect",
    [IO_BOARD_INB_WIFI_EN]          = "wifi on",
    [IO_BOARD_INB_SOUND_MUTE]       = "sound mute"
#endif
};


static const char * s_OutputBitNames[IO_BOARD_OUTB__COUNT] =
{
    [IO_BOARD_OUTB_LED_RGB_RED]     = "rgb red",
    [IO_BOARD_OUTB_LED_RGB_GREEN]   = "rgb green",
    [IO_BOARD_OUTB_LED_RGB_BLUE]    = "rgb blue",
    [IO_BOARD_OUTB_LED_1]           = "led 1",
    [IO_BOARD_OUTB_LED_2]           = "led 2",
    [IO_BOARD_OUTB_LED_3]           = "led 3" 
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    ,
    [IO_BOARD_OUTB_BOARD_BACKLIGHT] = "backlight",
    [IO_BOARD_OUTB_SD_SELECT]       = "sd select",
    [IO_BOARD_OUTB_WIFI_EN]         = "wifi on",
    [IO_BOARD_OUTB_SOUND_MUTE]      = "sound mute"
#endif
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


static const struct IO_IFACE IO_BOARD_IFACE =
{
    IO_IFACE_DECLARE(IO_BOARD_DESCRIPTION, BOARD),
    .Update         = update,
    .GetInput       = getInput,
    .SetOutput      = setOutput,
    .InputName      = inputName,
    .OutputName     = outputName
};


void IO_BOARD_Init (struct IO_BOARD *B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    IO_INIT_STATIC_PORT_INFO (B, BOARD);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_IFACE, B->portInfo, 15);
}


void IO_BOARD_Attach (struct IO_BOARD *const B)
{
    BOARD_AssertParams (B);

    INPUT_SetGateway ((struct IO *)B, 0);

    INPUT_MAP_BIT (MAIN, A, IO_BOARD_INB_TEC_1);
    INPUT_MAP_BIT (MAIN, B, IO_BOARD_INB_TEC_2);
    INPUT_MAP_BIT (MAIN, C, IO_BOARD_INB_TEC_3);
    INPUT_MAP_BIT (MAIN, D, IO_BOARD_INB_TEC_4);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    INPUT_MAP_BIT (CONTROL, StorageDetect, IO_BOARD_INB_SD_DETECT);
    INPUT_MAP_BIT (CONTROL, WirelessEnable, IO_BOARD_INB_WIFI_EN);
    INPUT_MAP_BIT (CONTROL, SoundMute, IO_BOARD_INB_SOUND_MUTE);
#endif

    OUTPUT_SetGateway ((struct IO *)B, 0);

    OUTPUT_MAP_BIT (SIGN, Warning, IO_BOARD_OUTB_LED_1);
    OUTPUT_MAP_BIT (SIGN, Red, IO_BOARD_OUTB_LED_RGB_RED);
    OUTPUT_MAP_BIT (SIGN, Green, IO_BOARD_OUTB_LED_RGB_GREEN);
    OUTPUT_MAP_BIT (SIGN, Blue, IO_BOARD_OUTB_LED_RGB_BLUE);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    OUTPUT_MAP_BIT (CONTROL, Backlight, IO_BOARD_OUTB_BOARD_BACKLIGHT);
    OUTPUT_MAP_BIT (CONTROL, StorageEnable, IO_BOARD_OUTB_SD_SELECT);
    OUTPUT_MAP_BIT (CONTROL, WirelessEnable, IO_BOARD_OUTB_WIFI_EN);
    OUTPUT_MAP_BIT (CONTROL, SoundMute, IO_BOARD_OUTB_SOUND_MUTE);
#endif
}


void update (struct IO *const Io)
{
    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    B->inbData = 0;

    GET_SWITCH_STATE(TEC_1);
    GET_SWITCH_STATE(TEC_2);
    GET_SWITCH_STATE(TEC_3);
    GET_SWITCH_STATE(TEC_4);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    GET_SWITCH_STATE(SD_DETECT);
#endif

    SET_GPIO_STATE(LED_RGB_RED);
    SET_GPIO_STATE(LED_RGB_GREEN);
    SET_GPIO_STATE(LED_RGB_BLUE);
    SET_GPIO_STATE(LED_1);
    SET_GPIO_STATE(LED_2);
    SET_GPIO_STATE(LED_3);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    Board_SetLCDBacklight (B->outbData & (1 << IO_BOARD_OUTB_BOARD_BACKLIGHT));
    SET_GPIO_STATE(SD_SELECT)
#endif
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) Port;
    (void) IoType;

    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    return (B->inbData & (1 << DriverCode));
}


void setOutput (struct IO *const Io, const enum IO_Type IoType,
                const IO_Code DriverCode, const IO_Port Port,
                const uint32_t Value)
{
    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    (void) Port;
    (void) IoType;

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

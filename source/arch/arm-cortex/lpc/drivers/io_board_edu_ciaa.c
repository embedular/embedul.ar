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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/io_board_edu_ciaa.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/edu_ciaa/board.h"
#include "embedul.ar/source/core/device/board.h"


#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    #define IO_BOARD_EDU_CIAA_DESCRIPTION       "edu-ciaa w/retro board io"
#else
    #define IO_BOARD_EDU_CIAA_DESCRIPTION       "edu-ciaa board io"
#endif


#define GET_SWITCH_STATE(_n) \
    B->inbData |= BOARD_SWITCH_GET_STATE(_n)? \
                            (1 << IO_BOARD_EDU_CIAA_INB_##_n) : 0;

#define SET_GPIO_STATE(_n) \
    (B->outbData & (1 << IO_BOARD_EDU_CIAA_OUTB_##_n))? \
                            BOARD_GPIO_SET_STATE(OUT_##_n, ENABLED) : \
                            BOARD_GPIO_SET_STATE(OUT_##_n, DISABLED);


static const char * s_InputBitNames[IO_BOARD_EDU_CIAA_INB__COUNT] =
{
    [IO_BOARD_EDU_CIAA_INB_TEC_1]            = "TEC 1",
    [IO_BOARD_EDU_CIAA_INB_TEC_2]            = "TEC 2",
    [IO_BOARD_EDU_CIAA_INB_TEC_3]            = "TEC 3",
    [IO_BOARD_EDU_CIAA_INB_TEC_4]            = "TEC 4"
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    , 
    [IO_BOARD_EDU_CIAA_INB_SD_DETECT]        = "SD detect",
    [IO_BOARD_EDU_CIAA_INB_WIFI_EN]          = "WiFi enable",
    [IO_BOARD_EDU_CIAA_INB_SOUND_MUTE]       = "Sound mute"
#endif
};


static const char * s_OutputBitNames[IO_BOARD_EDU_CIAA_OUTB__COUNT] =
{
    [IO_BOARD_EDU_CIAA_OUTB_LED_RGB_RED]     = "RGB Red",
    [IO_BOARD_EDU_CIAA_OUTB_LED_RGB_GREEN]   = "RGB Green",
    [IO_BOARD_EDU_CIAA_OUTB_LED_RGB_BLUE]    = "RGB Blue",
    [IO_BOARD_EDU_CIAA_OUTB_LED_1]           = "LED 1",
    [IO_BOARD_EDU_CIAA_OUTB_LED_2]           = "LED 2",
    [IO_BOARD_EDU_CIAA_OUTB_LED_3]           = "LED 3" 
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    ,
    [IO_BOARD_EDU_CIAA_OUTB_BOARD_BACKLIGHT] = "Backlight",
    [IO_BOARD_EDU_CIAA_OUTB_SD_SELECT]       = "SD select",
    [IO_BOARD_EDU_CIAA_OUTB_WIFI_EN]         = "WiFi enable",
    [IO_BOARD_EDU_CIAA_OUTB_SOUND_MUTE]      = "Sound mute"
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


static const struct IO_IFACE IO_BOARD_EDU_CIAA_IFACE =
{
    IO_IFACE_DECLARE(IO_BOARD_EDU_CIAA_DESCRIPTION, BOARD_EDU_CIAA),
    .Update         = update,
    .GetInput       = getInput,
    .SetOutput      = setOutput,
    .InputName      = inputName,
    .OutputName     = outputName
};


void IO_BOARD_EDU_CIAA_Init (struct IO_BOARD_EDU_CIAA *B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    IO_INIT_STATIC_PORT_INFO (B, BOARD_EDU_CIAA);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_EDU_CIAA_IFACE, B->portInfo, 15);
}


void IO_BOARD_EDU_CIAA_Attach (struct IO_BOARD_EDU_CIAA *const B)
{
    BOARD_AssertParams (B);

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)B, 0);

    MIO_MAP_INPUT_BIT (MAIN, A, IO_BOARD_EDU_CIAA_INB_TEC_1);
    MIO_MAP_INPUT_BIT (MAIN, B, IO_BOARD_EDU_CIAA_INB_TEC_2);
    MIO_MAP_INPUT_BIT (MAIN, C, IO_BOARD_EDU_CIAA_INB_TEC_3);
    MIO_MAP_INPUT_BIT (MAIN, D, IO_BOARD_EDU_CIAA_INB_TEC_4);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    MIO_MAP_INPUT_BIT (CONTROL, StorageDetect, IO_BOARD_EDU_CIAA_INB_SD_DETECT);
    MIO_MAP_INPUT_BIT (CONTROL, WirelessEnable, IO_BOARD_EDU_CIAA_INB_WIFI_EN);
    MIO_MAP_INPUT_BIT (CONTROL, SoundMute, IO_BOARD_EDU_CIAA_INB_SOUND_MUTE);
#endif


    MIO_RegisterGateway (MIO_Dir_Output, (struct IO *)B, 0);

    MIO_MAP_OUTPUT_BIT (SIGN, Warning, IO_BOARD_EDU_CIAA_OUTB_LED_1);
    MIO_MAP_OUTPUT_BIT (SIGN, Red, IO_BOARD_EDU_CIAA_OUTB_LED_2);
    MIO_MAP_OUTPUT_BIT (SIGN, Green, IO_BOARD_EDU_CIAA_OUTB_LED_3);
    MIO_MAP_OUTPUT_BIT (SIGN, Blue, IO_BOARD_EDU_CIAA_OUTB_LED_RGB_BLUE);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    MIO_MAP_OUTPUT_BIT (CONTROL, Backlight, IO_BOARD_EDU_CIAA_OUTB_BOARD_BACKLIGHT);
    MIO_MAP_OUTPUT_BIT (CONTROL, StorageEnable, IO_BOARD_EDU_CIAA_OUTB_SD_SELECT);
    MIO_MAP_OUTPUT_BIT (CONTROL, WirelessEnable, IO_BOARD_EDU_CIAA_OUTB_WIFI_EN);
    MIO_MAP_OUTPUT_BIT (CONTROL, SoundMute, IO_BOARD_EDU_CIAA_OUTB_SOUND_MUTE);
#endif
}


void update (struct IO *const Io)
{
    struct IO_BOARD_EDU_CIAA *const B = (struct IO_BOARD_EDU_CIAA *) Io;

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
    Board_SetLCDBacklight (B->outbData & (1 << IO_BOARD_EDU_CIAA_OUTB_BOARD_BACKLIGHT));
    SET_GPIO_STATE(SD_SELECT)
#endif
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) Port;
    (void) IoType;

    struct IO_BOARD_EDU_CIAA *const B = (struct IO_BOARD_EDU_CIAA *) Io;

    return (B->inbData & (1 << DriverCode));
}


void setOutput (struct IO *const Io, const enum IO_Type IoType,
                const IO_Code DriverCode, const IO_Port Port,
                const uint32_t Value)
{
    struct IO_BOARD_EDU_CIAA *const B = (struct IO_BOARD_EDU_CIAA *) Io;

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

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


static const char * s_InputNames[] =
{
    "tec 1", "tec 2", "tec 3", "tec 4"
    #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    , "sd_detect", "wifi on", "sound mute"
    #endif
};


static const char * s_OutputNames[] =
{
    "rgb red", "rgb green", "rgb blue", "led 1", "led 2", "led 3" 
    #ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    , "backlight", "sd select", "wifi on", "sound mute"
    #endif
};


static void         update              (struct IO *const Io);
static IO_Count
                    availableInputs     (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t InputSource);
static IO_Count
                    availableOutputs    (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t OutputSource);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t InputSource);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t OutputSource,
                                         const uint32_t Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);


static const struct IO_IFACE IO_BOARD_IFACE =
{
    .Description        = IO_BOARD_DESCRIPTION,
    .Update             = update,
    .AvailableInputs    = availableInputs,
    .AvailableOutputs   = availableOutputs,
    .GetInput           = getInput,
    .SetOutput          = setOutput,
    .InputName          = inputName,
    .OutputName         = outputName
};


void IO_BOARD_Init (struct IO_BOARD *B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_IFACE, 15);
}


void IO_BOARD_Attach (struct IO_BOARD *const B)
{
    BOARD_AssertParams (B);

    INPUT_SetDevice ((struct IO *)B, 0);

    INPUT_MAP_BIT (MAIN, A, IO_BOARD_INB_TEC_1);
    INPUT_MAP_BIT (MAIN, B, IO_BOARD_INB_TEC_2);
    INPUT_MAP_BIT (MAIN, C, IO_BOARD_INB_TEC_3);
    INPUT_MAP_BIT (MAIN, D, IO_BOARD_INB_TEC_4);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    INPUT_MAP_BIT (CONTROL, StorageDetect, IO_BOARD_INB_SD_DETECT);
    INPUT_MAP_BIT (CONTROL, WirelessEnable, IO_BOARD_INB_WIFI_EN);
    INPUT_MAP_BIT (CONTROL, SoundMute, IO_BOARD_INB_SOUND_MUTE);
#endif

    OUTPUT_SetDevice ((struct IO *)B, 0);

    OUTPUT_MapBit (OUTPUT_Bit_Warning, IO_BOARD_OUTB_LED_1);
    OUTPUT_MapBit (OUTPUT_Bit_RedSign, IO_BOARD_OUTB_LED_RGB_RED);
    OUTPUT_MapBit (OUTPUT_Bit_GreenSign, IO_BOARD_OUTB_LED_RGB_GREEN);
    OUTPUT_MapBit (OUTPUT_Bit_BlueSign, IO_BOARD_OUTB_LED_RGB_BLUE);
#ifdef BOARD_EDU_CIAA_WITH_RETRO_PONCHO
    OUTPUT_MapBit (OUTPUT_Bit_Backlight, IO_BOARD_OUTB_BOARD_BACKLIGHT);
    OUTPUT_MapBit (OUTPUT_Bit_StorageEnable, IO_BOARD_OUTB_SD_SELECT);
    OUTPUT_MapBit (OUTPUT_Bit_WirelessEnable, IO_BOARD_OUTB_WIFI_EN);
    OUTPUT_MapBit (OUTPUT_Bit_SoundMute, IO_BOARD_OUTB_SOUND_MUTE);
#endif
}


void update (struct IO *const Io)
{
    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    B->inbData = 0;

    #define GET_SWITCH_STATE(_n) \
                B->inbData |= BOARD_SWITCH_GET_STATE(_n)? \
                        (1 << IO_BOARD_INB_##_n) : 0;

    #define SET_GPIO_STATE(_n) \
                (B->outbData & (1 << IO_BOARD_OUTB_##_n))? \
                        BOARD_GPIO_SET_STATE(OUT_##_n, ENABLED) : \
                        BOARD_GPIO_SET_STATE(OUT_##_n, DISABLED);

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
    
    #undef SET_GPIO_STATE
    #undef GET_SWITCH_STATE
}


IO_Count availableInputs (struct IO *const Io, const enum IO_Type IoType,
                          const uint32_t InputSource)
{
    (void) Io;
    (void) InputSource;

    // This driver handles no analog inputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    }

    return IO_BOARD_INB__COUNT;
}


IO_Count availableOutputs (struct IO *const Io, const enum IO_Type IoType,
                           const uint32_t OutputSource)
{
    (void) Io;
    (void) OutputSource;

    // This driver handles no analog outputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    }

    return IO_BOARD_OUTB__COUNT;
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const uint16_t Index, const uint32_t InputSource)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                         Index < IO_BOARD_INB__COUNT);
    
    (void) InputSource;

    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    return (B->inbData & (1 << Index));
}


void setOutput (struct IO *const Io, const enum IO_Type IoType,
                const uint16_t Index, const uint32_t OutputSource,
                const uint32_t Value)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                         Index < IO_BOARD_OUTB__COUNT);

    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    (void) OutputSource;

    if (Value)
    {
        B->outbData |= (1 << Index);
    }
    else
    {
        B->outbData &= ~(1 << Index);
    }
}


const char * inputName (struct IO *const Io, const enum IO_Type IoType,
                        const uint16_t Index)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                         Index < IO_BOARD_INB__COUNT);

    (void) Io;

    return s_InputNames[Index];
}


const char * outputName (struct IO *const Io, const enum IO_Type IoType,
                         const uint16_t Index)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                         Index < IO_BOARD_OUTB__COUNT);

    (void) Io;

    return s_OutputNames[Index];
}

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/board_retro_ciaa/io_board.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/boards/retro_ciaa/board.h"
#include "embedul.ar/source/core/device/board.h"

/*
void EVRT_IRQHandler (void)
{
}
*/

static const char * s_InputNames[IO_BOARD_INB__COUNT] =
{
    "isp", "wakeup", "backlight", "sd power", "wifi on", "sound mute"
};


static const char * s_OutputNames[IO_BOARD_OUTB__COUNT] =
{
    "warning", "backlight", "sd power", "wifi on", "sound mute"
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
    .Description        = "retro-ciaa board io",
    .Update             = update,
    .AvailableInputs    = availableInputs,
    .AvailableOutputs   = availableOutputs,
    .GetInput           = getInput,
    .SetOutput          = setOutput,
    .InputName          = inputName,
    .OutputName         = outputName
};


void IO_BOARD_Init (struct IO_BOARD *const B)
{
    BOARD_AssertParams (B);

    DEVICE_IMPLEMENTATION_Clear (B);

    // Switch wired to WAKEUP0 is active HIGH
    B->wakeupActive = EVRT_SRC_ACTIVE_RISING_EDGE;

    Chip_EVRT_Init ();
    Chip_EVRT_ConfigIntSrcActiveType (EVRT_SRC_WAKEUP0, B->wakeupActive);
    Chip_EVRT_SetUpIntSrc (EVRT_SRC_WAKEUP0, ENABLE);

    LPC_EVRT->CLR_STAT |= 1 << EVRT_SRC_WAKEUP0;

    // NVIC_EnableIRQ (EVENTROUTER_IRQn);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)B, &IO_BOARD_IFACE, 15);
}


void IO_BOARD_Attach (struct IO_BOARD *const B)
{
    BOARD_AssertParams (B);

    OUTPUT_SetDevice ((struct IO *)B, 0);

    // It will command the board backlight on the SSL module, if present
    OUTPUT_MapBit (OUTPUT_Bit_Backlight, IO_BOARD_OUTB_BOARD_BACKLIGHT);
    OUTPUT_MapBit (OUTPUT_Bit_Warning, IO_BOARD_OUTB_LED_WARN);
    OUTPUT_MapBit (OUTPUT_Bit_StoragePower, IO_BOARD_OUTB_SD_POW);
    OUTPUT_MapBit (OUTPUT_Bit_WirelessEnable, IO_BOARD_OUTB_WIFI_EN);
    OUTPUT_MapBit (OUTPUT_Bit_SoundMute, IO_BOARD_OUTB_SOUND_MUTE);


    INPUT_SetDevice ((struct IO *)B, 0);

    INPUT_MAP_BIT (MAIN, A, IO_BOARD_INB_ISP);
    INPUT_MAP_BIT (MAIN, B, IO_BOARD_INB_WAKEUP);
    INPUT_MAP_BIT (CONTROL, Backlight, IO_BOARD_INB_BOARD_BACKLIGHT);
    INPUT_MAP_BIT (CONTROL, StoragePower, IO_BOARD_INB_SD_POW);
    INPUT_MAP_BIT (CONTROL, WirelessEnable, IO_BOARD_INB_WIFI_EN);
    INPUT_MAP_BIT (CONTROL, SoundMute, IO_BOARD_INB_SOUND_MUTE);
}


static void getWakeupState (struct IO_BOARD *const B)
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
        B->inbData |= 1 << IO_BOARD_INB_WAKEUP;
    }
}


static void getBoardBacklight (struct IO_BOARD *const B)
{
    if (Board_DetectedStackPortModules (BOARD_SP_SSL))
    {
        B->inbData |= Board_GetSslModuleBacklight()? 
                            1 << IO_BOARD_INB_BOARD_BACKLIGHT : 0;
    }
}


static void getSdPower (struct IO_BOARD *const B)
{
    B->inbData |= Board_GetSdPower()? 
                        1 << IO_BOARD_INB_SD_POW : 0;
}


static void setBoardBacklight (struct IO_BOARD *const B)
{
    if (Board_DetectedStackPortModules (BOARD_SP_SSL))
    {
        Board_SetSslModuleBacklight (
                      B->outbData & (1 << IO_BOARD_OUTB_BOARD_BACKLIGHT));
    }
}


void update (struct IO *const Io)
{
    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

    B->inbData = 0;

    #define GET_SWITCH_STATE(_n) \
                B->inbData |= BOARD_SWITCH_GET_STATE(_n)? \
                        (1 << IO_BOARD_INB_##_n) : 0;

    #define GET_GPIO_STATE(_n) \
                B->inbData |= BOARD_GPIO_GET_STATE(OUT_##_n)? \
                        (1 << IO_BOARD_INB_##_n) : 0;

    #define SET_LED_STATE(_n) \
                (B->outbData & (1 << IO_BOARD_OUTB_LED_##_n))? \
                        BOARD_LED_ON(_n) : BOARD_LED_OFF(_n);

    #define SET_GPIO_STATE(_n) \
                (B->outbData & (1 << IO_BOARD_OUTB_##_n))? \
                        BOARD_GPIO_SET_STATE(OUT_##_n, ENABLED) : \
                        BOARD_GPIO_SET_STATE(OUT_##_n, DISABLED);

    // Input
    GET_SWITCH_STATE(ISP);
    getWakeupState      (B);
    getBoardBacklight   (B);
    getSdPower          (B);
    GET_GPIO_STATE(WIFI_EN);
    GET_GPIO_STATE(SOUND_MUTE);

    // Output
    SET_LED_STATE(WARN);
    setBoardBacklight   (B);
    Board_SetSdPower    (B->outbData & (1 << IO_BOARD_OUTB_SD_POW));
    SET_GPIO_STATE(WIFI_EN);
    SET_GPIO_STATE(SOUND_MUTE);

    #undef SET_GPIO_STATE
    #undef SET_LED_STATE
    #undef GET_GPIO_STATE
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

    (void) OutputSource;

    struct IO_BOARD *const B = (struct IO_BOARD *) Io;

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

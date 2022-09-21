/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] huewheel logic device.

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

#include "embedul.ar/source/drivers/io_huewheel.h"
#include "embedul.ar/source/core/misc/hsl.h"
#include "embedul.ar/source/core/device/board.h"


#define HUEWHEEL_DEFAULT_STEP           0x0280


static const char * s_OutputNamesBit[] =
{
    "dir"
};


static const char * s_OutputNamesRange[] =
{
    "step", "maxlum", "phase", "duration"
};


static void         update              (struct IO *const Io);
static IO_Count     availableOutputs    (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t OutputSource);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t OutputSource,
                                         const uint32_t Value);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);


static const struct IO_IFACE IO_HUEWHEEL_IFACE =
{
    .Description        = "Huewheel-based marquee",
    .Update             = update,
    .AvailableInputs    = UNSUPPORTED,
    .AvailableOutputs   = availableOutputs,
    .GetInput           = UNSUPPORTED,
    .SetOutput          = setOutput,
    .InputName          = UNSUPPORTED,
    .OutputName         = outputName
};


static void animReset (struct IO_HUEWHEEL *const H)
{
    H->currentOffset    = 0;
    H->step             = HUEWHEEL_DEFAULT_STEP;
    H->dir              = HUEWHEEL_Dir_CounterCW;

    ANIM_SetValue (&H->globalSaturation, HSL_SATURATION_FULL);
    ANIM_SetValue (&H->globalLuminance, HSL_LUMINANCE_FULLSAT);
}


static void animFlash (struct IO_HUEWHEEL *const H, const uint32_t Duration)
{
    const uint32_t CurrentLuminance = ANIM_GetValue (&H->globalLuminance);

    ANIM_Start (&H->globalLuminance, ANIM_Type_PingPong,
                CurrentLuminance, H->flashMaxLuminance, 0, H->flashPhase, 
                Duration, 0);
}


void IO_HUEWHEEL_Init (struct IO_HUEWHEEL *const H,
                       struct IO *const OutputDriver,
                       const uint32_t OutputDriverSource,
                       const struct HUEWHEEL_PolarPlacement
                       *const PolarPlacement,
                       const uint32_t PolarPlacementElements)
{
    BOARD_AssertParams (H && IO_Initialized(OutputDriver) && 
                         PolarPlacement && PolarPlacementElements);

    DEVICE_IMPLEMENTATION_Clear (H);

    H->outputDriver             = OutputDriver;
    H->outputDriverSource       = OutputDriverSource;
    H->polarPlacement           = PolarPlacement;
    H->polarPlacementElements   = PolarPlacementElements;

    animReset (H);

    // Update once per frame (~60 Hz) by default.
    IO_Init ((struct IO *)H, &IO_HUEWHEEL_IFACE, 15);
};


void IO_HUEWHEEL_Attach (struct IO_HUEWHEEL *const H)
{
    BOARD_AssertParams (H);

    OUTPUT_SetDevice ((struct IO *)H, 0);

    OUTPUT_MapBit   (OUTPUT_Bit_MarqueeDir, IO_HUEWHEEL_OUTB_Dir);
    OUTPUT_MapRange (OUTPUT_Range_MarqueeStep, IO_HUEWHEEL_OUTR_Step);
    OUTPUT_MapRange (OUTPUT_Range_MarqueeFlashMaxLuminance, 
                     IO_HUEWHEEL_OUTR_MaxLuminance);
    OUTPUT_MapRange (OUTPUT_Range_MarqueeFlashPhase, IO_HUEWHEEL_OUTR_Phase);
    OUTPUT_MapRange (OUTPUT_Range_MarqueeFlashDuration,
                     IO_HUEWHEEL_OUTR_Duration);
}


static void update (struct IO *const Io)
{
    struct IO_HUEWHEEL *const H = (struct IO_HUEWHEEL *) Io;

    for (uint32_t i = 0; i < H->polarPlacementElements; ++i)
    {
        const struct HUEWHEEL_PolarPlacement * Pp = &H->polarPlacement[i];

        const uint8_t Hue = ((Pp->PolarPlacement << 8) + H->currentOffset) >> 8;

        const struct RGB888 RGB = HSL_ToRgb (Hue, 
                                        ANIM_GetValue(&H->globalSaturation),
                                        ANIM_GetValue(&H->globalLuminance));

        IO_SetOutput (H->outputDriver, IO_Type_Range,
                        Pp->OutputIndexRed,
                        H->outputDriverSource,
                        RGB.r,
                        IO_UpdateValue_Async);

        IO_SetOutput (H->outputDriver, IO_Type_Range,
                        Pp->OutputIndexGreen,
                        H->outputDriverSource,
                        RGB.g,
                        IO_UpdateValue_Async);

        IO_SetOutput (H->outputDriver, IO_Type_Range,
                        Pp->OutputIndexBlue,
                        H->outputDriverSource,
                        RGB.b,
                        IO_UpdateValue_Async);
    }

    IO_Update (H->outputDriver);

    H->currentOffset = (H->dir == HUEWHEEL_Dir_Clockwise)?
                                    H->currentOffset + H->step :
                                    H->currentOffset - H->step;
}


static IO_Count availableOutputs (struct IO *const Io, const enum IO_Type IoType,
                                  const uint32_t OutputSource)
{
    (void) Io;
    (void) OutputSource;

    return (IoType == IO_Type_Bit)? IO_HUEWHEEL_OUTB__COUNT : 
                                    IO_HUEWHEEL_OUTR__COUNT;
}


static void setOutput (struct IO *const Io, const enum IO_Type IoType,
                       const uint16_t Index, const uint32_t OutputSource,
                       const uint32_t Value)
{
    (void) OutputSource;

    struct IO_HUEWHEEL *const H = (struct IO_HUEWHEEL *) Io;

    if (IoType == IO_Type_Bit)
    {
        BOARD_AssertParams (Index < IO_HUEWHEEL_OUTB__COUNT);

        switch (Index)
        {
            case IO_HUEWHEEL_OUTB_Dir:
                H->dir = Value? HUEWHEEL_Dir_CounterCW : 
                                HUEWHEEL_Dir_Clockwise;
                break;
        }
    }
    else 
    {
        BOARD_AssertParams (Index < IO_HUEWHEEL_OUTR__COUNT);

        switch (Index)
        {
            case IO_HUEWHEEL_OUTR_Step:
                H->step = Value;
                break;

            case IO_HUEWHEEL_OUTR_MaxLuminance:
                H->flashMaxLuminance = Value;
                break;

            case IO_HUEWHEEL_OUTR_Phase:
                H->flashPhase = Value;
                break;

            case IO_HUEWHEEL_OUTR_Duration:
                animFlash (H, Value);
                break;
        }
    }
}


static const char * outputName (struct IO *const Io, const enum IO_Type IoType,
                                const uint16_t Index)
{
    (void) Io;

    if (IoType == IO_Type_Bit)
    {
        BOARD_AssertParams (Index < IO_HUEWHEEL_OUTB__COUNT);
        return s_OutputNamesBit[Index];
    }

    BOARD_AssertParams (Index < IO_HUEWHEEL_OUTR__COUNT);
    return s_OutputNamesRange[Index];
}

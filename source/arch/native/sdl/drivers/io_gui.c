/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] SDL GUI.

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

#include "embedul.ar/source/arch/native/sdl/drivers/io_gui.h"
#include "embedul.ar/source/core/manager/screen/rect.h"
#include "embedul.ar/source/core/manager/screen/line.h"
#include "embedul.ar/source/core/device/board.h"


#define IO_GUI_ELEMENT_DEFAULT_BOFF     0x00
#define IO_GUI_ELEMENT_DEFAULT_TOFF     0xFF
#define IO_GUI_ELEMENT_DEFAULT_BON      0x88
#define IO_GUI_ELEMENT_DEFAULT_TON      0xFF


#define SET_IO_GUI_ELEMENT(_n,_g,_bon,_ton) \
    .Name = _n, \
    .Gradient = _g, \
    .BackOn = _bon, \
    .TextOn = _ton


#define SET_IO_GUI_ELEMENT_DEF(_n) \
    SET_IO_GUI_ELEMENT(_n,NULL, \
        IO_GUI_ELEMENT_DEFAULT_BON, \
        IO_GUI_ELEMENT_DEFAULT_TON)


const char *const s_GUIFunctionName[IO_GUI_Function__COUNT] =
{
    [IO_GUI_Function_Input]     = "Input",
    [IO_GUI_Function_Output]    = "Output"
};


struct ButtonState
{
    const RGB332_Select Back;
    const RGB332_Select BorderNW;
    const RGB332_Select BorderSE;
};


static const struct ButtonState s_Button[2] =
{
    [0] = { .Back = 0x2d, .BorderNW = 0x50, .BorderSE = 0x49 },
    [1] = { .Back = 0x88, .BorderNW = 0x49, .BorderSE = 0xa8 }
};


struct IO_GUI_Element 
{
    const char              * const Name;
    struct RGB332_Gradient  * const Gradient;
    const RGB332_Select     BackOn;
    const RGB332_Select     TextOn;
};


const struct IO_GUI_Element s_InputBitNames[IO_GUI_INB__COUNT] = 
{
    [IO_GUI_INB_ControlBacklight] =
        {
            SET_IO_GUI_ELEMENT_DEF("Backlight")
        },
    [IO_GUI_INB_ControlStoragePower] =
        {
            SET_IO_GUI_ELEMENT_DEF("Storage power")
        },
    [IO_GUI_INB_ControlStorageDetect] =
        {
            SET_IO_GUI_ELEMENT_DEF("Storage detect")
        },
    [IO_GUI_INB_ControlWirelessEnable] =
        {
            SET_IO_GUI_ELEMENT_DEF("Wireless enable")
        },
    [IO_GUI_INB_ControlSoundMute] =
        {
            SET_IO_GUI_ELEMENT_DEF("Sound mute")
        },
    [IO_GUI_INB_Gp1Right] =
        {
            SET_IO_GUI_ELEMENT_DEF("Right")
        },
    [IO_GUI_INB_Gp1Left] =
        {
            SET_IO_GUI_ELEMENT_DEF("Left")
        },
    [IO_GUI_INB_Gp1Down] =
        {
            SET_IO_GUI_ELEMENT_DEF("Down")
        },
    [IO_GUI_INB_Gp1Up] =
        {
            SET_IO_GUI_ELEMENT_DEF("Up")
        },
    [IO_GUI_INB_Gp1Start] =
        {
            SET_IO_GUI_ELEMENT_DEF("Start")
        },
    [IO_GUI_INB_Gp1Select] =
        {
            SET_IO_GUI_ELEMENT_DEF("Select")
        },
    [IO_GUI_INB_Gp1A] =
        {
            SET_IO_GUI_ELEMENT_DEF("A")
        },
    [IO_GUI_INB_Gp1B] =
        {
            SET_IO_GUI_ELEMENT_DEF("B")
        },
    [IO_GUI_INB_Gp1C] =
        {
            SET_IO_GUI_ELEMENT_DEF("C")
        },
    [IO_GUI_INB_Gp1X] =
        {
            SET_IO_GUI_ELEMENT_DEF("X")
        },
    [IO_GUI_INB_Gp1Y] =
        {
            SET_IO_GUI_ELEMENT_DEF("Y")
        },
    [IO_GUI_INB_Gp1Z] =
        {
            SET_IO_GUI_ELEMENT_DEF("Z")
        },
    [IO_GUI_INB_Gp2Right] =
        {
            SET_IO_GUI_ELEMENT_DEF("Right")
        },
    [IO_GUI_INB_Gp2Left] =
        {
            SET_IO_GUI_ELEMENT_DEF("Left")
        },
    [IO_GUI_INB_Gp2Down] =
        {
            SET_IO_GUI_ELEMENT_DEF("Down")
        },
    [IO_GUI_INB_Gp2Up] =
        {
            SET_IO_GUI_ELEMENT_DEF("Up")
        },
    [IO_GUI_INB_Gp2Start] =
        {
            SET_IO_GUI_ELEMENT_DEF("Start")
        },
    [IO_GUI_INB_Gp2Select] =
        {
            SET_IO_GUI_ELEMENT_DEF("Select")
        },
    [IO_GUI_INB_Gp2A] =
        {
            SET_IO_GUI_ELEMENT_DEF("A")
        },
    [IO_GUI_INB_Gp2B] =
        {
            SET_IO_GUI_ELEMENT_DEF("B")
        },
    [IO_GUI_INB_Gp2C] =
        {
            SET_IO_GUI_ELEMENT_DEF("C")
        },
    [IO_GUI_INB_Gp2X] =
        {
            SET_IO_GUI_ELEMENT_DEF("X")
        },
    [IO_GUI_INB_Gp2Y] =
        {
            SET_IO_GUI_ELEMENT_DEF("Y")
        },
    [IO_GUI_INB_Gp2Z] =
        {
            SET_IO_GUI_ELEMENT_DEF("Z")
        },
    [IO_GUI_INB_LightdevOvertemp__BEGIN] =
        {
            .Name = NULL
        },
    [IO_GUI_INB_LightdevChannelError__BEGIN] =
        {
            .Name = NULL
        },
    [IO_GUI_INB_MainA] =
        {
            SET_IO_GUI_ELEMENT_DEF("A")
        },
    [IO_GUI_INB_MainB] =
        {
            SET_IO_GUI_ELEMENT_DEF("B")
        },
    [IO_GUI_INB_MainC] =
        {
            SET_IO_GUI_ELEMENT_DEF("C")
        },
    [IO_GUI_INB_MainD] =
        {
            SET_IO_GUI_ELEMENT_DEF("D")
        },
    [IO_GUI_INB_MainPointerPressed] =
        {
            SET_IO_GUI_ELEMENT_DEF("Pointer pressed")
        }
};


const struct IO_GUI_Element s_InputRangeNames[IO_GUI_INR__COUNT] = 
{
    [IO_GUI_INR_LightdevChannelsShorted__BEGIN] =
        {
            .Name = NULL
        },
    [IO_GUI_INR_LightdevChannelsOpen__BEGIN] =
        {
            .Name = NULL
        },
    [IO_GUI_INR_MainPointerX] =
        {
            SET_IO_GUI_ELEMENT_DEF("Pointer X")
        },
    [IO_GUI_INR_MainPointerY] =
        {
            SET_IO_GUI_ELEMENT_DEF("Pointer Y")
        },
    [IO_GUI_INR_MainPointerOverScreenType]  =
        {
            SET_IO_GUI_ELEMENT_DEF("Pointer over screen type")
        }
};


const struct IO_GUI_Element s_OutputBitNames[IO_GUI_OUTB__COUNT] = 
{
    [IO_GUI_OUTB_ControlBacklight] =
        {
            SET_IO_GUI_ELEMENT_DEF("Backlight")
        },
    [IO_GUI_OUTB_ControlStoragePower] =
        {
            SET_IO_GUI_ELEMENT_DEF("Storage power")
        },
    [IO_GUI_OUTB_ControlStorageEnable] =
        {
            SET_IO_GUI_ELEMENT_DEF("Storage enable")
        },
    [IO_GUI_OUTB_ControlWirelessEnable] =
        {
            SET_IO_GUI_ELEMENT_DEF("Wireless enable")
        },
    [IO_GUI_OUTB_ControlSoundMute] =
        {
            SET_IO_GUI_ELEMENT_DEF("Sound mute")
        },
    [IO_GUI_OUTB_MarqueeDir] =
        {
            SET_IO_GUI_ELEMENT_DEF("Dir")
        },
    [IO_GUI_OUTB_SignWarning] =
        {
            SET_IO_GUI_ELEMENT_DEF("Warning")
        },
    [IO_GUI_OUTB_SignRed] =
        {
            SET_IO_GUI_ELEMENT_DEF("Red")
        },
    [IO_GUI_OUTB_SignGreen] =
        {
            SET_IO_GUI_ELEMENT_DEF("Green")
        },
    [IO_GUI_OUTB_SignBlue] =
        {
            SET_IO_GUI_ELEMENT_DEF("Blue")
        },
};


const struct IO_GUI_Element s_OutputRangeNames[IO_GUI_OUTR__COUNT] = 
{
    [IO_GUI_OUTR_LightdevIref__BEGIN] =
        { 
            .Name = NULL
        },
    [IO_GUI_OUTR_LightdevPwm__BEGIN] =
        {
            .Name = NULL
        },
    [IO_GUI_OUTR_MarqueeStep] =
        {
            SET_IO_GUI_ELEMENT_DEF("Step")
        },
    [IO_GUI_OUTR_MarqueeFlashMaxLuminance] =
        {
            SET_IO_GUI_ELEMENT_DEF("Flash max luminance")
        },
    [IO_GUI_OUTR_MarqueeFlashPhase] =
        {
            SET_IO_GUI_ELEMENT_DEF("Flash phase")
        },
    [IO_GUI_OUTR_MarqueeFlashDuration]  =
        {
            SET_IO_GUI_ELEMENT_DEF("Flash duration")
        }
};


const struct IO_GUI_Element *const
    s_IONames[IO_GUI_Function__COUNT][IO_Type__COUNT] =
{
    [IO_GUI_Function_Input][IO_Type_Bit]    = s_InputBitNames,
    [IO_GUI_Function_Input][IO_Type_Range]  = s_InputRangeNames,
    [IO_GUI_Function_Output][IO_Type_Bit]   = s_OutputBitNames,
    [IO_GUI_Function_Output][IO_Type_Range] = s_OutputRangeNames
};


static void         update              (struct IO *const Io);
static IO_Value     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port,
                                         const IO_Value Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_GUI_IFACE =
{
    IO_IFACE_DECLARE("sdl gui", GUI),
    .Update         = update,
    .GetInput       = getInput,
    .SetOutput      = setOutput,
    .InputName      = inputName,
    .OutputName     = outputName
};


inline static void resetShowSelectedElement (struct IO_GUI *const G)
{
    G->showElementPage      = 0;
    G->showElementSelected  = 0;
}


void IO_GUI_Init (struct IO_GUI *const G, const enum SCREEN_Role Screen)
{
    BOARD_AssertParams      (G);
    BOARD_AssertInitialized (SCREEN_IsAvailable(Screen));

    DEVICE_IMPLEMENTATION_Clear (G);

    IO_INIT_STATIC_PORT_INFO (G, GUI);

    G->screen = Screen;

    resetShowSelectedElement (G);

    G->showType         = IO_Type_Bit;
    G->showFunction     = IO_GUI_Function_Output;
    G->showProfile[IO_GUI_Function_Output]
                        = OUTPUT_PROFILE_Type_SIGN;
    G->showProfile[IO_GUI_Function_Input]
                        = INPUT_PROFILE_Type_MAIN;

    BITFIELD_Init (&G->inBitfield, G->inBitBuffer,
                   BITFIELD_COUNT(IO_GUI_INB__COUNT), NULL, 0);
    BITFIELD_Init (&G->outBitfield, G->outBitBuffer,
                   BITFIELD_COUNT(IO_GUI_OUTB__COUNT), NULL, 0);

    memset (G->inRanges, 0, sizeof(G->inRanges));
    memset (G->outRanges, 0, sizeof(G->outRanges));

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)G, &IO_GUI_IFACE, G->portInfo, 15);
}


void IO_GUI_Attach (struct IO_GUI *const G)
{
    INPUT_RegisterGateway ((struct IO *) G, 0);

    INPUT_MAP_BIT (CONTROL, Backlight, IO_GUI_INB_ControlBacklight);
    INPUT_MAP_BIT (CONTROL, StoragePower, IO_GUI_INB_ControlStoragePower);
    INPUT_MAP_BIT (CONTROL, StorageDetect, IO_GUI_INB_ControlStorageDetect);
    INPUT_MAP_BIT (CONTROL, WirelessEnable, IO_GUI_INB_ControlWirelessEnable);
    INPUT_MAP_BIT (CONTROL, SoundMute, IO_GUI_INB_ControlSoundMute);

    INPUT_MAP_BIT (GP1, Right, IO_GUI_INB_Gp1Right);
    INPUT_MAP_BIT (GP1, Left, IO_GUI_INB_Gp1Left);
    INPUT_MAP_BIT (GP1, Down, IO_GUI_INB_Gp1Down);
    INPUT_MAP_BIT (GP1, Up, IO_GUI_INB_Gp1Up);
    INPUT_MAP_BIT (GP1, Start, IO_GUI_INB_Gp1Start);
    INPUT_MAP_BIT (GP1, Select, IO_GUI_INB_Gp1Select);
    INPUT_MAP_BIT (GP1, A, IO_GUI_INB_Gp1A);
    INPUT_MAP_BIT (GP1, B, IO_GUI_INB_Gp1B);
    INPUT_MAP_BIT (GP1, C, IO_GUI_INB_Gp1C);
    INPUT_MAP_BIT (GP1, X, IO_GUI_INB_Gp1X);
    INPUT_MAP_BIT (GP1, Y, IO_GUI_INB_Gp1Y);
    INPUT_MAP_BIT (GP1, Z, IO_GUI_INB_Gp1Z);

    INPUT_MAP_BIT (GP2, Right, IO_GUI_INB_Gp2Right);
    INPUT_MAP_BIT (GP2, Left, IO_GUI_INB_Gp2Left);
    INPUT_MAP_BIT (GP2, Down, IO_GUI_INB_Gp2Down);
    INPUT_MAP_BIT (GP2, Up, IO_GUI_INB_Gp2Up);
    INPUT_MAP_BIT (GP2, Start, IO_GUI_INB_Gp2Start);
    INPUT_MAP_BIT (GP2, Select, IO_GUI_INB_Gp2Select);
    INPUT_MAP_BIT (GP2, A, IO_GUI_INB_Gp2A);
    INPUT_MAP_BIT (GP2, B, IO_GUI_INB_Gp2B);
    INPUT_MAP_BIT (GP2, C, IO_GUI_INB_Gp2C);
    INPUT_MAP_BIT (GP2, X, IO_GUI_INB_Gp2X);
    INPUT_MAP_BIT (GP2, Y, IO_GUI_INB_Gp2Y);
    INPUT_MAP_BIT (GP2, Z, IO_GUI_INB_Gp2Z);

    // Both Bit_Overtemp, Bit_ChannelError, RangeChannelsShorted, and
    // Range_ChannelsOpen share the same number of elements
    for (enum INPUT_PROFILE_LIGHTDEV_Bit i = 0;
         i <= INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__END -
              INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__BEGIN; ++i)
    {
        INPUT_Map (INPUT_PROFILE_Type_LIGHTDEV, IO_Type_Bit,
                   INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__BEGIN + i,
                   IO_GUI_INB_LightdevOvertemp__BEGIN + i);

        INPUT_Map (INPUT_PROFILE_Type_LIGHTDEV, IO_Type_Bit,
                   INPUT_PROFILE_LIGHTDEV_Bit_ChannelError__BEGIN + i,
                   IO_GUI_INB_LightdevChannelError__BEGIN + i);

        INPUT_Map (INPUT_PROFILE_Type_LIGHTDEV, IO_Type_Range,
                   INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted__BEGIN + i,
                   IO_GUI_INR_LightdevChannelsShorted__BEGIN + i);

        INPUT_Map (INPUT_PROFILE_Type_LIGHTDEV, IO_Type_Range,
                   INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen__BEGIN + i,
                   IO_GUI_INR_LightdevChannelsOpen__BEGIN + i);
    }

    INPUT_MAP_BIT (MAIN, A, IO_GUI_INB_MainA);
    INPUT_MAP_BIT (MAIN, B, IO_GUI_INB_MainB);
    INPUT_MAP_BIT (MAIN, C, IO_GUI_INB_MainC);
    INPUT_MAP_BIT (MAIN, D, IO_GUI_INB_MainD);
    INPUT_MAP_BIT (MAIN, PointerPressed, IO_GUI_INB_MainPointerPressed);
    INPUT_MAP_RANGE (MAIN, PointerX, IO_GUI_INR_MainPointerX);
    INPUT_MAP_RANGE (MAIN, PointerY, IO_GUI_INR_MainPointerY);
    INPUT_MAP_RANGE (MAIN, PointerOverScreenType,
                     IO_GUI_INR_MainPointerOverScreenType);


    OUTPUT_RegisterGateway ((struct IO *) G, 0);

    OUTPUT_MAP_BIT (CONTROL, Backlight, IO_GUI_OUTB_ControlBacklight);
    OUTPUT_MAP_BIT (CONTROL, StoragePower, IO_GUI_OUTB_ControlStoragePower);
    OUTPUT_MAP_BIT (CONTROL, StorageEnable, IO_GUI_OUTB_ControlStorageEnable);
    OUTPUT_MAP_BIT (CONTROL, WirelessEnable, IO_GUI_OUTB_ControlWirelessEnable);
    OUTPUT_MAP_BIT (CONTROL, SoundMute, IO_GUI_OUTB_ControlSoundMute);

    // Both Iref and Pwm share the same number of elements
    for (enum OUTPUT_PROFILE_LIGHTDEV_Range i = 0;
         i <= OUTPUT_PROFILE_LIGHTDEV_Range_Iref__END -
              OUTPUT_PROFILE_LIGHTDEV_Range_Iref__BEGIN; ++i)
    {
        OUTPUT_Map (OUTPUT_PROFILE_Type_LIGHTDEV, IO_Type_Range,
                    OUTPUT_PROFILE_LIGHTDEV_Range_Iref__BEGIN + i,
                    IO_GUI_OUTR_LightdevIref__BEGIN + i);

        OUTPUT_Map (OUTPUT_PROFILE_Type_LIGHTDEV, IO_Type_Range,
                    OUTPUT_PROFILE_LIGHTDEV_Range_Pwm__BEGIN + i,
                    IO_GUI_OUTR_LightdevPwm__BEGIN + i);
    }

    OUTPUT_MAP_BIT (MARQUEE, Dir, IO_GUI_OUTB_MarqueeDir);
    OUTPUT_MAP_RANGE (MARQUEE, Step, IO_GUI_OUTR_MarqueeStep);
    OUTPUT_MAP_RANGE (MARQUEE, FlashMaxLuminance,
                      IO_GUI_OUTR_MarqueeFlashMaxLuminance);
    OUTPUT_MAP_RANGE (MARQUEE, FlashPhase, IO_GUI_OUTR_MarqueeFlashPhase);
    OUTPUT_MAP_RANGE (MARQUEE, FlashDuration, IO_GUI_OUTR_MarqueeFlashDuration);

    OUTPUT_MAP_BIT (SIGN, Warning, IO_GUI_OUTB_SignWarning);
    OUTPUT_MAP_BIT (SIGN, Red, IO_GUI_OUTB_SignRed);
    OUTPUT_MAP_BIT (SIGN, Green, IO_GUI_OUTB_SignGreen);
    OUTPUT_MAP_BIT (SIGN, Blue, IO_GUI_OUTB_SignBlue);
}


void IO_GUI__setInputBit (struct IO_GUI *const G, const enum IO_GUI_INB Inb,
                         const bool Value)
{
    BITFIELD_SetBit (&G->inBitfield, Inb, Value);
}


void IO_GUI__toggleInputBit (struct IO_GUI *const G, const enum IO_GUI_INB Inb)
{
    const bool V = BITFIELD_GetBit (&G->inBitfield, Inb);
    BITFIELD_SetBit (&G->inBitfield, Inb, V? false : true);
}


void IO_GUI__setInputRange (struct IO_GUI *const G, const enum IO_GUI_INR Inr,
                           const uint32_t Value)
{
    G->inRanges[Inr] = Value;
}


static IO_Code getProfileCodes (struct IO_GUI *const G)
{
    const uint32_t ProfileType = G->showProfile[G->showFunction];

    const IO_Code Codes = (G->showFunction == IO_GUI_Function_Input)?
                           INPUT_ProfileCodes(ProfileType, G->showType) :
                           OUTPUT_ProfileCodes(ProfileType, G->showType);
    return Codes;
}


static IO_Value getSelfValue (struct IO_GUI *const G,
                              const uint32_t DriverCode)
{
    if (G->showType == IO_Type_Bit)
    {
        struct BITFIELD *const B =
            (G->showFunction == IO_GUI_Function_Input)? 
                &G->inBitfield : &G->outBitfield;

        return BITFIELD_GetBit (B, DriverCode);
    }

    return (G->showFunction == IO_GUI_Function_Input)?
                G->inRanges[DriverCode] : G->outRanges[DriverCode];
}


static void drawElementBackground (struct IO_GUI *const G,
                                   const int32_t X, const int32_t Y,
                                   const IO_Code ProfileCode,
                                   const char *const Caption,
                                   const RGB332_Select ElementColor)
{
    SCREEN_RECT_Draw        (G->screen, X, Y, 78*8, 3*8, ElementColor);
    SCREEN_FONT_DrawString  (G->screen, X+1*8, Y+1*8, 0xFF,
                             SCREEN_FONT_DP_Shadow, 0,
                             SCREEN_FONT_AutoMaxOctets(G->screen),
                             VARIANT_ToString(&VARIANT_SpawnAuto(ProfileCode)));
    SCREEN_FONT_DrawString  (G->screen, X+6*8, Y+1*8, 0xFF,
                             SCREEN_FONT_DP_Shadow, 0,
                             SCREEN_FONT_AutoMaxOctets(G->screen), Caption);
}


static void drawButton (struct IO_GUI *const G,
                        const int32_t X, const int32_t Y,
                        const int32_t W, const int32_t H,
                        const struct ButtonState *Button,
                        const char *const Caption)
{
    SCREEN_RECT_Draw        (G->screen, X, Y, W, H, Button->Back);
    SCREEN_LINE_DrawHoriz   (G->screen, X, Y, W, Button->BorderNW);
    SCREEN_LINE_DrawVert    (G->screen, X, Y, H, Button->BorderNW);
    SCREEN_LINE_DrawHoriz   (G->screen, X, Y+H, W, Button->BorderSE);
    SCREEN_LINE_DrawVert    (G->screen, X+W, Y, H, Button->BorderSE);

    SCREEN_FONT_DrawString  (G->screen, X+W/2, Y+H/2-4, 0xFF,
                             SCREEN_FONT_DP_Shadow |
                             SCREEN_FONT_DP_AlingHCenter, 0,
                             SCREEN_FONT_AutoMaxOctets(G->screen),
                             Caption);
}


static void drawSelfInputControls (struct IO_GUI *const G,
                                   const int32_t X, const int32_t Y,
                                   const bool LeftButtonPressed,
                                   const bool RightButtonPressed,
                                   const char *const LeftButtonCaption,
                                   const char *const RightButtonCaption)
{
    drawButton (G, X+37*8, Y, 4*8, 3*8,
                &s_Button[LeftButtonPressed], LeftButtonCaption);
    drawButton (G, X+42*8, Y, 4*8, 3*8,
                &s_Button[RightButtonPressed], RightButtonCaption);
}


static void updateRangeIncrement (struct IO_GUI *const G,
                                  const IO_Code SelfDriverCode,
                                  const int32_t Multiplier)
{
    if (!G->lastPointerPressed)
    {
        G->lastRangeTimeout = BOARD_TicksNow() + 1000;
        G->rangeIncrement   = 0;

        G->inRanges[SelfDriverCode] += 1 * Multiplier;
    }
    else
    {
        if (G->lastRangeTimeout < BOARD_TicksNow())
        {
            switch (G->rangeIncrement)
            {
                case 0:
                    G->rangeIncrement = 1;
                    G->lastRangeTimeout = BOARD_TicksNow() + 1500;
                    break;

                case 1:
                    G->rangeIncrement = 100;
                    G->lastRangeTimeout = BOARD_TicksNow() + 2000;
                    break;

                case 100:
                    G->rangeIncrement = 10000;
                    G->lastRangeTimeout = BOARD_TicksNow() + 2500;
                    break;

                case 10000:
                    G->rangeIncrement = 10000000;
                    break;
            }
        }

        if (G->rangeIncrement)
        {
            G->inRanges[SelfDriverCode] += G->rangeIncrement * Multiplier;
        }
    }
}


static void processSelfInputProfileCode (
                        struct IO_GUI *const G,
                        const int32_t X, const int32_t Y,
                        const IO_Code SelfDriverCode,
                        const uint32_t PointerX, const uint32_t PointerY,
                        const bool Pressed)
{
    static const char *const LeftButtonCaption[IO_Type__COUNT] =
    {
        [IO_Type_Bit]   = "S",
        [IO_Type_Range] = "<"
    };

    static const char *const RightButtonCaption[IO_Type__COUNT] =
    {
        [IO_Type_Bit]   = "T",
        [IO_Type_Range] = ">"
    };

    bool leftButtonPressed  = false;
    bool rightButtonPressed = false;

    // Check for button presses around the current element area
    if (Pressed &&
        (int32_t)PointerY >= Y &&
        (int32_t)PointerY <= Y + 3*8)
    {
        leftButtonPressed = ((int32_t)PointerX >= X+37*8 &&
                             (int32_t)PointerX <= X+37*8+4*8);

        rightButtonPressed = ((int32_t)PointerX >= X+42*8 &&
                              (int32_t)PointerX <= X+42*8+4*8);
    }

    if (leftButtonPressed)
    {
        if (G->showType == IO_Type_Bit)
        {
            G->lastBitCodePressed = SelfDriverCode;
            IO_GUI__setInputBit (G, SelfDriverCode, true);
        }
        else if (G->showType == IO_Type_Range)
        {
            updateRangeIncrement (G, SelfDriverCode, -1);
        }
    }
    else
    {
        if (G->showType == IO_Type_Bit)
        {
            if (G->lastBitCodePressed == SelfDriverCode)
            {
                // Keeps a value on only when pressing the "S" button
                // (a value set by the "T" button is persistent)
                IO_GUI__setInputBit (G, SelfDriverCode, false);
                G->lastBitCodePressed = IO_INVALID_CODE;
            }
        }
    }

    if (rightButtonPressed)
    {
        if (G->showType == IO_Type_Bit)
        {
            if (G->lastBitCodeToggled != SelfDriverCode)
            {
                IO_GUI__toggleInputBit (G, SelfDriverCode);
                G->lastBitCodeToggled = SelfDriverCode;
            }
        }
        else if (G->showType == IO_Type_Range)
        {
            updateRangeIncrement (G, SelfDriverCode, 1);
        }
    }
    else
    {
        if (G->showType == IO_Type_Bit)
        {
            if (G->lastBitCodeToggled == SelfDriverCode)
            {
                G->lastBitCodeToggled = IO_INVALID_CODE;
            }
        }
    }

    drawSelfInputControls (G, X, Y, leftButtonPressed, rightButtonPressed,
                           LeftButtonCaption[G->showType],
                           RightButtonCaption[G->showType]);
}


static void drawSelfProfileCodeValue (struct IO_GUI *const G,
                                      const int32_t X, const int32_t Y,
                                      const IO_Code ProfileCode,
                                      const char *const ValueName,
                                      const char *const Value,
                                      const RGB332_Select ValueBackColor,
                                      const RGB332_Select ValueTextColor)
{
    drawElementBackground (G, X, Y, ProfileCode, ValueName, 0x04);

    SCREEN_RECT_Draw        (G->screen, X+47*8, Y+1*8-4, 30*8, 1*8+8, 
                             ValueBackColor);
    SCREEN_FONT_DrawString  (G->screen, X+62*8, Y+1*8, ValueTextColor,
                             SCREEN_FONT_DP_Shadow |
                             SCREEN_FONT_DP_AlingHCenter, 0,
                             SCREEN_FONT_AutoMaxOctets(G->screen), Value);
}


static void processProfileCode (
                        struct IO_GUI *const G,
                         const uint32_t PointerX, const uint32_t PointerY,
                         const bool Pressed, const IO_Code ProfileCode,
                         const uint32_t ElementIndex)
{
    const int32_t   X           = 1*8;
    const int32_t   Y           = 7*8 + ElementIndex * 4*8;
    const uint32_t  ProfileType = G->showProfile[G->showFunction];

    bool IsMapped = (G->showFunction == IO_GUI_Function_Input)?
                     INPUT_IsMapped (ProfileType, G->showType, ProfileCode) :
                     OUTPUT_IsMapped (ProfileType, G->showType, ProfileCode);

    if (IsMapped)
    {
        const IO_Code SelfDriverCode = 
            (G->showFunction == IO_GUI_Function_Input)?
                INPUT__isMappedByDriver(ProfileType, G->showType, ProfileCode,
                                        &G->device) :
                OUTPUT__isMappedByDriver(ProfileType, G->showType, ProfileCode,
                                         &G->device);

        // Mapped to this driver
        if (SelfDriverCode != IO_INVALID_CODE)
        {
            RGB332_Select vBackColor = IO_GUI_ELEMENT_DEFAULT_BOFF;
            RGB332_Select vTextColor = IO_GUI_ELEMENT_DEFAULT_TOFF;

            const char *const ValueName =
                (G->showFunction == IO_GUI_Function_Input)?
                    inputName(&G->device, G->showType, SelfDriverCode) :
                    outputName(&G->device, G->showType, SelfDriverCode);

            const struct IO_GUI_Element *const E = 
                &s_IONames[G->showFunction][G->showType][SelfDriverCode];

            const IO_Value Value = getSelfValue (G, SelfDriverCode);
            
            // Bit enabled
            if (G->showType == IO_Type_Bit && Value)
            {
                vBackColor = E->Name? E->BackOn : IO_GUI_ELEMENT_DEFAULT_BON;
                vTextColor = E->Name? E->TextOn : IO_GUI_ELEMENT_DEFAULT_TON;
            }

            const enum VARIANT_Base Base = (G->showType == IO_Type_Bit)?
                                                VARIANT_Base_Hex_UpperSuffix :
                                                VARIANT_Base_Dec;

            drawSelfProfileCodeValue (
                        G, X, Y, ProfileCode, ValueName,
                        VARIANT_ToString(&VARIANT_SpawnBaseAuto(Base, Value)),
                        vBackColor, vTextColor);

            if (G->showFunction == IO_GUI_Function_Input)
            {
                processSelfInputProfileCode (G, X, Y, SelfDriverCode,
                                             PointerX, PointerY, Pressed);
            }
        }
        // Mapped to other driver
        else
        {
            const struct IO_ConstGateway Gateway = 
                (G->showFunction == IO_GUI_Function_Input)?
                    INPUT__getMappedGateway(ProfileType, G->showType,
                                            ProfileCode) :
                    OUTPUT__getMappedGateway(ProfileType, G->showType,
                                             ProfileCode);

            char driverDesc[64];

            snprintf (driverDesc, sizeof(driverDesc),
                        "Mapped to \"%s\", port %u",
                        OBJECT_Description((struct IO *)Gateway.Driver),
                        Gateway.DriverPort);

            drawElementBackground (G, X, Y, ProfileCode, driverDesc, 0x01);
        }
    }
    // Profile code not mapped
    else
    {
        drawElementBackground (G, X, Y, ProfileCode, "Not mapped", 0x20);
    }
}


static void drawMenuItem (struct IO_GUI *const G,
                          const uint32_t ItemNr,
                          const char *const Title,
                          const char *const Value,
                          const bool Pressed)
{
    const int32_t X = ItemNr * 20 * 8;

    SCREEN_RECT_Draw        (G->screen, X, 0, 20*8, 5*8, 0x04); //0x10);
    SCREEN_FONT_DrawString  (G->screen, X+10*8, 1*8, 0xFF,
                             SCREEN_FONT_DP_Shadow |
                             SCREEN_FONT_DP_AlingHCenter, 0,
                             SCREEN_FONT_AutoMaxOctets(G->screen), Title);

    if (Value)
    {
        drawButton (G, X+8, 3*8-3, 18*8, 2*8, &s_Button[Pressed], Value);
    }
    else 
    {
        SCREEN_FONT_DrawString  (G->screen, X+10*8, 3*8-3+2*8/2-4, 0xe0,
                                 SCREEN_FONT_DP_Shadow |
                                 SCREEN_FONT_DP_AlingHCenter, 0,
                                 SCREEN_FONT_AutoMaxOctets(G->screen),
                                 "N / A");
    }
}


inline static const char * getProfileName (const enum IO_GUI_Function Function,
                                           const uint32_t ProfileType)
{
    return (Function == IO_GUI_Function_Input)?
            INPUT_PROFILE_GetTypeName(ProfileType) :
            OUTPUT_PROFILE_GetTypeName(ProfileType);
}


static void processMenu (struct IO_GUI *const G, const uint32_t PointerX,
                         const uint32_t PointerY, const bool Pressed)
{
    uint32_t itemPressed = 0;

    if (Pressed && !G->lastPointerPressed)
    {
        const IO_Value ScreenType =
            getInput (&G->device, IO_Type_Range,
                      IO_GUI_INR_MainPointerOverScreenType, 0);

        if (ScreenType == G->screen)
        {
            if (PointerY < 5*8)
            {
                // Fourth item: Elements shown
                if (PointerX >= 20*8*3)
                {   
                    itemPressed = 4;

                    const IO_Code ProfileCodes = getProfileCodes (G);

                    G->showElementPage =
                        ((G->showElementPage + 1) * 13 < ProfileCodes)?
                            G->showElementPage + 1 : 0;
                }
                else 
                {
                    resetShowSelectedElement (G);

                    // First item: Input or Output
                    if (PointerX < 20*8*1)
                    {
                        itemPressed = 1;

                        G->showFunction =
                            (G->showFunction + 1 < IO_GUI_Function__COUNT)?
                                G->showFunction + 1 : 0;
                    }
                    // Second item: Profile
                    else if (PointerX < 20*8*2)
                    {
                        itemPressed = 2;

                        const uint32_t ProfileCount =
                            (G->showFunction == IO_GUI_Function_Input)?
                                INPUT_PROFILE_Type__COUNT :
                                OUTPUT_PROFILE_Type__COUNT;

                        G->showProfile[G->showFunction] = 
                            (G->showProfile[G->showFunction] + 1 < 
                            ProfileCount)?
                                G->showProfile[G->showFunction] + 1 : 0;
                    }
                    // Third item: Bit or Range
                    else if (PointerX < 20*8*3)
                    {
                        itemPressed = 3;

                        G->showType = 
                            (G->showType == IO_Type_Bit)?
                                IO_Type_Range : IO_Type_Bit;
                    }
                }
            }
        }
    }

    const IO_Code ProfileCodes = getProfileCodes (G);

    drawMenuItem (G, 0, "Function",
                  s_GUIFunctionName[G->showFunction],
                  itemPressed == 1);
    drawMenuItem (G, 1, "Profile",
                  getProfileName(G->showFunction,
                                            G->showProfile[G->showFunction]),
                  itemPressed == 2);
    drawMenuItem (G, 2, "Type",
                  G->showType == IO_Type_Bit? "Bit" : "Range",
                  itemPressed == 3);

    if (ProfileCodes)
    {
        char page[32];

        const uint32_t ElementsBegin    = G->showElementPage * 13;
        const uint32_t ElementsEnd      = ElementsBegin + 13 < ProfileCodes? 
                                            ElementsBegin + 13 - 1 :
                                            ProfileCodes - 1U;

        snprintf (page, sizeof(page), "%u-%u (%u total)",
                ElementsBegin, ElementsEnd, ProfileCodes);

        drawMenuItem (G, 3, "Element codes", page, itemPressed == 4);
    }
    else
    {
        drawMenuItem (G, 3, "Element codes", NULL, 0);
    }
}


void update (struct IO *const Io)
{
    struct IO_GUI *const G = (struct IO_GUI *) Io;

    SCREEN_ClearBack (G->screen, 0x00);

    const uint32_t PointerX = getInput (&G->device, IO_Type_Range,
                                        IO_GUI_INR_MainPointerX, 0);
    const uint32_t PointerY = getInput (&G->device, IO_Type_Range,
                                        IO_GUI_INR_MainPointerY, 0);

    const bool Pressed = getInput (&G->device, IO_Type_Bit,
                                   IO_GUI_INB_MainPointerPressed, 0)?
                                   true : false;

    processMenu (G, PointerX, PointerY, Pressed);

    // Selected profile codes (elements)
    const IO_Code ProfileCodes = getProfileCodes (G);

    for (uint32_t elementIndex = 0; elementIndex < 13; ++ elementIndex)
    {
        const IO_Code ProfileCode = G->showElementPage * 13 + elementIndex;
        if (ProfileCode >= ProfileCodes)
        {
            break;
        }

        processProfileCode (G, PointerX, PointerY, Pressed, ProfileCode,
                            elementIndex);
    }

    G->lastPointerPressed = Pressed;
}


IO_Value getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) Port;

    struct IO_GUI *const G = (struct IO_GUI *) Io;

    if (IoType == IO_Type_Bit)
    {
        return BITFIELD_GetBit (&G->inBitfield, DriverCode);
    }

    return G->inRanges[DriverCode];
}


static void setOutput (struct IO *const Io, const enum IO_Type IoType,
                       const IO_Code DriverCode, const IO_Port Port,
                       const IO_Value Value)
{
    (void) Port;

    struct IO_GUI *const G = (struct IO_GUI *) Io;

    if (IoType == IO_Type_Bit)
    {
        BITFIELD_SetBit (&G->outBitfield, DriverCode, Value? true : false);
        return;
    }

    G->outRanges[DriverCode] = Value;
}


static void setNameRegionBuffer (struct IO_GUI *const G,
                                 const char *const NameFmt,
                                 const uint32_t Index)
{
    snprintf (G->nameRegionBuffer, sizeof(G->nameRegionBuffer), NameFmt, Index);
}


const char * inputName (struct IO *const Io,
                        const enum IO_Type IoType,
                        const IO_Code DriverCode)
{
    struct IO_GUI *const G = (struct IO_GUI *) Io;

    if (IoType == IO_Type_Bit)
    {
        if (s_InputBitNames[DriverCode].Name)
        {
            return s_InputBitNames[DriverCode].Name;
        }

        if (DriverCode >= IO_GUI_INB_LightdevOvertemp__BEGIN &&
            DriverCode <= IO_GUI_INB_lightdevOvertemp__END)
        {
            setNameRegionBuffer (G, "Device %u overtemp.",
                        DriverCode - IO_GUI_INB_LightdevOvertemp__BEGIN);
        }
        else if (DriverCode >= IO_GUI_INB_LightdevChannelError__BEGIN &&
                 DriverCode <= IO_GUI_INB_lightdevChannelError__END)
        {
            setNameRegionBuffer (G, "Device %u channel(s) error",
                        DriverCode - IO_GUI_INB_LightdevChannelError__BEGIN);
        }
        else
        {
            BOARD_AssertState ("Invalid driver code segment");
        }

        return G->nameRegionBuffer;
    }

    // IO_Type_Range
    if (s_InputRangeNames[DriverCode].Name)
    {
        return s_InputRangeNames[DriverCode].Name;
    }

    if (DriverCode >= IO_GUI_INR_LightdevChannelsShorted__BEGIN &&
        DriverCode <= IO_GUI_INR_LightdevChannelsShorted__END)
    {
        setNameRegionBuffer (G, "Device %u channels shorted",
                    DriverCode - IO_GUI_INR_LightdevChannelsShorted__BEGIN);
    }
    else if (DriverCode >= IO_GUI_INR_LightdevChannelsOpen__BEGIN &&
             DriverCode <= IO_GUI_INR_LightdevChannelsOpen__END)
    {
        setNameRegionBuffer (G, "Device %u channels open",
                    DriverCode - IO_GUI_INR_LightdevChannelsOpen__BEGIN);
    }
    else
    {
        BOARD_AssertState ("Invalid driver code segment");
    }

    return G->nameRegionBuffer;
}


static const char * outputName (struct IO *const Io, const enum IO_Type IoType,
                                const IO_Code DriverCode)
{
    struct IO_GUI *const G = (struct IO_GUI *) Io;

    if (IoType == IO_Type_Bit)
    {
        // No output bit regions
        return s_OutputBitNames[DriverCode].Name;
    }

    if (s_OutputRangeNames[DriverCode].Name)
    {
        return s_OutputRangeNames[DriverCode].Name;
    }

    if (DriverCode >= IO_GUI_OUTR_LightdevIref__BEGIN &&
        DriverCode <= IO_GUI_OUTR_LightdevIref__END)
    {
        setNameRegionBuffer (G, "Channel %u Iref",
                    DriverCode - IO_GUI_OUTR_LightdevIref__BEGIN);
    }
    else if (DriverCode >= IO_GUI_OUTR_LightdevPwm__BEGIN &&
             DriverCode <= IO_GUI_OUTR_LightdevPwm__END)
    {
        setNameRegionBuffer (G, "Channel %u PWM",
                    DriverCode - IO_GUI_OUTR_LightdevPwm__BEGIN);
    }
    else
    {
        BOARD_AssertState ("Invalid driver code segment");
    }

    return G->nameRegionBuffer;
}

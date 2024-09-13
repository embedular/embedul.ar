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

#pragma once

#include "embedul.ar/source/core/manager/mio.h"
#include "embedul.ar/source/core/manager/screen/role.h"
#include "embedul.ar/source/core/bitfield.h"
#include "embedul.ar/source/core/cyclic.h"


#define IO_GUI_PORT_COUNT      1


// Keep it sync'ed with input profiles and GUI controls
enum IO_GUI_INB
{
    IO_GUI_INB_ControlBacklight,
    IO_GUI_INB_ControlStoragePower,
    IO_GUI_INB_ControlStorageDetect,
    IO_GUI_INB_ControlWirelessEnable,
    IO_GUI_INB_ControlSoundMute,
    IO_GUI_INB_Gp1Right,
    IO_GUI_INB_Gp1Left,
    IO_GUI_INB_Gp1Down,
    IO_GUI_INB_Gp1Up,
    IO_GUI_INB_Gp1Start,
    IO_GUI_INB_Gp1Select,
    IO_GUI_INB_Gp1A,
    IO_GUI_INB_Gp1B,
    IO_GUI_INB_Gp1C,
    IO_GUI_INB_Gp1X,
    IO_GUI_INB_Gp1Y,
    IO_GUI_INB_Gp1Z,
    IO_GUI_INB_Gp2Right,
    IO_GUI_INB_Gp2Left,
    IO_GUI_INB_Gp2Down,
    IO_GUI_INB_Gp2Up,
    IO_GUI_INB_Gp2Start,
    IO_GUI_INB_Gp2Select,
    IO_GUI_INB_Gp2A,
    IO_GUI_INB_Gp2B,
    IO_GUI_INB_Gp2C,
    IO_GUI_INB_Gp2X,
    IO_GUI_INB_Gp2Y,
    IO_GUI_INB_Gp2Z,
    IO_GUI_INB_LightdevOvertemp__BEGIN,
    IO_GUI_INB_lightdevOvertemp__END =
                        IO_GUI_INB_LightdevOvertemp__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    IO_GUI_INB_LightdevChannelError__BEGIN,
    IO_GUI_INB_lightdevChannelError__END =
                        IO_GUI_INB_LightdevChannelError__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    IO_GUI_INB_MainA,
    IO_GUI_INB_MainB,
    IO_GUI_INB_MainC,
    IO_GUI_INB_MainD,
    IO_GUI_INB_MainPointerPressed,
    IO_GUI_INB__COUNT
};


enum IO_GUI_INR
{
    IO_GUI_INR_MainPointerX,
    IO_GUI_INR_MainPointerY,
    IO_GUI_INR_MainPointerOverScreenType,
    IO_GUI_INR_LightdevChannelsShorted__BEGIN,
    IO_GUI_INR_LightdevChannelsShorted__END =
                        IO_GUI_INR_LightdevChannelsShorted__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    IO_GUI_INR_LightdevChannelsOpen__BEGIN,
    IO_GUI_INR_LightdevChannelsOpen__END =
                        IO_GUI_INR_LightdevChannelsOpen__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    IO_GUI_INR__COUNT
};


// Keep it sync'ed with output profiles and GUI controls
enum IO_GUI_OUTB
{
    IO_GUI_OUTB_ControlBacklight,
    IO_GUI_OUTB_ControlStoragePower,
    IO_GUI_OUTB_ControlStorageEnable,
    IO_GUI_OUTB_ControlWirelessEnable,
    IO_GUI_OUTB_ControlSoundMute,
    IO_GUI_OUTB_MarqueeDir,
    IO_GUI_OUTB_SignWarning,
    IO_GUI_OUTB_SignRed,
    IO_GUI_OUTB_SignGreen,
    IO_GUI_OUTB_SignBlue,
    IO_GUI_OUTB__COUNT
};


enum IO_GUI_OUTR
{
    IO_GUI_OUTR_MarqueeStep,
    IO_GUI_OUTR_MarqueeFlashMaxLuminance,
    IO_GUI_OUTR_MarqueeFlashPhase,
    IO_GUI_OUTR_MarqueeFlashDuration,
    IO_GUI_OUTR_LightdevIref__BEGIN,
    IO_GUI_OUTR_LightdevIref__END =
                        IO_GUI_OUTR_LightdevIref__BEGIN +
                        LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS - 1,
    IO_GUI_OUTR_LightdevPwm__BEGIN,
    IO_GUI_OUTR_LightdevPwm__END =
                        IO_GUI_OUTR_LightdevPwm__BEGIN +
                        LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS - 1,
    IO_GUI_OUTR__COUNT
};


struct IO_GUI
{
    struct IO               device;
    struct IO_PortInfo      portInfo[IO_GUI_PORT_COUNT];
    enum SCREEN_Role        screen;
    uint32_t                inBitBuffer[BITFIELD_COUNT(IO_GUI_INB__COUNT)];
    uint32_t                outBitBuffer[BITFIELD_COUNT(IO_GUI_OUTB__COUNT)];
    IO_Value                inRanges[IO_GUI_INR__COUNT];
    IO_Value                outRanges[IO_GUI_OUTR__COUNT];
    struct BITFIELD         inBitfield;
    struct BITFIELD         outBitfield;
    char                    nameRegionBuffer[32];
    struct CYCLIC           nameRegionBufferCyclic;
    bool                    lastPointerPressed;
    TIMER_Ticks             lastRangeTimeout;
    uint32_t                rangeIncrement;
    IO_Code                 lastBitCodePressed;
    IO_Code                 lastBitCodeToggled;
    enum MIO_Dir            showDir;
    uint32_t                showProfile[MIO_Dir__COUNT];
    enum IO_Type            showType;
    uint32_t                showElementPage;
    uint32_t                showElementSelected;
};


void    IO_GUI_Init             (struct IO_GUI *const G,
                                 const enum SCREEN_Role Screen);
void    IO_GUI_Attach           (struct IO_GUI *const G);
// Some of the input values depend on the SDL message pump, not the GUI. The
// following functions allows to set input values while processing the message
// pump from outside the driver. 
void    IO_GUI__setInputBit     (struct IO_GUI *const G,
                                 const enum IO_GUI_INB Inb,
                                 const bool Value);
void    IO_GUI__toggleInputBit  (struct IO_GUI *const G,
                                 const enum IO_GUI_INB Inb);
void    IO_GUI__setInputRange   (struct IO_GUI *const G,
                                 const enum IO_GUI_INR Inr,
                                 const uint32_t Value);

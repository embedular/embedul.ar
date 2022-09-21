/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] output devices manager.

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

#include "embedul.ar/source/core/device/io.h"
#include <stdint.h>


#define OUTPUT_MAX_DEVICES      LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_DEVICES


#define OUTPUT_BIT(_inbn) \
            OUTPUT_Bit(OUTPUT_Bit_ ## _inbn)


enum OUTPUT_Bit
{
    OUTPUT_Bit_Warning,
    OUTPUT_Bit_RedSign,
    OUTPUT_Bit_GreenSign,
    OUTPUT_Bit_BlueSign,
    OUTPUT_Bit_Backlight,
    OUTPUT_Bit_StoragePower,
    OUTPUT_Bit_StorageEnable,
    OUTPUT_Bit_WirelessEnable,
    OUTPUT_Bit_SoundMute,
    OUTPUT_Bit_MarqueeDir,
    OUTPUT_Bit__COUNT
};


enum OUTPUT_Range
{
    OUTPUT_Range_MarqueeStep,
    OUTPUT_Range_MarqueeFlashMaxLuminance,
    OUTPUT_Range_MarqueeFlashPhase,
    OUTPUT_Range_MarqueeFlashDuration,
    OUTPUT_Range_LightCh0Iref,
    OUTPUT_Range_LightChIref__END = 
                            OUTPUT_Range_LightCh0Iref + 
                            LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS - 1,
    OUTPUT_Range_LightCh0Pwm,
    OUTPUT_Range_LightChPwm__END =
                            OUTPUT_Range_LightCh0Pwm +
                            LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS - 1,
    OUTPUT_Range__COUNT
};


struct OUTPUT_Device
{
    struct IO               * driver;
    uint32_t                driverSource;
};


struct OUTPUT_Mapping
{
    uint16_t                deviceId;
    uint16_t                driverIndex;
};


enum OUTPUT_UpdateValue
{
    OUTPUT_UpdateValue_Now = 0,
    OUTPUT_UpdateValue_Defer
};


struct OUTPUT
{
    struct OUTPUT_Device    device          [OUTPUT_MAX_DEVICES];
    struct OUTPUT_Mapping   bitMapping      [OUTPUT_Bit__COUNT];
    struct OUTPUT_Mapping   rangeMapping    [OUTPUT_Range__COUNT];
    uint16_t                nextDeviceId;
};


void            OUTPUT_Init             (struct OUTPUT *const O);
void            OUTPUT_SetDevice        (struct IO *const Driver,
                                         const uint32_t DriverSource);
void            OUTPUT_MapBit           (const enum OUTPUT_Bit Outb,
                                         const uint16_t DriverIndex);
void            OUTPUT_MapRange         (const enum OUTPUT_Range Outr,
                                         const uint16_t DriverIndex);
void            OUTPUT_Bit              (const enum OUTPUT_Bit Outb,
                                         const uint32_t Value,
                                         const enum OUTPUT_UpdateValue When);
void            OUTPUT_BitNow           (const enum OUTPUT_Bit Outb,
                                         const uint32_t Value);
void            OUTPUT_BitDefer         (const enum OUTPUT_Bit Outb,
                                         const uint32_t Value);
void            OUTPUT_Range            (const enum OUTPUT_Range Outr,
                                         const uint32_t Value,
                                         const enum OUTPUT_UpdateValue When);
void            OUTPUT_RangeNow         (const enum OUTPUT_Range Outr,
                                         const uint32_t Value);
void            OUTPUT_RangeDefer       (const enum OUTPUT_Range Outr,
                                         const uint32_t Value);
const char *    OUTPUT_BitName          (const enum OUTPUT_Bit Outb);
const char *    OUTPUT_RangeName        (const enum OUTPUT_Range Outr);
void            OUTPUT_Update           (void);

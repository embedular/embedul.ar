/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] input devices manager.

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
#include "embedul.ar/source/core/manager/input/switch_action.h"


#define INPUT_MAX_DEVICES       LIB_EMBEDULAR_CONFIG_INPUT_MAX_DEVICES

#if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
#define INPUT_SWITCH_ACTION(_inbn,_swa) \
            ((INPUT_BitAsSwitchAction(INPUT_Bit_ ## _inbn) == \
                        SWITCH_ACTION_Type_ ## _swa)? true : false)
#endif

#define INPUT_BIT(_inbn) \
            INPUT_Bit(INPUT_Bit_ ## _inbn)


enum INPUT_BitRole
{
    INPUT_BitRole_Any  = 0,
    INPUT_BitRole_Board,
    INPUT_BitRole_P1,
    INPUT_BitRole_P2,
    INPUT_BitRole_Switch        // Board + P1 + P2
};


enum INPUT_Bit
{
    INPUT_Bit_BoardA,
    INPUT_Bit_BoardB,
    INPUT_Bit_BoardC,
    INPUT_Bit_BoardD,
    INPUT_Bit_P1Right,
    INPUT_Bit_P1Left,
    INPUT_Bit_P1Down,
    INPUT_Bit_P1Up,
    INPUT_Bit_P1Start,
    INPUT_Bit_P1Select,
    INPUT_Bit_P1A,
    INPUT_Bit_P1B,
    INPUT_Bit_P1C,
    INPUT_Bit_P1X,
    INPUT_Bit_P1Y,
    INPUT_Bit_P1Z,
    INPUT_Bit_P2Right,
    INPUT_Bit_P2Left,
    INPUT_Bit_P2Down,
    INPUT_Bit_P2Up,
    INPUT_Bit_P2Start,
    INPUT_Bit_P2Select,
    INPUT_Bit_P2A,
    INPUT_Bit_P2B,
    INPUT_Bit_P2C,
    INPUT_Bit_P2X,
    INPUT_Bit_P2Y,
    INPUT_Bit_P2Z,
    INPUT_Bit_Backlight,
    INPUT_Bit_StoragePower,
    INPUT_Bit_StorageDetect,
    INPUT_Bit_WirelessEnable,
    INPUT_Bit_SoundMute,
    INPUT_Bit_LightDevice0Overtemp,
    INPUT_Bit_LightDeviceOvertemp__END =
                            INPUT_Bit_LightDevice0Overtemp +
                            LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_Bit_LightDevice0ChError,
    INPUT_Bit_LightChError__END = 
                            INPUT_Bit_LightDevice0ChError +
                            LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_Bit__COUNT,
    INPUT_Bit_Board__BEGIN  = INPUT_Bit_BoardA,
    INPUT_Bit_Board__END    = INPUT_Bit_BoardD,
    INPUT_Bit_P1__BEGIN     = INPUT_Bit_P1Right,
    INPUT_Bit_P1__END       = INPUT_Bit_P1Z,
    INPUT_Bit_P2__BEGIN     = INPUT_Bit_P2Right,
    INPUT_Bit_P2__END       = INPUT_Bit_P2Z,
    INPUT_Bit_Switch__BEGIN = INPUT_Bit_Board__BEGIN,
    INPUT_Bit_Switch__END   = INPUT_Bit_P2__END
};


enum INPUT_Range
{
    INPUT_Range_LightChShortedBitfield0,
    INPUT_Range_LightChShortedBitfield__END = 
                            INPUT_Range_LightChShortedBitfield0 + 
                            LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_Range_LightChOpenBitfield0,
    INPUT_Range_LightChOpenBitfield__END = 
                            INPUT_Range_LightChOpenBitfield0 +
                            LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_Range__COUNT
};

// Device = Driver + Source
struct INPUT_Device
{
    struct IO               * driver;
    uint32_t                driverSource;
};


// driverIndex = inB or inR
struct INPUT_Mapping
{
    uint16_t                deviceId;
    uint16_t                driverIndex;
};


enum INPUT_UpdateValue
{
    INPUT_UpdateValue_Now = 0,
    INPUT_UpdateValue_Buffer
};


struct INPUT
{
    struct INPUT_Device     device          [INPUT_MAX_DEVICES];
    struct INPUT_Mapping    bitMapping      [INPUT_Bit__COUNT];
    struct INPUT_Mapping    rangeMapping    [INPUT_Range__COUNT];
#if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
    struct SWITCH_ACTION    switchAction    [INPUT_Bit__COUNT];
#endif
    uint16_t                nextDeviceId;
};


void            INPUT_Init                  (struct INPUT *const I);
void            INPUT_SetDevice             (struct IO *const Driver,
                                             const uint32_t DriverSource);
void            INPUT_MapBit                (const enum INPUT_Bit Inb,
                                             const uint16_t DriverIndex);
void            INPUT_MapRange              (const enum INPUT_Range Inr,
                                             const uint16_t DriverIndex);
bool            INPUT_MapBitByOnState       (const enum INPUT_Bit Inb);
uint32_t        INPUT_Bit                   (const enum INPUT_Bit Inb,
                                             const enum INPUT_UpdateValue When);
uint32_t        INPUT_BitNow                (const enum INPUT_Bit Inb);
uint32_t        INPUT_BitBuffer             (const enum INPUT_Bit Inb);
uint32_t        INPUT_Range                 (const enum INPUT_Range Inr,
                                             const enum INPUT_UpdateValue When);
uint32_t        INPUT_RangeNow              (const enum INPUT_Range Inr);
uint32_t        INPUT_RangeBuffer           (const enum INPUT_Range Inr);
const char *    INPUT_BitName               (const enum INPUT_Bit Inb);
const char *    INPUT_RangeName             (const enum INPUT_Range Inr);
bool            INPUT_BitIsMapped           (const enum INPUT_Bit Inb);
bool            INPUT_RangeIsMapped         (const enum INPUT_Range Inr);
bool            INPUT_AnyBit                (void);
bool            INPUT_AnyBitByRange         (const enum INPUT_Bit Begin,
                                             const enum INPUT_Bit End);
bool            INPUT_AnyBitByRole          (const enum INPUT_BitRole BitRole);
#if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
enum SWITCH_ACTION_Type
                INPUT_BitAsSwitchAction     (const enum INPUT_Bit Inb);
#endif
void            INPUT_Update                (void);

#pragma once

#include "embedul.ar/source/core/manager/input/profile.h"


#define INPUT_PROFILE_CONTROL_BIT_MAP       1
#define INPUT_PROFILE_CONTROL_BIT_ACTION    0
#define INPUT_PROFILE_CONTROL_RANGE_MAP     0


enum INPUT_PROFILE_CONTROL_Bit
{
    INPUT_PROFILE_CONTROL_Bit_Backlight,
    INPUT_PROFILE_CONTROL_Bit_StoragePower,
    INPUT_PROFILE_CONTROL_Bit_StorageDetect,
    INPUT_PROFILE_CONTROL_Bit_WirelessEnable,
    INPUT_PROFILE_CONTROL_Bit_SoundMute,
    INPUT_PROFILE_CONTROL_Bit__COUNT
};


INPUT_PROFILE_STRUCT(CONTROL);

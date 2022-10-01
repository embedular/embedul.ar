#pragma once

#include "embedul.ar/source/core/manager/output/profile.h"


#define OUTPUT_PROFILE_CONTROL_BIT_MAP       1
#define OUTPUT_PROFILE_CONTROL_RANGE_MAP     0


enum OUTPUT_PROFILE_CONTROL_Bit
{
    OUTPUT_PROFILE_CONTROL_Bit_Backlight,
    OUTPUT_PROFILE_CONTROL_Bit_StoragePower,
    OUTPUT_PROFILE_CONTROL_Bit_StorageEnable,
    OUTPUT_PROFILE_CONTROL_Bit_WirelessEnable,
    OUTPUT_PROFILE_CONTROL_Bit_SoundMute,
    OUTPUT_PROFILE_CONTROL_Bit__COUNT
};


OUTPUT_PROFILE_STRUCT(CONTROL);

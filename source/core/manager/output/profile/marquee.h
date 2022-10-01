#pragma once

#include "embedul.ar/source/core/manager/output/profile.h"


#define OUTPUT_PROFILE_MARQUEE_BIT_MAP       1
#define OUTPUT_PROFILE_MARQUEE_RANGE_MAP     1


enum OUTPUT_PROFILE_MARQUEE_Bit
{
    OUTPUT_PROFILE_MARQUEE_Bit_Dir,
    OUTPUT_PROFILE_MARQUEE_Bit__COUNT
};


enum OUTPUT_PROFILE_MARQUEE_Range
{
    OUTPUT_PROFILE_MARQUEE_Range_Step,
    OUTPUT_PROFILE_MARQUEE_Range_FlashMaxLuminance,
    OUTPUT_PROFILE_MARQUEE_Range_FlashPhase,
    OUTPUT_PROFILE_MARQUEE_Range_FlashDuration,
    OUTPUT_PROFILE_MARQUEE_Range__COUNT
};


OUTPUT_PROFILE_STRUCT(MARQUEE);

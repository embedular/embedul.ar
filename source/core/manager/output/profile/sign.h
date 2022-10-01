#pragma once

#include "embedul.ar/source/core/manager/output/profile.h"


#define OUTPUT_PROFILE_SIGN_BIT_MAP       1
#define OUTPUT_PROFILE_SIGN_RANGE_MAP     0


enum OUTPUT_PROFILE_SIGN_Bit
{
    OUTPUT_PROFILE_SIGN_Bit_Warning,
    OUTPUT_PROFILE_SIGN_Bit_Red,
    OUTPUT_PROFILE_SIGN_Bit_Green,
    OUTPUT_PROFILE_SIGN_Bit_Blue,
    OUTPUT_PROFILE_SIGN_Bit__COUNT
};


OUTPUT_PROFILE_STRUCT(SIGN);

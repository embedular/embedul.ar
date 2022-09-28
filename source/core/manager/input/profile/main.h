#pragma once

#include "embedul.ar/source/core/manager/input/profile.h"


#define INPUT_PROFILE_MAIN_BIT_MAP       1
#define INPUT_PROFILE_MAIN_BIT_ACTION    1
#define INPUT_PROFILE_MAIN_RANGE_MAP     0


enum INPUT_PROFILE_MAIN_Bit
{
    INPUT_PROFILE_MAIN_Bit_A,
    INPUT_PROFILE_MAIN_Bit_B,
    INPUT_PROFILE_MAIN_Bit_C,
    INPUT_PROFILE_MAIN_Bit_D,
    INPUT_PROFILE_MAIN_Bit__COUNT
};


INPUT_PROFILE_STRUCT(MAIN);

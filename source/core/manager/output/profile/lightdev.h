#pragma once

#include "embedul.ar/source/core/manager/output/profile.h"


#define OUTPUT_PROFILE_LIGHTDEV_BIT_MAP       0
#define OUTPUT_PROFILE_LIGHTDEV_RANGE_MAP     1


#if (LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS <= 0)
    #error This output profile requires OUTPUT_MAX_LIGHT_CHANNELS > 0
#endif

#define OUTPUT_PROFILE_LIGHTDEV(_ptype,_pname,_dev_i) \
    (BOARD_AssertParams(_dev_i), ((enum OUTPUT_PROFILE_LIGHTDEV_ ## _ptype) \
        OUTPUT_PROFILE_LIGHTDEV_ ## _ptype ## _ ## _pname ## __BEGIN + _dev_i))

#define OUTPUT_PROFILE_LIGHTDEV_Range_Iref(_dev_i) \
    OUTPUT_PROFILE_LIGHTDEV(Range,Iref,_dev_i)

#define OUTPUT_PROFILE_LIGHTDEV_Range_Pwm(_dev_i) \
    OUTPUT_PROFILE_LIGHTDEV(Range,Pwm,_dev_i)


enum OUTPUT_PROFILE_LIGHTDEV_Range
{
    OUTPUT_PROFILE_LIGHTDEV_Range_Iref__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS
    OUTPUT_PROFILE_LIGHTDEV_Range_Iref__END =
                    OUTPUT_PROFILE_LIGHTDEV_Range_Iref__BEGIN +
                    LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS - 1,
    OUTPUT_PROFILE_LIGHTDEV_Range_Pwm__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS
    OUTPUT_PROFILE_LIGHTDEV_Range_Pwm__END =
                    OUTPUT_PROFILE_LIGHTDEV_Range_Pwm__BEGIN +
                    LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS - 1,
    OUTPUT_PROFILE_LIGHTDEV_Range__COUNT
};


OUTPUT_PROFILE_STRUCT(LIGHTDEV);

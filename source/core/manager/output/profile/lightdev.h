/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [OUTPUT MANAGER] lighting device profile.

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

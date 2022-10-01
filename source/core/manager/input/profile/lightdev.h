/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [INPUT MANAGER] lighting device profile.

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

#include "embedul.ar/source/core/manager/input/profile.h"


#define INPUT_PROFILE_LIGHTDEV_BIT_MAP      1
#define INPUT_PROFILE_LIGHTDEV_BIT_ACTION   0
#define INPUT_PROFILE_LIGHTDEV_RANGE_MAP    1


#if (LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES <= 0)
    #error This input profile requires INPUT_MAX_LIGHTING_DEVICES > 0
#endif

#define INPUT_PROFILE_LIGHTDEV(_ptype,_pname,_dev_i) \
    (BOARD_AssertParams(_dev_i), ((enum INPUT_PROFILE_LIGHTDEV_ ## _ptype) \
        INPUT_PROFILE_LIGHTDEV_ ## _ptype ## _ ## _pname ## __BEGIN + _dev_i))

#define INPUT_PROFILE_LIGHTDEV_Bit_Overtemp(_dev_i) \
    INPUT_PROFILE_LIGHTDEV(Bit,Overtemp,_dev_i)

#define INPUT_PROFILE_LIGHTDEV_Bit_ChannelError(_dev_i) \
    INPUT_PROFILE_LIGHTDEV(Bit,ChannelError,_dev_i)

#define INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted(_dev_i) \
    INPUT_PROFILE_LIGHTDEV(Range,ChannelsShorted,_dev_i)

#define INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen(_dev_i) \
    INPUT_PROFILE_LIGHTDEV(Range,ChannelsOpen,_dev_i)


enum INPUT_PROFILE_LIGHTDEV_Bit
{
    INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES
    INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__END =
                        INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    INPUT_PROFILE_LIGHTDEV_Bit_ChannelError__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES
    INPUT_PROFILE_LIGHTDEV_Bit_ChannelError__END = 
                        INPUT_PROFILE_LIGHTDEV_Bit_ChannelError__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    INPUT_PROFILE_LIGHTDEV_Bit__COUNT
};


enum INPUT_PROFILE_LIGHTDEV_Range
{
    // Bitfield, one bit per channel, LSB to MSB.
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted__END = 
                        INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted__BEGIN + 
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen__END = 
                        INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1,
    INPUT_PROFILE_LIGHTDEV_Range__COUNT
};


INPUT_PROFILE_STRUCT(LIGHTDEV);

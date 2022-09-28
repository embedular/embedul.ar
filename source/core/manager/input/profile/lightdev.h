#pragma once

#include "embedul.ar/source/core/manager/input/profile.h"
//#include "embedul.ar/source/core/device/board.h"


#define INPUT_PROFILE_LIGHTDEV_BIT_MAP      1
#define INPUT_PROFILE_LIGHTDEV_BIT_ACTION   0
#define INPUT_PROFILE_LIGHTDEV_RANGE_MAP    1


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
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1
    INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__END =
                        INPUT_PROFILE_LIGHTDEV_Bit_Overtemp__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_PROFILE_LIGHTDEV_Bit_ChannelError__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1
    INPUT_PROFILE_LIGHTDEV_Bit_ChannelError__END = 
                        INPUT_PROFILE_LIGHTDEV_Bit_ChannelError__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_PROFILE_LIGHTDEV_Bit__COUNT
};


enum INPUT_PROFILE_LIGHTDEV_Range
{
    // Bitfield, one bit per channel, LSB to MSB.
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted__END = 
                        INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted__BEGIN + 
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen__BEGIN,
    // ... LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES - 1
    INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen__END = 
                        INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen__BEGIN +
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES,
    INPUT_PROFILE_LIGHTDEV_Range__COUNT
};


INPUT_PROFILE_STRUCT(LIGHTDEV);

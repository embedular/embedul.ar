/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [INPUT MANAGER] profiles.

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

#include "embedul.ar/source/core/cc.h"
#include "embedul.ar/source/core/misc/input/action.h"
#include "embedul.ar/source/core/misc/io_profile.h"


#define INPUT_PROFILE_BIT_MAP_0(_pname)
#define INPUT_PROFILE_BIT_MAP_1(_pname) struct IO_PROFILE_Map \
                        bitMap[INPUT_PROFILE_ ## _pname ## _Bit__COUNT];

#define INPUT_PROFILE_BIT_ACTION_0(_pname)
#define INPUT_PROFILE_BIT_ACTION_1(_pname) struct INPUT_ACTION \
                        bitAction[INPUT_PROFILE_ ## _pname ## _Bit__COUNT];

#define INPUT_PROFILE_RANGE_MAP_0(_pname)
#define INPUT_PROFILE_RANGE_MAP_1(_pname) struct IO_PROFILE_Map \
                        rangeMap[INPUT_PROFILE_ ## _pname ## _Range__COUNT];

#define INPUT_PROFILE_STRUCT__(_pname,_bm,_ba,_rm) \
    struct INPUT_PROFILE_ ## _pname { \
        CC_ExpPaste(INPUT_PROFILE_BIT_MAP_,_bm)(_pname) \
        CC_ExpPaste(INPUT_PROFILE_BIT_ACTION_,_ba)(_pname) \
        CC_ExpPaste(INPUT_PROFILE_RANGE_MAP_,_rm)(_pname) \
    }

#define INPUT_PROFILE_STRUCT(_pname) \
    INPUT_PROFILE_STRUCT__(_pname, \
                           INPUT_PROFILE_ ## _pname ## _BIT_MAP, \
                           INPUT_PROFILE_ ## _pname ## _BIT_ACTION, \
                           INPUT_PROFILE_ ## _pname ## _RANGE_MAP)

#define INPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
    CC_AssertType(_profile,struct INPUT_PROFILE_ ## _pname); \
    INPUT_PROFILE__attach ( \
                    _board->mio.inProfiles, \
                    _board->mio.inBitActions, \
                    INPUT_PROFILE_Group_ ## _pname,

#define INPUT_PROFILE_ATTACH__END )

#define INPUT_PROFILE_ATTACH_BIT_11(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
                    _profile.bitMap, \
                    _profile.bitAction, \
                    INPUT_PROFILE_ ## _pname ## _Bit__COUNT, \

#define INPUT_PROFILE_ATTACH_BIT_10(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
                    _profile.bitMap, \
                    NULL, \
                    INPUT_PROFILE_ ## _pname ## _Bit__COUNT, \

#define INPUT_PROFILE_ATTACH_BIT_00(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
                    NULL, \
                    NULL, \
                    0, \

#define INPUT_PROFILE_ATTACH_RANGE_1(_pname,_profile) \
                    _profile.rangeMap, \
                    INPUT_PROFILE_ ## _pname ## _Range__COUNT \
                    INPUT_PROFILE_ATTACH__END

#define INPUT_PROFILE_ATTACH_RANGE_0(_pname,_profile) \
                    NULL, \
                    0 \
                    INPUT_PROFILE_ATTACH__END

#define INPUT_PROFILE_ATTACH__111(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_BIT_11(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_RANGE_1(_pname,_profile)

#define INPUT_PROFILE_ATTACH__110(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_BIT_11(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_RANGE_0(_pname,_profile)

#define INPUT_PROFILE_ATTACH__101(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_BIT_10(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_RANGE_1(_pname,_profile)

#define INPUT_PROFILE_ATTACH__100(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_BIT_10(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_RANGE_0(_pname,_profile)

#define INPUT_PROFILE_ATTACH__001(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_BIT_00(_pname,_board,_profile) \
                    INPUT_PROFILE_ATTACH_RANGE_1(_pname,_profile)

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    #define INPUT_PROFILE__CODE(_pname) \
        CC_ExpPaste( \
            CC_ExpPaste( \
                CC_Exp(INPUT_PROFILE_ ## _pname ## _BIT_MAP), \
                CC_Exp(INPUT_PROFILE_ ## _pname ## _BIT_ACTION) \
            ), \
            CC_Exp(INPUT_PROFILE_ ## _pname ## _RANGE_MAP) \
        )
#else
    #define INPUT_PROFILE__CODE(_pname) \
        CC_ExpPaste( \
            CC_ExpPaste( \
                CC_Exp(INPUT_PROFILE_ ## _pname ## _BIT_MAP), \
                0 \
            ), \
            CC_Exp(INPUT_PROFILE_ ## _pname ## _RANGE_MAP) \
        )
#endif

#define INPUT_PROFILE_ATTACH(_pname,_board,_profile) \
    CC_ExpPaste(INPUT_PROFILE_ATTACH__,CC_Exp(INPUT_PROFILE__CODE(_pname))) \
                                                    (_pname,_board,_profile)


enum INPUT_PROFILE_SelectFlag
{
    INPUT_PROFILE_SelectFlag_CONTROL    = 0x01,
    INPUT_PROFILE_SelectFlag_GP1        = 0x02,
    INPUT_PROFILE_SelectFlag_GP2        = 0x04,
    INPUT_PROFILE_SelectFlag_LIGHTDEV   = 0x08,
    INPUT_PROFILE_SelectFlag_MAIN       = 0x10,
    INPUT_PROFILE_SelectFlag__ALL       = 0x1F,
    INPUT_PROFILE_SelectFlag__GAMEPADS  = (INPUT_PROFILE_SelectFlag_GP1 |
                                           INPUT_PROFILE_SelectFlag_GP2),
    INPUT_PROFILE_SelectFlag__BUTTONS
                                        = (INPUT_PROFILE_SelectFlag__GAMEPADS |
                                           INPUT_PROFILE_SelectFlag_MAIN)
};


enum INPUT_PROFILE_Group
{
    INPUT_PROFILE_Group_CONTROL,
    INPUT_PROFILE_Group_GP1,
    INPUT_PROFILE_Group_GP2,
    INPUT_PROFILE_Group_LIGHTDEV,
    INPUT_PROFILE_Group_MAIN,
    INPUT_PROFILE_Group__COUNT
};


extern const char *const INPUT_PROFILE_GroupString[INPUT_PROFILE_Group__COUNT];


struct INPUT_PROFILE
{
    struct IO_PROFILE_Map   * map[IO_Type__COUNT];
    struct INPUT_ACTION     * bitAction;
    uint16_t                count[IO_Type__COUNT];
};


const char *
INPUT_PROFILE_GetGroupName (const enum INPUT_PROFILE_Group ProfileGroup);


void INPUT_PROFILE__attach (
    struct IO_PROFILE inProfiles[static const INPUT_PROFILE_Group__COUNT],
    struct INPUT_ACTION *InBitActions[static const INPUT_PROFILE_Group__COUNT],
    const enum INPUT_PROFILE_Group InProfileGroup,
    struct IO_PROFILE_Map *const BitMap,
    struct INPUT_ACTION *const BitAction,
    const uint32_t BitCount,
    struct IO_PROFILE_Map *const RangeMap,
    const uint32_t RangeCount);

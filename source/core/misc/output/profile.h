/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [OUTPUT MANAGER] profiles.

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
#include "embedul.ar/source/core/misc/io_profile.h"


#define OUTPUT_PROFILE_BIT_MAP_0(_pname)
#define OUTPUT_PROFILE_BIT_MAP_1(_pname) struct IO_PROFILE_Map \
                        bitMap[OUTPUT_PROFILE_ ## _pname ## _Bit__COUNT]

#define OUTPUT_PROFILE_RANGE_MAP_0(_pname)
#define OUTPUT_PROFILE_RANGE_MAP_1(_pname) struct IO_PROFILE_Map \
                        rangeMap[OUTPUT_PROFILE_ ## _pname ## _Range__COUNT]

#define OUTPUT_PROFILE_STRUCT__(_pname,_bm,_rm) \
    struct OUTPUT_PROFILE_ ## _pname { \
        CC_ExpPaste(OUTPUT_PROFILE_BIT_MAP_,_bm)(_pname); \
        CC_ExpPaste(OUTPUT_PROFILE_RANGE_MAP_,_rm)(_pname); \
    }

#define OUTPUT_PROFILE_STRUCT(_pname) \
    OUTPUT_PROFILE_STRUCT__(_pname, \
                            OUTPUT_PROFILE_ ## _pname ## _BIT_MAP, \
                            OUTPUT_PROFILE_ ## _pname ## _RANGE_MAP)

#define OUTPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
    CC_AssertType(_profile,struct OUTPUT_PROFILE_ ## _pname); \
    OUTPUT_PROFILE__attach ( \
                    _board->mio.outProfiles, \
                    OUTPUT_PROFILE_Group_ ## _pname,

#define OUTPUT_PROFILE_ATTACH__END )

#define OUTPUT_PROFILE_ATTACH_BIT_1(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
                    _profile.bitMap, \
                    OUTPUT_PROFILE_ ## _pname ## _Bit__COUNT, \

#define OUTPUT_PROFILE_ATTACH_BIT_0(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
                    NULL, \
                    0, \

#define OUTPUT_PROFILE_ATTACH_RANGE_1(_pname,_profile) \
                    _profile.rangeMap, \
                    OUTPUT_PROFILE_ ## _pname ## _Range__COUNT \
                    OUTPUT_PROFILE_ATTACH__END

#define OUTPUT_PROFILE_ATTACH_RANGE_0(_pname,_profile) \
                    NULL, \
                    0 \
                    OUTPUT_PROFILE_ATTACH__END

#define OUTPUT_PROFILE_ATTACH__11(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_BIT_1(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_RANGE_1(_pname,_profile)

#define OUTPUT_PROFILE_ATTACH__10(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_BIT_1(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_RANGE_0(_pname,_profile)

#define OUTPUT_PROFILE_ATTACH__01(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_BIT_0(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_RANGE_1(_pname,_profile)

#define OUTPUT_PROFILE_ATTACH__00(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_BIT_0(_pname,_board,_profile) \
                    OUTPUT_PROFILE_ATTACH_RANGE_0(_pname,_profile)

#define OUTPUT_PROFILE__CODE(_pname) \
    CC_ExpPaste( \
        CC_Exp(OUTPUT_PROFILE_ ## _pname ## _BIT_MAP), \
        CC_Exp(OUTPUT_PROFILE_ ## _pname ## _RANGE_MAP))

#define OUTPUT_PROFILE_ATTACH(_pname,_board,_profile) \
    CC_ExpPaste(OUTPUT_PROFILE_ATTACH__,CC_Exp(OUTPUT_PROFILE__CODE(_pname))) \
                                                    (_pname,_board,_profile)


enum OUTPUT_PROFILE_SelectFlag
{
    OUTPUT_PROFILE_SelectFlag_CONTROL   = 0x01,
    OUTPUT_PROFILE_SelectFlag_LIGHTDEV  = 0x02,
    OUTPUT_PROFILE_SelectFlag_MARQUEE   = 0x04,
    OUTPUT_PROFILE_SelectFlag_SIGN      = 0x08
};


enum OUTPUT_PROFILE_Group
{
    OUTPUT_PROFILE_Group_CONTROL,
    OUTPUT_PROFILE_Group_LIGHTDEV,
    OUTPUT_PROFILE_Group_MARQUEE,
    OUTPUT_PROFILE_Group_SIGN,
    OUTPUT_PROFILE_Group__COUNT
};


extern const char *const 
OUTPUT_PROFILE_GroupString[OUTPUT_PROFILE_Group__COUNT];


struct OUTPUT_PROFILE
{
    struct IO_PROFILE_Map   * map[IO_Type__COUNT];
    uint16_t                count[IO_Type__COUNT];
};


const char *
OUTPUT_PROFILE_GetGroupName (const enum OUTPUT_PROFILE_Group ProfileGroup);


void OUTPUT_PROFILE__attach (
    struct IO_PROFILE OutProfiles[static const OUTPUT_PROFILE_Group__COUNT],
    const enum OUTPUT_PROFILE_Group OutProfileGroup,
    struct IO_PROFILE_Map *const BitMap,
    const uint32_t BitCount,
    struct IO_PROFILE_Map *const RangeMap,
    const uint32_t RangeCount);

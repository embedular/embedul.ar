#pragma once

#include "embedul.ar/source/core/cc.h"
#include "embedul.ar/source/core/manager/io_profile.h"


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
    OUTPUT_PROFILE__attach (_board->output.profiles, \
                    OUTPUT_PROFILE_Type_ ## _pname,

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


enum OUTPUT_PROFILE_Type
{
    OUTPUT_PROFILE_Type_CONTROL,
    OUTPUT_PROFILE_Type_LIGHTDEV,
    OUTPUT_PROFILE_Type_MARQUEE,
    OUTPUT_PROFILE_Type_SIGN,
    OUTPUT_PROFILE_Type__COUNT
};


extern const char * OUTPUT_PROFILE_TypeString[OUTPUT_PROFILE_Type__COUNT];


struct OUTPUT_PROFILE
{
    struct IO_PROFILE_Map   * bitMap;
    struct IO_PROFILE_Map   * rangeMap;
    uint16_t                bitCount;
    uint16_t                rangeCount;
};


const char * OUTPUT_PROFILE_GetTypeName (enum OUTPUT_PROFILE_Type ProfileType);


void OUTPUT_PROFILE__attach (
    struct OUTPUT_PROFILE ProfilesArray[static const OUTPUT_PROFILE_Type__COUNT],
    const enum OUTPUT_PROFILE_Type ProfileType,
    struct IO_PROFILE_Map *const BitMap,
    const uint32_t BitCount,
    struct IO_PROFILE_Map *const RangeMap,
    const uint32_t RangeCount);

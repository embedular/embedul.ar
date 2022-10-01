#pragma once

#include "embedul.ar/source/core/cc.h"
#include "embedul.ar/source/core/manager/input/action.h"
#include "embedul.ar/source/core/manager/io_profile.h"


#define INPUT_PROFILE_BIT_MAP_0(_pname)
#define INPUT_PROFILE_BIT_MAP_1(_pname) struct IO_PROFILE_Map \
                        bitMap[INPUT_PROFILE_ ## _pname ## _Bit__COUNT]

#define INPUT_PROFILE_BIT_ACTION_0(_pname)
#define INPUT_PROFILE_BIT_ACTION_1(_pname) struct INPUT_ACTION \
                        bitAction[INPUT_PROFILE_ ## _pname ## _Bit__COUNT]

#define INPUT_PROFILE_RANGE_MAP_0(_pname)
#define INPUT_PROFILE_RANGE_MAP_1(_pname) struct IO_PROFILE_Map \
                        rangeMap[INPUT_PROFILE_ ## _pname ## _Range__COUNT]

#define INPUT_PROFILE_STRUCT__(_pname,_bm,_ba,_rm) \
    struct INPUT_PROFILE_ ## _pname { \
        CC_ExpPaste(INPUT_PROFILE_BIT_MAP_,_bm)(_pname); \
        CC_ExpPaste(INPUT_PROFILE_BIT_ACTION_,_ba)(_pname); \
        CC_ExpPaste(INPUT_PROFILE_RANGE_MAP_,_rm)(_pname); \
    }

#define INPUT_PROFILE_STRUCT(_pname) \
    INPUT_PROFILE_STRUCT__(_pname, \
                           INPUT_PROFILE_ ## _pname ## _BIT_MAP, \
                           INPUT_PROFILE_ ## _pname ## _BIT_ACTION, \
                           INPUT_PROFILE_ ## _pname ## _RANGE_MAP)

#define INPUT_PROFILE_ATTACH__BEGIN(_pname,_board,_profile) \
    CC_AssertType(_profile,struct INPUT_PROFILE_ ## _pname); \
    INPUT_PROFILE__attach (_board->input.profiles, \
                    INPUT_PROFILE_Type_ ## _pname,

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
    INPUT_PROFILE_SelectFlag__BUTTONS
                                        = (INPUT_PROFILE_SelectFlag_GP1 |
                                           INPUT_PROFILE_SelectFlag_GP2 |
                                           INPUT_PROFILE_SelectFlag_MAIN)
};


enum INPUT_PROFILE_Type
{
    INPUT_PROFILE_Type_CONTROL,
    INPUT_PROFILE_Type_GP1,
    INPUT_PROFILE_Type_GP2,
    INPUT_PROFILE_Type_LIGHTDEV,
    INPUT_PROFILE_Type_MAIN,
    INPUT_PROFILE_Type__COUNT
};


extern const char * INPUT_PROFILE_TypeString[INPUT_PROFILE_Type__COUNT];


struct INPUT_PROFILE
{
    struct IO_PROFILE_Map   * bitMap;
    struct INPUT_ACTION     * bitAction;
    struct IO_PROFILE_Map   * rangeMap;
    uint16_t                bitCount;
    uint16_t                rangeCount;
};


const char * INPUT_PROFILE_GetTypeName (enum INPUT_PROFILE_Type ProfileType);


void INPUT_PROFILE__attach (
    struct INPUT_PROFILE ProfilesArray[static const INPUT_PROFILE_Type__COUNT],
    const enum INPUT_PROFILE_Type ProfileType,
    struct IO_PROFILE_Map *const BitMap,
    struct INPUT_ACTION *const BitAction,
    const uint32_t BitCount,
    struct IO_PROFILE_Map *const RangeMap,
    const uint32_t RangeCount);

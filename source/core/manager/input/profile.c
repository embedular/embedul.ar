#include "embedul.ar/source/core/manager/input/profile.h"
#include "embedul.ar/source/core/device/board.h"


const char * INPUT_PROFILE_TypeString[INPUT_PROFILE_Type__COUNT] = 
{
    [INPUT_PROFILE_Type_CONTROL]    = "control",
    [INPUT_PROFILE_Type_GP1]        = "gp1",
    [INPUT_PROFILE_Type_GP2]        = "gp2",
    [INPUT_PROFILE_Type_LIGHTDEV]   = "lightdev",
    [INPUT_PROFILE_Type_MAIN]       = "main"
};


const char * INPUT_PROFILE_GetTypeName (enum INPUT_PROFILE_Type ProfileType)
{
    BOARD_AssertParams (ProfileType < INPUT_PROFILE_Type__COUNT);
    return INPUT_PROFILE_TypeString[ProfileType];
}


void INPUT_PROFILE__attach (
    struct INPUT_PROFILE ProfilesArray[static const INPUT_PROFILE_Type__COUNT],
    const enum INPUT_PROFILE_Type ProfileType,
    struct IO_PROFILE_Map *const BitMap,
    struct INPUT_ACTION *const BitAction,
    const uint32_t BitCount,
    struct IO_PROFILE_Map *const RangeMap,
    const uint32_t RangeCount)
{
    BOARD_AssertParams (ProfilesArray &&
                        ProfileType < INPUT_PROFILE_Type__COUNT);

    BOARD_AssertParams ((BitAction && (BitMap && BitCount)) ||
                        !BitAction);

    BOARD_AssertParams ((BitMap && BitCount) || (RangeMap && RangeCount));

    struct INPUT_PROFILE *const P = &ProfilesArray[ProfileType];
    memset (P, 0, sizeof (*P));

    if (BitMap)
    {
        memset (BitMap, IO_INVALID_CODE, sizeof(*BitMap) * BitCount);
        P->bitMap       = BitMap;
        P->bitCount     = BitCount;
    }

    if (BitAction)
    {
        memset (BitAction, 0, sizeof(*BitAction) * BitCount);
        P->bitAction    = BitAction;
    }

    if (RangeMap)
    {
        memset (RangeMap, IO_INVALID_CODE, sizeof(*RangeMap) * RangeCount);
        P->rangeMap     = RangeMap;
        P->rangeCount   = RangeCount;
    }
}
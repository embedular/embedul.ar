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
#include "embedul.ar/source/core/misc/input/profile.h"
#include "embedul.ar/source/core/device/board.h"


const char *const INPUT_PROFILE_GroupString[INPUT_PROFILE_Group__COUNT] = 
{
    [INPUT_PROFILE_Group_CONTROL]    = "Control",
    [INPUT_PROFILE_Group_GP1]        = "Gp1",
    [INPUT_PROFILE_Group_GP2]        = "Gp2",
    [INPUT_PROFILE_Group_LIGHTDEV]   = "Lightdev",
    [INPUT_PROFILE_Group_MAIN]       = "Main"
};


const char *
INPUT_PROFILE_GetGroupName (const enum INPUT_PROFILE_Group ProfileGroup)
{
    BOARD_AssertParams (ProfileGroup < INPUT_PROFILE_Group__COUNT);
    return INPUT_PROFILE_GroupString[ProfileGroup];
}


void INPUT_PROFILE__attach (
    struct IO_PROFILE inProfiles[static const INPUT_PROFILE_Group__COUNT],
    struct INPUT_ACTION *InBitActions[static const INPUT_PROFILE_Group__COUNT],
    const enum INPUT_PROFILE_Group InProfileGroup,
    struct IO_PROFILE_Map *const BitMap,
    struct INPUT_ACTION *const BitAction,
    const uint32_t BitCount,
    struct IO_PROFILE_Map *const RangeMap,
    const uint32_t RangeCount)
{
    BOARD_AssertParams (inProfiles && InBitActions &&
                        InProfileGroup < INPUT_PROFILE_Group__COUNT);
    BOARD_AssertParams ((BitAction && (BitMap && BitCount)) || !BitAction);
    BOARD_AssertParams ((BitMap && BitCount) || (RangeMap && RangeCount));

    struct IO_PROFILE *const P = &inProfiles[InProfileGroup];

    memset (P, 0, sizeof (*P));

    InBitActions[InProfileGroup] = NULL;

    if (BitMap)
    {
        memset (BitMap, IO_INVALID_CODE, sizeof(*BitMap) * BitCount);
        P->map[IO_Type_Bit]     = BitMap;
        P->count[IO_Type_Bit]   = BitCount;
    }

    if (BitAction)
    {
    #if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
        memset (BitAction, 0, sizeof(*BitAction) * BitCount);
        InBitActions[InProfileGroup] = BitAction;
    #else
        BOARD_AssertState (false);
    #endif
    }

    if (RangeMap)
    {
        memset (RangeMap, IO_INVALID_CODE, sizeof(*RangeMap) * RangeCount);
        P->map[IO_Type_Range]   = RangeMap;
        P->count[IO_Type_Range] = RangeCount;
    }
}

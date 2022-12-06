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
#include "embedul.ar/source/core/misc/output/profile.h"
#include "embedul.ar/source/core/device/board.h"


const char *const OUTPUT_PROFILE_GroupString[OUTPUT_PROFILE_Group__COUNT] = 
{
    [OUTPUT_PROFILE_Group_CONTROL]   = "Control",
    [OUTPUT_PROFILE_Group_LIGHTDEV]  = "Lightdev",
    [OUTPUT_PROFILE_Group_MARQUEE]   = "Marquee",
    [OUTPUT_PROFILE_Group_SIGN]      = "Sign"
};


const char *
OUTPUT_PROFILE_GetGroupName (const enum OUTPUT_PROFILE_Group ProfileGroup)
{
    BOARD_AssertParams (ProfileGroup < OUTPUT_PROFILE_Group__COUNT);
    return OUTPUT_PROFILE_GroupString[ProfileGroup];
}


void OUTPUT_PROFILE__attach (
    struct IO_PROFILE OutProfiles[static const OUTPUT_PROFILE_Group__COUNT],
    const enum OUTPUT_PROFILE_Group OutProfileGroup,
    struct IO_PROFILE_Map *const BitMap,
    const uint32_t BitCount,
    struct IO_PROFILE_Map *const RangeMap,
    const uint32_t RangeCount)
{
    BOARD_AssertParams (OutProfiles &&
                        OutProfileGroup < OUTPUT_PROFILE_Group__COUNT);

    BOARD_AssertParams ((BitMap && BitCount) || (RangeMap && RangeCount));

    struct IO_PROFILE *const P = &OutProfiles[OutProfileGroup];

    memset (P, 0, sizeof (*P));

    if (BitMap)
    {
        memset (BitMap, IO_INVALID_CODE, sizeof(*BitMap) * BitCount);
        P->map[IO_Type_Bit]     = BitMap;
        P->count[IO_Type_Bit]   = BitCount;
    }

    if (RangeMap)
    {
        memset (RangeMap, IO_INVALID_CODE, sizeof(*RangeMap) * RangeCount);
        P->map[IO_Type_Range]   = RangeMap;
        P->count[IO_Type_Range] = RangeCount;
    }
}

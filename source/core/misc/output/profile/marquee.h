/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [OUTPUT MANAGER] marquee profile.

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

#include "embedul.ar/source/core/misc/output/profile.h"


#define OUTPUT_PROFILE_MARQUEE_BIT_MAP       1
#define OUTPUT_PROFILE_MARQUEE_RANGE_MAP     1


enum OUTPUT_PROFILE_MARQUEE_Bit
{
    OUTPUT_PROFILE_MARQUEE_Bit_Dir,
    OUTPUT_PROFILE_MARQUEE_Bit__COUNT
};


enum OUTPUT_PROFILE_MARQUEE_Range
{
    OUTPUT_PROFILE_MARQUEE_Range_Step,
    OUTPUT_PROFILE_MARQUEE_Range_FlashMaxLuminance,
    OUTPUT_PROFILE_MARQUEE_Range_FlashPhase,
    OUTPUT_PROFILE_MARQUEE_Range_FlashDuration,
    OUTPUT_PROFILE_MARQUEE_Range__COUNT
};


OUTPUT_PROFILE_STRUCT(MARQUEE);

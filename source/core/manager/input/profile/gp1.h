/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [INPUT MANAGER] first gamepad profile.

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


#define INPUT_PROFILE_GP1_BIT_MAP       1
#define INPUT_PROFILE_GP1_BIT_ACTION    1
#define INPUT_PROFILE_GP1_RANGE_MAP     0


enum INPUT_PROFILE_GP1_Bit
{
    INPUT_PROFILE_GP1_Bit_Right,
    INPUT_PROFILE_GP1_Bit_Left,
    INPUT_PROFILE_GP1_Bit_Down,
    INPUT_PROFILE_GP1_Bit_Up,
    INPUT_PROFILE_GP1_Bit_Start,
    INPUT_PROFILE_GP1_Bit_Select,
    INPUT_PROFILE_GP1_Bit_A,
    INPUT_PROFILE_GP1_Bit_B,
    INPUT_PROFILE_GP1_Bit_C,
    INPUT_PROFILE_GP1_Bit_X,
    INPUT_PROFILE_GP1_Bit_Y,
    INPUT_PROFILE_GP1_Bit_Z,
    INPUT_PROFILE_GP1_Bit__COUNT
};


INPUT_PROFILE_STRUCT(GP1);

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  input switch action detection.

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

#include "embedul.ar/source/core/timer.h"
#include <stdbool.h>


enum INPUT_ACTION_Type
{
    INPUT_ACTION_Type_None = 0,
    INPUT_ACTION_Type_Pressed,
    INPUT_ACTION_Type_Clicked,
    INPUT_ACTION_Type_DoubleClicked,
    INPUT_ACTION_Type_OnHold,
    INPUT_ACTION_Type_Released
};


struct INPUT_ACTION
{
    enum INPUT_ACTION_Type
                    lastAction;
    TIMER_Ticks     holdStarted;
    TIMER_Ticks     lastClicked;
    TIMER_Ticks     lastHold;
    bool            currentStatus;  // true:pressed, false:released
    uint16_t        pressedCount;
    uint16_t        clickedCount;
    uint16_t        doubleClickedCount;
    uint16_t        onHoldCount;
    uint16_t        releasedCount;
};


void                    INPUT_ACTION_Reset      (struct INPUT_ACTION *const A);
enum INPUT_ACTION_Type  INPUT_ACTION_Update     (struct INPUT_ACTION *const A,
                                                 const bool NewStatus);
enum INPUT_ACTION_Type  INPUT_ACTION_Last       (struct INPUT_ACTION *const A);

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


enum SWITCH_ACTION_Type
{
    SWITCH_ACTION_Type_None = 0,
    SWITCH_ACTION_Type_Pressed,
    SWITCH_ACTION_Type_Clicked,
    SWITCH_ACTION_Type_DoubleClicked,
    SWITCH_ACTION_Type_OnHold,
    SWITCH_ACTION_Type_Released
};


struct SWITCH_ACTION
{
    enum SWITCH_ACTION_Type
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


void                    SWITCH_ACTION_Reset     (struct SWITCH_ACTION *const S);
enum SWITCH_ACTION_Type SWITCH_ACTION_Update    (struct SWITCH_ACTION *const S,
                                                 const bool NewStatus);
enum SWITCH_ACTION_Type SWITCH_ACTION_Last      (struct SWITCH_ACTION *const S);

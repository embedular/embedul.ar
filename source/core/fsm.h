/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] finite state machine.

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
#include <stdint.h>
#include <stdbool.h>


#ifndef FSM_MAX_REPEATING_CALLS
    #define FSM_MAX_REPEATING_CALLS     20
#endif


enum FSM_Stage
{
    FSM_Stage_Begin = 0,
    FSM_Stage_Main,
    FSM_Stage_End,
    FSM_Stage__LAST = FSM_Stage_End
};


enum FSM_StateReturn
{
    FSM_StateReturn_Yield = 0,
    FSM_StateReturn_Again,
    FSM_StateReturn_Exit
};


struct FSM;


typedef enum FSM_StateReturn (* FSM_StateFunc) (struct FSM *const F,
                                    enum FSM_Stage stage, TIMER_Ticks ticks);


struct FSM
{
    FSM_StateFunc   state;
    enum FSM_Stage  stage;
    const char      * info;
    uint32_t        stateCalls;
    uint32_t        stageCalls;
    TIMER_Ticks     stateStartTicks;
    TIMER_Ticks     stageStartTicks;
    TIMER_Ticks     stateCountdownTicks;
    void            * param;
};


void            FSM_Init                    (struct FSM *const F,
                                             void *const Param);
void            FSM_SetStateInfo            (struct FSM *const F,
                                             const char *const Info);
bool            FSM_StateTimeout            (struct FSM *const F,
                                             const TIMER_Ticks Timeout);
bool            FSM_StageTimeout            (struct FSM *const F,
                                             const TIMER_Ticks Timeout);
bool            FSM_StateCountdown          (struct FSM *const F,
                                             const TIMER_Ticks Timeout);
TIMER_Ticks     FSM_StateCountdownSeconds   (struct FSM *const F);
void            FSM_GotoStage               (struct FSM *const F,
                                             const enum FSM_Stage NewStage);
void            FSM_StateTransition         (struct FSM *const F,
                                             const FSM_StateFunc NextState);
enum FSM_StateReturn
                FSM_Process                 (struct FSM *const F,
                                             const TIMER_Ticks Now,
                                             const TIMER_Ticks Timeout);

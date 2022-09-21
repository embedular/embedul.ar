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

#include "embedul.ar/source/core/fsm.h"
#include "embedul.ar/source/core/device/board.h"


void FSM_Init (struct FSM *const F, void *const Param)
{
    BOARD_AssertParams (F);

    OBJECT_Clear (F);

    F->param = Param;
}


void FSM_SetStateInfo (struct FSM *const F, const char *const Info)
{
    BOARD_AssertParams (F && Info);
    F->info = Info;
}


bool FSM_StateTimeout (struct FSM *const F, const TIMER_Ticks Timeout)
{
    return (F && F->stateStartTicks + Timeout >= BOARD_TicksNow());
}


bool FSM_StageTimeout (struct FSM *const F, const TIMER_Ticks Timeout)
{
    return (F && F->stageStartTicks + Timeout <= BOARD_TicksNow());
}


bool FSM_StateCountdown (struct FSM *const F, const TIMER_Ticks Timeout)
{
    BOARD_AssertParams (F);

    const TIMER_Ticks NextTimeout = BOARD_TicksNow() + Timeout;

    if (!F->stateCountdownTicks)
    {
        F->stateCountdownTicks = NextTimeout;
    }
    else if (BOARD_TicksNow() >= F->stateCountdownTicks)
    {
        F->stateCountdownTicks = Timeout? NextTimeout : 0;

        return true;
    }

    return false;
}

/*
TIMER_Ticks FSM_StateCountdownSeconds (struct FSM *f)
{
    BOARD_AssertParams (f);

    if (!f->stateCountdownTicks)
    {
        return 0;
    }

    const TIMER_Ticks Now = BOARD_TicksNow ();

    if (Now >= f->stateCountdownTicks)
    {
        return 0;
    }

    return (f->stateCountdownTicks - Now) / BOARD_TickPeriod ();
}
*/

void FSM_GotoStage (struct FSM *const F, const enum FSM_Stage NewStage)
{
    BOARD_AssertParams (F && NewStage <= FSM_Stage__LAST);

    F->stage              = NewStage;
    F->stageCalls         = 0;
    F->stageStartTicks    = BOARD_TicksNow ();
}


void FSM_StateTransition (struct FSM *const F, const FSM_StateFunc NextState)
{
    BOARD_AssertParams (F && NextState);

    F->state                  = NextState;
    F->info                   = NULL;
    F->stateCalls             = 0;
    F->stateStartTicks        = BOARD_TicksNow ();
    F->stateCountdownTicks    = 0;

    FSM_GotoStage (F, FSM_Stage_Begin);
}


enum FSM_StateReturn FSM_Process (struct FSM *const F,
                                  const TIMER_Ticks Now,
                                  const TIMER_Ticks Timeout)
{
    BOARD_AssertParams (F);

    const TIMER_Ticks CurTimeout = (Timeout)? Now + Timeout : 0;

    uint32_t                repeatingCalls = 0;
    enum FSM_StateReturn    ret = FSM_StateReturn_Yield;

    do
    {
        BOARD_AssertState (F->state && F->stage <= FSM_Stage__LAST);

        #if (FSM_MAX_REPEATING_CALLS != 0)
            BOARD_AssertState (repeatingCalls < FSM_MAX_REPEATING_CALLS);
        #endif

        if (CurTimeout && BOARD_TicksNow() >= CurTimeout)
        {
            break;
        }

        ret = F->state (F, F->stage, BOARD_TicksNow());

        ++ F->stateCalls;
        ++ F->stageCalls;
        ++ repeatingCalls;
    }
    while (ret == FSM_StateReturn_Again);

    return ret;
}

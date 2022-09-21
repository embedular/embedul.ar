/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] linear sequence of timed actions.

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

#include "embedul.ar/source/core/sequence.h"
#include "embedul.ar/source/core/device/board.h"


static void restart (struct SEQUENCE *const S)
{
    S->currentStage         = 0;
    S->currentStagePeriod   = 0;
    S->currentStageTimeout  = 0;
    S->currentStageAction   = SEQUENCE_Action_Entry;
}


void SEQUENCE_Init (struct SEQUENCE *const S,
                    const struct SEQUENCE_Stage *const Stages,
                    const uint32_t StagesCount, void *const StagesParam)
{
    BOARD_AssertParams (S && Stages && StagesCount);

    OBJECT_Clear (S);

    S->stages       = Stages;
    S->stagesCount  = StagesCount;
    S->stagesParam  = StagesParam;

    restart (S);
}


void SEQUENCE_Restart (struct SEQUENCE *const S)
{
    BOARD_AssertParams (S);
    restart (S);
}


bool SEQUENCE_Update (struct SEQUENCE *const S)
{
    BOARD_AssertParams (S);
    BOARD_AssertState  (S->currentStage < S->stagesCount);

    const TIMER_Ticks Now = BOARD_TicksNow ();

    const struct SEQUENCE_Stage *const Stage = &S->stages[S->currentStage];

    // Check for events that cancel a running sequence. If ended, the sequence
    // will execute an Action_Exit on this same iteration.
    if (S->currentStageAction == SEQUENCE_Action_Run)
    {
        // Running stage timeout, 
        if (Stage->timeout && S->currentStageStarted + Stage->timeout <= Now)
        {
            SEQUENCE_ExitStage (S);
        }

        if (Stage->input)
        {
            const uint32_t Type = SEQUENCE_INPUT__GT (Stage->input);
            const uint8_t  G1   = SEQUENCE_INPUT__G1 (Stage->input);
            const uint8_t  G2   = SEQUENCE_INPUT__G2 (Stage->input);

            const bool AnyBit   = Type == SEQUENCE_INPUT__TYPE_ANY && 
                                  INPUT_AnyBit();
            const bool Bit      = Type == SEQUENCE_INPUT__TYPE_BIT &&
                                  INPUT_BitBuffer(G1);
            const bool Range    = Type == SEQUENCE_INPUT__TYPE_RANGE &&
                                  INPUT_AnyBitByRange(G1, G2);
            const bool Role     = Type == SEQUENCE_INPUT__TYPE_ROLE &&
                                  INPUT_AnyBitByRole(G1);
        #if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
            const bool Action   = Type == SEQUENCE_INPUT__TYPE_BIT_ACTION &&
                                  INPUT_BitAsSwitchAction(G1) == G2;
            if (AnyBit || Bit || Range || Role || Action)
            {
                SEQUENCE_ExitStage (S);
            }
        #else
            if (AnyBit || Bit || Range || Role)
            {
                SEQUENCE_ExitStage (S);
            }
        #endif
        }
    }

    if (S->exitRequested)
    {
        S->exitRequested        = false;
        S->currentStageAction   = SEQUENCE_Action_Exit;
    }

    // Stage init
    if (S->currentStageAction == SEQUENCE_Action_Entry)
    {
        // Stage function required when no timeout
        BOARD_AssertState (Stage->timeout || (!Stage->timeout && Stage->func));

        if (Stage->period)
        {
            // Stage function required when specifying a running period
            // ('Now' allows to run the first period immediately)
            BOARD_AssertState (Stage->func);
            S->currentStagePeriod = Now;
        }

        S->currentStageStarted = Now;
    }
    else if (S->currentStageAction == SEQUENCE_Action_Run)
    {
        if (Stage->period)
        {
            if (S->currentStagePeriod > Now)
            {
                // Skip this update; func() will be called on the specified
                // periods only.
                return true;
            }
            else 
            {
                // Prepare for the next period and call func() below
                S->currentStagePeriod += Stage->period;
            }
        }
    }

    // Stage on either Entry, Run or Exit
    if (Stage->func)
    {
        Stage->func (S, S->stagesParam, S->currentStageAction,
                     Now - S->currentStageStarted);
    }

    // Stage status update
    if (S->currentStageAction == SEQUENCE_Action_Entry)
    {
        S->currentStageAction = SEQUENCE_Action_Run;
    }
    else if (S->currentStageAction == SEQUENCE_Action_Exit)
    {
        if (++ S->currentStage >= S->stagesCount)
        {
            // End of sequence reached
            return false;
        }

        // New current stage
        S->currentStageAction = SEQUENCE_Action_Entry;
    }

    // Keep processing sequences
    return true;
}


void SEQUENCE_ExitStage (struct SEQUENCE *const S)
{
    BOARD_AssertParams (S);
    BOARD_AssertState  (S->currentStageAction == SEQUENCE_Action_Run);

    S->exitRequested = true;
}

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

#pragma once

#include "embedul.ar/source/core/timer.h"
#include <stdbool.h>


// uint32 segmented as four 8-bit octets: 0xTT332211
// where TT is the sequence input type and 33, 22 and 11 are type parameters.
#define SEQUENCE_INPUT__TYPE_MASK           0xFF000000
#define SEQUENCE_INPUT__TYPE_ANY            0x01000000
#define SEQUENCE_INPUT__TYPE_BIT            0x02000000
#define SEQUENCE_INPUT__TYPE_RANGE          0x03000000
#define SEQUENCE_INPUT__TYPE_ROLE           0x04000000
#if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
#define SEQUENCE_INPUT__TYPE_BIT_ACTION     0x05000000
#endif

#define SEQUENCE_INPUT__S1(_1)              (_1 & 0xFF)
#define SEQUENCE_INPUT__S2(_1,_2)           (SEQUENCE_INPUT__S1(_2) << 8) | \
                                            (SEQUENCE_INPUT__S1(_1)
#define SEQUENCE_INPUT__GT(_input)          (_input & SEQUENCE_INPUT__TYPE_MASK)
#define SEQUENCE_INPUT__G1(_input)          (_input & 0xFF)
#define SEQUENCE_INPUT__G2(_input)          ((_input >> 8) & 0xFF)


#define SEQUENCE_INPUT_ANY                  (SEQUENCE_INPUT__TYPE_ANY)
#define SEQUENCE_INPUT_BIT(_bit)            (SEQUENCE_INPUT__TYPE_BIT | \
                                             (_bit & 0xFF))
#define SEQUENCE_INPUT_RANGE(_begin,_end)   (SEQUENCE_INPUT__TYPE_RANGE | \
                                             ((_end & 0xFF) << 8) | \
                                             (_begin & 0xFF))
#define SEQUENCE_INPUT_ROLE(_role)          (SEQUENCE_INPUT__TYPE_ROLE | \
                                             (_role & 0xFF))
#if (LIB_EMBEDULAR_CONFIG_INPUT_SWITCH_ACTION == 1)
#define SEQUENCE_INPUT_BIT_ACTION(_bit,_action) \
                                            (SEQUENCE_INPUT__TYPE_BIT_ACTION | \
                                             (_action << 8) | _bit)
#endif


enum SEQUENCE_Action
{
    SEQUENCE_Action_Entry = -2,
    SEQUENCE_Action_Exit = -1,
    SEQUENCE_Action_Run = 0
};


struct SEQUENCE_Stage;


struct SEQUENCE
{
    const struct SEQUENCE_Stage     * stages;
    uint32_t                        stagesCount;
    void                            * stagesParam;
    uint32_t                        currentStage;
    TIMER_Ticks                     currentStageStarted;
    TIMER_Ticks                     currentStagePeriod;
    TIMER_Ticks                     currentStageTimeout;
    enum SEQUENCE_Action            currentStageAction;
    bool                            exitRequested;
};


typedef void (* SEQUENCE_StageFunc) (struct SEQUENCE *const S, 
                                     void *const Param,
                                     const enum SEQUENCE_Action Action,
                                     const TIMER_Ticks Elapsed);


struct SEQUENCE_Stage
{
    SEQUENCE_StageFunc              func;
    uint32_t                        period;
    uint32_t                        timeout;
    uint32_t                        input;
};


void    SEQUENCE_Init          (struct SEQUENCE *const S,
                                const struct SEQUENCE_Stage *const Stages,
                                const uint32_t StagesCount,
                                void *const StagesParam);
bool    SEQUENCE_Update        (struct SEQUENCE *const S);
void    SEQUENCE_ExitStage     (struct SEQUENCE *const S);
void    SEQUENCE_Restart       (struct SEQUENCE *const S);

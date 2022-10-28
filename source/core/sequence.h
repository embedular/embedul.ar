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
#include "embedul.ar/source/core/manager/input/profile.h"


enum SEQUENCE_InputType
{
    SEQUENCE_InputType_None = 0,
    SEQUENCE_InputType_Bit,
    SEQUENCE_InputType_AnyBit,
#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    SEQUENCE_InputType_BitAction,
#endif
    SEQUENCE_InputType_Range
};


enum SEQUENCE_Action
{
    SEQUENCE_Action_Entry = -2,
    SEQUENCE_Action_Exit = -1,
    SEQUENCE_Action_Run = 0
};


struct SEQUENCE;


typedef void (* SEQUENCE_StageFunc) (struct SEQUENCE *const Sequence, 
                                     void *const Param,
                                     const enum SEQUENCE_Action Action,
                                     const TIMER_Ticks Elapsed);


struct SEQUENCE_StageInput
{
    enum SEQUENCE_InputType             type;
    union 
    {
        enum INPUT_PROFILE_Type         profileType;
        enum INPUT_PROFILE_SelectFlag   profileSelectFlags;
    };
    IO_Code                             profileCode;
    IO_Value                            rangeMin;
    IO_Value                            rangeMax;
#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    enum INPUT_ACTION_Type              action;
#endif
};


#define SEQUENCE_INPUT_BIT(_ptype,_pcode) \
    (struct SEQUENCE_StageInput) { \
        .type           = SEQUENCE_InputType_Bit, \
        .profileType    = _ptype, \
        .profileCode    = _pcode \
    }

#define SEQUENCE_INPUT_ANY_BIT(_pflags) \
    (struct SEQUENCE_StageInput) { \
        .type               = SEQUENCE_InputType_AnyBit, \
        .profileSelectFlags = _pflags \
    }

#if (LIB_EMBEDULAR_CONFIG_INPUT_ACTION == 1)
    #define SEQUENCE_INPUT_BIT_ACTION(_ptype,_pcode,_action) \
        (struct SEQUENCE_StageInput) { \
            .type           = SEQUENCE_InputType_BitAction, \
            .profileType    = _ptype, \
            .profileCode    = _pcode, \
            .action         = _action \
        }
#endif

#define SEQUENCE_INPUT_RANGE(_ptype,_pcode,_min,_max) \
    (struct SEQUENCE_StageInput) { \
        .type           = SEQUENCE_InputType_Range, \
        .profileType    = _ptype, \
        .profileCode    = _pcode, \
        .rangeMin       = _min, \
        .rangeMax       = _max \
    }


struct SEQUENCE_Stage
{
    SEQUENCE_StageFunc              func;
    uint32_t                        period;
    uint32_t                        timeout;
    struct SEQUENCE_StageInput      input;
};


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


void    SEQUENCE_Init          (struct SEQUENCE *const S,
                                const struct SEQUENCE_Stage *const Stages,
                                const uint32_t StagesCount,
                                void *const StagesParam);
bool    SEQUENCE_Update        (struct SEQUENCE *const S);
void    SEQUENCE_ExitStage     (struct SEQUENCE *const S);
void    SEQUENCE_Restart       (struct SEQUENCE *const S);

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] animations by linear interpolation.

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

#include "embedul.ar/source/core/anim.h"
#include "embedul.ar/source/core/device/board.h"


/**
 * Start to process an animation according to passed parameters.
 *
 * :param Type: Animation flow to follow.
 * :param VBegin: Interpolated value initial state.
 * :param VEnd: Interpolated value end state.
 * :param Delay: Amount of milliseconds to stop at ``VBegin``.
 * :param Phase: ``VBegin`` to ``VEnd`` period, in milliseconds.
 * :param Duration: Total duration, including ``Delay`` and ``Phase``, in
 *                  milliseconds. Depends on animation type, see 
 *                  :c:enum:`ANIM_Type`.
 * :param Repeat: Animation repeat count. For infinite repeats use
 *                :c:macro:`ANIM_REPEAT_FOREVER`.
 */
void ANIM_Start (struct ANIM *const A, const enum ANIM_Type Type,
                 const uint32_t VBegin, const uint32_t VEnd, 
                 const uint32_t Delay, const uint32_t Phase,
                 const uint32_t Duration, const uint32_t Repeat)
{
    BOARD_AssertParams (A);

    OBJECT_Clear (A);

	if (Type == ANIM_Type_None || (!Phase && !Duration) || VBegin == VEnd)
	{
        A->type     = ANIM_Type_None;
        A->vCurrent = VEnd;
		return;
	}

    A->type     = Type;
	A->vBegin	= VBegin;
	A->vEnd     = VEnd;
	A->delay	= Delay;
	A->phase	= Phase;
	A->duration = (Phase > Duration)? Phase : Duration;
	A->repeat	= Repeat;
	A->vCurrent = (Phase)? VBegin : VEnd;

	const TIMER_Ticks Now = TICKS_Now ();

	if (!Delay)
	{
		A->delayStarted	= 0;
		A->animStarted	= Now;
		A->phaseStarted	= (Phase)? Now : 0;
	}
	else
    {
		A->delayStarted	= Now;
		A->animStarted	= 0;
		A->phaseStarted	= 0;
	}
}


/**
 * Used to fast-forward an animation. Repeatedly going back and forth in time
 * may cause undefined behavior. You have been warned.
 *
 * :param Delta: Amount of time to fast-forward the animation, in
 *               milliseconds.
 */
void ANIM_TimeShift (struct ANIM *const A, const uint32_t Delta)
{
    BOARD_AssertParams (A);

    if (A->type != ANIM_Type_None)
    {
        A->delta = Delta;
    }
}


/**
 * Update animation state with millisecond granularity at most.
 */
void ANIM_Update (struct ANIM *const A)
{
    BOARD_AssertParams (A);

    if (A->type == ANIM_Type_None)
    {
        return;
    }

	const TIMER_Ticks Now = TICKS_Now() + A->delta;

	if (A->delayStarted)
	{
		if (Now - A->delayStarted >= A->delay)
		{
			A->delayStarted = 0;
			A->animStarted	= Now;
			A->phaseStarted	= (A->phase)? Now : 0;
		}
		else
        {
			return;
		}
	}

	const TIMER_Ticks AnimElapsed = Now - A->animStarted;

	if (AnimElapsed >= A->duration)
	{
		// End in vBegin only if the animation type is ping-pong
        // (vBegin-vEnd-vBegin) and if phase time is different from duration
		A->vCurrent = (A->type == ANIM_Type_PingPong
                       && A->phase != A->duration)? A->vBegin : A->vEnd;

		if (!A->repeat)
		{
			A->type = ANIM_Type_None;
			return;
		}

		if (A->repeat != ANIM_REPEAT_FOREVER)
		{
			-- A->repeat;
		}

		// Repeats don't have delay. Delay only occurs on the 1st animation.
		ANIM_Start (A, A->type, A->vBegin, A->vEnd, A->delay,
                    A->phase, A->duration, A->repeat);
		return;
	}

	const TIMER_Ticks PhaseElapsed = Now - A->phaseStarted;

	// In phase time...
	if (A->phaseStarted)
	{
		// Status change
		if (PhaseElapsed >= A->phase)
		{
			A->phaseStarted	= 0;
			A->vCurrent		= A->vEnd;
		}
		else //if (a->type == ANIM_Type_PingPong)
		{
            const float Dp = (float)PhaseElapsed / (float)A->phase;
			// From vBegin to vEnd
            if (A->vEnd > A->vBegin)
            {
                A->vCurrent = (uint32_t)((Dp
                                          * (float)(A->vEnd - A->vBegin)
                                          + 0.4999f) + A->vBegin);
            }
            // From vEnd to vBegin
            else
            {
                A->vCurrent = (uint32_t)(((1.0f - Dp)
                                          * (float)(A->vBegin - A->vEnd)
                                          + 0.4999f) + A->vEnd);
            }
		}
	}
	// In "no phase" time
	else if (A->type == ANIM_Type_PingPong)
	{
        const float Dp = ((float)(AnimElapsed - A->phase)
                                / (float)(A->duration - A->phase));
		// From vEnd to vBegin
        if (A->vEnd > A->vBegin)
        {
            A->vCurrent = (uint32_t)(((1.0f - Dp)
                                      * (float)(A->vEnd - A->vBegin)
                                      + 0.4999f) + A->vBegin);
        }
        else
        {
            A->vCurrent = (uint32_t)((Dp
                                      * (float)(A->vBegin - A->vEnd)
                                      + 0.4999f) + A->vEnd);
        }
	}
}


/**
 * Set a static current value. The animation type will be set to 
 * :c:enum:`ANIM_Type.ANIM_Type_None`. This is usually used to stop an ongoing
 * animation.
 *
 * :param Value: New current, static value.
 */
void ANIM_SetValue (struct ANIM *const A, const uint32_t Value)
{
    BOARD_AssertParams (A);

	A->type     = ANIM_Type_None;
	A->vCurrent	= Value;
}


/**
 * Get current value.
 *
 * :return: Animation current value. Depends on :c:enum:`ANIM_Type`.
 */
uint32_t ANIM_GetValue (struct ANIM *const A)
{
    BOARD_AssertParams (A);
    return A->vCurrent;
}


/**
 * Get pending status. An animation is pending when its current type is
 * other than :c:enum:`ANIM_Type.ANIM_Type_None`.
 *
 * :return: :c:macro:`true` if pending, :c:macro:`false` otherwise.
 */
bool ANIM_Pending (struct ANIM *const A)
{
    BOARD_AssertParams (A);
    return (A->type == ANIM_Type_None)? false : true;
}

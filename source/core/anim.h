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

#pragma once

#include "embedul.ar/source/core/timer.h"
#include <stdbool.h>


/**
 * Description
 * ===========
 *
 * Performs linear interpolations and holds at specific milisecond periods to
 * animate an initial :c:type:`uint32_t` value. Use cases include LED vibrant
 * patterns and display flashes.
 *
 *
 * Design and development status
 * =============================
 *
 * Feature-complete.
 *
 *
 * Changelog
 * =========
 *
 * ======= ========== =================== ======================================
 * Version Date*      Author              Comment
 * ======= ========== =================== ======================================
 * 1.0.0   2022.9.7   sgermino            Initial release.
 * ======= ========== =================== ======================================
 *
 * \* Date format is Year.Month.Day.
 *
 *
 * API reference
 * =============
 */


/**
 * Pass this value on the ``repeat`` parameter of :c:func:`ANIM_Start` to
 * perpetually repeat an animation.
 */
#define ANIM_REPEAT_FOREVER     ((uint32_t) -1)


/**
 * Specifies the animation flow to follow through ``begin``, ``end``, ``delay``,
 * ``phase`` and ``duration`` parameters of :c:func:`ANIM_Start`.
 */
enum ANIM_Type
{
    /** No animation / animation finished. Static value assigned. */
	ANIM_Type_None = 0,
    /**
     * Holds ``vBegin`` value until ``delay`` milliseconds passed. Then linear
     * interpolates ``vBegin`` through ``vEnd`` in ``phase`` milliseconds.
     * ``vEnd`` value is on hold until completion of the animation time
     * ``duration`` also in milliseconds. */
    ANIM_Type_Blink,
    /**
     * Holds ``vBegin`` value until ``delay`` milliseconds passed. Then linear
     * interpolates ``vBegin`` through ``vEnd`` in ``phase`` milliseconds.
     * Finally, there is a linear interpolation from ``vEnd`` to ``vBegin``
     * through the rest of the animation time ``duration``, also expressed in
     * milliseconds.*/
	ANIM_Type_PingPong
};


/**
 * The user should treat this as an opaque structure. No member should be
 * directly accessed or modified.
 */
struct ANIM
{
	enum ANIM_Type  type;           // Current animation type
	uint32_t        delay;
	uint32_t        phase;
	uint32_t        duration;
	uint32_t        repeat;
	TIMER_Ticks     delayStarted;   // Timestamp at delay start
	TIMER_Ticks     animStarted;    // Timestamp at animation start
	TIMER_Ticks     phaseStarted;   // Timestamp at phase start
	uint32_t        delta;          // delta + NOW, for fast-forward
	uint32_t        vBegin;         // Value set at start
	uint32_t        vEnd;           // Target value
	uint32_t        vCurrent;       // Current value
};


void        ANIM_Start          (struct ANIM *const A,
                                 const enum ANIM_Type Type,
                                 const uint32_t VBegin, const uint32_t VEnd, 
                                 const uint32_t Delay, const uint32_t Phase,
                                 const uint32_t Duration, 
                                 const uint32_t Repeat,
                                 const TIMER_Ticks Now);
void        ANIM_TimeShift      (struct ANIM *const A, const uint32_t Delta);
void        ANIM_Update         (struct ANIM *const A, const TIMER_Ticks Now);
void        ANIM_SetValue       (struct ANIM *const A, const uint32_t Value);
uint32_t    ANIM_GetValue       (struct ANIM *const A);
bool        ANIM_Pending        (struct ANIM *const A);

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] periodic system clock data types.

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

#include <stdint.h>


/**
 * Description
 * ===========
 *
 * Timer types are used in the :c:struct:`SYSTEM` timer interface and wherever
 * is needed to track time passed.
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
 * ======= ========= ==================== ======================================
 * Version Date      Author               Comment
 * ======= ========= ==================== ======================================
 * 1.0.0   2021.9.07 sgermino             Initial release.
 * ======= ========= ==================== ======================================
 */
 

/**
 * 64-bit data type to store timer ticks. A 32-bit data type is not enough
 * since, at 1000 Hz, it will overrun in 49 days of continuous operation. On the
 * other hand, a 64-bit value allows the timer to keep incrementing for several
 * hundred million years or, at 100 Hz, until the Sun becomes a red giant
 * engulfing Earth and this device with it.
 */
typedef uint64_t TIMER_Ticks;


/**
 * Tick hook function pointer.
 */
typedef void (* TIMER_TickHookFunc) (const TIMER_Ticks Ticks);

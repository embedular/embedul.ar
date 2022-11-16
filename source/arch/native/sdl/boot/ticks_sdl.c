/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [TICKS driver] arm cortex systick.

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

#include "embedul.ar/source/core/device/ticks.h"
#include "embedul.ar/source/core/device/board.h"
#include "SDL.h"


struct TICKS_SDL
{
    struct TICKS    device;
};


static struct TICKS_SDL s_ticks_sdl;


static TIMER_Ticks      now             (struct TICKS *const T);
static void             delay           (struct TICKS *const T,
                                         const TIMER_Ticks Ticks);


static const struct TICKS_IFACE TICKS_SDL_IFACE =
{
    .Description    = "sdl ticks",
    .Now            = now,
    .Delay          = delay
};


void TICKS__boot (void)
{
    TICKS_Init ((struct TICKS *)&s_ticks_sdl, &TICKS_SDL_IFACE);
}


static TIMER_Ticks now (struct TICKS *const T)
{
    (void) T;

    // SDL_GetTicks uses a fixed tick duration of 1 ms
    return (TIMER_Ticks) SDL_GetTicks ();
}


static void delay (struct TICKS *const T, const TIMER_Ticks Ticks)
{
    (void) T;

    // 1 Tick = 1 millisecond
    SDL_Delay ((uint32_t)Ticks);
}

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] SDL keyboard.

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

#include "embedul.ar/source/core/device/io.h"
#include "embedul.ar/source/core/bitfield.h"
#include "SDL_scancode.h"


#define IO_KEYBOARD_SDL_PORT_COUNT      1


enum IO_KEYBOARD_SDL_INB
{
    // Use SDL_SCANCODE_* as DriverCode
    IO_KEYBOARD_SDL_INB__COUNT = SDL_NUM_SCANCODES
};


enum IO_KEYBOARD_SDL_INR
{
    IO_KEYBOARD_SDL_INR__COUNT
};


enum IO_KEYBOARD_SDL_OUTB
{
    IO_KEYBOARD_SDL_OUTB__COUNT
};


enum IO_KEYBOARD_SDL_OUTR
{
    IO_KEYBOARD_SDL_OUTR__COUNT
};


struct IO_KEYBOARD
{
    struct IO           device;
    struct IO_PortInfo  portInfo[IO_KEYBOARD_SDL_PORT_COUNT];
    uint32_t            inputBuffer[BITFIELD_COUNT(SDL_NUM_SCANCODES)];
    struct BITFIELD     inputBitfield;
};


void    IO_KEYBOARD_Init    (struct IO_KEYBOARD *const K);
void    IO_KEYBOARD_Attach  (struct IO_KEYBOARD *const K);

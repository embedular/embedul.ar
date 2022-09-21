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

#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/io_keyboard_sdl.h"
#include "embedul.ar/source/core/device/board.h"


static void         update              (struct IO *const Io);
static IO_Count     availableInputs     (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t InputSource);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t InputSource);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);


static const struct IO_IFACE IO_KEYBOARD_SDL_IFACE =
{
    .Description        = "sdl keyboard",
    .Update             = update,
    .AvailableInputs    = availableInputs,
    .AvailableOutputs   = UNSUPPORTED,
    .GetInput           = getInput,
    .SetOutput          = UNSUPPORTED,
    .InputName          = inputName,
    .OutputName         = UNSUPPORTED
};


void IO_KEYBOARD_SDL_Init (struct IO_KEYBOARD_SDL *const K)
{
    BOARD_AssertParams (K);

    DEVICE_IMPLEMENTATION_Clear (K);

    BITFIELD_Init (&K->inputBf, K->inputStatus,
                   IO_KEYBOARD_SDL_STATUS_COUNT, NULL, 0);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)K, &IO_KEYBOARD_SDL_IFACE, 15);
}


void IO_KEYBOARD_SDL_Attach (struct IO_KEYBOARD_SDL *const K)
{
    INPUT_SetDevice ((struct IO *) K, 0);

    // Default keymappings
    INPUT_MapBit (INPUT_Bit_BoardA, SDLK_0);
    INPUT_MapBit (INPUT_Bit_BoardB, SDLK_1);
    INPUT_MapBit (INPUT_Bit_BoardC, SDLK_2);
    INPUT_MapBit (INPUT_Bit_BoardD, SDLK_3);

    INPUT_MapBit (INPUT_Bit_P1Right, SDLK_RIGHT);
    INPUT_MapBit (INPUT_Bit_P1Left, SDLK_LEFT);
    INPUT_MapBit (INPUT_Bit_P1Down, SDLK_DOWN);
    INPUT_MapBit (INPUT_Bit_P1Up, SDLK_UP);
    INPUT_MapBit (INPUT_Bit_P1Start, SDLK_KP_ENTER);
    INPUT_MapBit (INPUT_Bit_P1Select, SDLK_KP_PLUS);
    INPUT_MapBit (INPUT_Bit_P1A, SDLK_KP1);
    INPUT_MapBit (INPUT_Bit_P1B, SDLK_KP2);
    INPUT_MapBit (INPUT_Bit_P1C, SDLK_KP3);
    INPUT_MapBit (INPUT_Bit_P1X, SDLK_KP4);
    INPUT_MapBit (INPUT_Bit_P1Y, SDLK_KP5);
    INPUT_MapBit (INPUT_Bit_P1Z, SDLK_KP6);

    INPUT_MapBit (INPUT_Bit_P2Right, SDLK_d);
    INPUT_MapBit (INPUT_Bit_P2Left, SDLK_a);
    INPUT_MapBit (INPUT_Bit_P2Down, SDLK_s);
    INPUT_MapBit (INPUT_Bit_P2Up, SDLK_w);
    INPUT_MapBit (INPUT_Bit_P2Start, SDLK_RETURN);
    INPUT_MapBit (INPUT_Bit_P2Select, SDLK_BACKSLASH);
    INPUT_MapBit (INPUT_Bit_P2A, SDLK_j);
    INPUT_MapBit (INPUT_Bit_P2B, SDLK_k);
    INPUT_MapBit (INPUT_Bit_P2C, SDLK_l);
    INPUT_MapBit (INPUT_Bit_P2X, SDLK_u);
    INPUT_MapBit (INPUT_Bit_P2Y, SDLK_i);
    INPUT_MapBit (INPUT_Bit_P2Z, SDLK_o);
}


void update (struct IO *const Io)
{
    struct IO_KEYBOARD_SDL *const K = (struct IO_KEYBOARD_SDL *) Io;

    SDL_PumpEvents ();

    const Uint8 * Keys = SDL_GetKeyState (NULL);

    memset (K->inputStatus, 0, sizeof(K->inputStatus));

    for (uint32_t i = 0; i < SDLK_LAST; ++i)
    {
        BITFIELD_SetBit (&K->inputBf, i, Keys[i]);
    }
}


IO_Count availableInputs (struct IO *const Io, const enum IO_Type IoType,
                          const uint32_t InputSource)
{
    (void) Io;
    (void) InputSource;

    // A keyboard has no analog inputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    }

    return SDLK_LAST;
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const uint16_t Index, const uint32_t InputSource)
{
    BOARD_AssertParams (IoType == IO_Type_Bit && Index < SDLK_LAST);
    
    (void) InputSource;

    struct IO_KEYBOARD_SDL *const K = (struct IO_KEYBOARD_SDL *) Io;

    return BITFIELD_GetBit (&K->inputBf, Index);
}


const char * inputName (struct IO *const Io,
                        const enum IO_Type IoType,
                        const uint16_t Index)
{
    BOARD_AssertParams (IoType == IO_Type_Bit && Index < SDLK_LAST);

    (void) Io;

    return SDL_GetKeyName (Index);
}

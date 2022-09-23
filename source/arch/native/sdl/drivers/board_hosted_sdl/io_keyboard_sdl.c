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
    INPUT_MapBit (INPUT_Bit_BoardA, SDL_SCANCODE_0);
    INPUT_MapBit (INPUT_Bit_BoardB, SDL_SCANCODE_1);
    INPUT_MapBit (INPUT_Bit_BoardC, SDL_SCANCODE_2);
    INPUT_MapBit (INPUT_Bit_BoardD, SDL_SCANCODE_3);

    INPUT_MapBit (INPUT_Bit_P1Right, SDL_SCANCODE_RIGHT);
    INPUT_MapBit (INPUT_Bit_P1Left, SDL_SCANCODE_LEFT);
    INPUT_MapBit (INPUT_Bit_P1Down, SDL_SCANCODE_DOWN);
    INPUT_MapBit (INPUT_Bit_P1Up, SDL_SCANCODE_UP);
    INPUT_MapBit (INPUT_Bit_P1Start, SDL_SCANCODE_KP_ENTER);
    INPUT_MapBit (INPUT_Bit_P1Select, SDL_SCANCODE_KP_PLUS);
    INPUT_MapBit (INPUT_Bit_P1A, SDL_SCANCODE_KP_1);
    INPUT_MapBit (INPUT_Bit_P1B, SDL_SCANCODE_KP_2);
    INPUT_MapBit (INPUT_Bit_P1C, SDL_SCANCODE_KP_3);
    INPUT_MapBit (INPUT_Bit_P1X, SDL_SCANCODE_KP_4);
    INPUT_MapBit (INPUT_Bit_P1Y, SDL_SCANCODE_KP_5);
    INPUT_MapBit (INPUT_Bit_P1Z, SDL_SCANCODE_KP_6);

    INPUT_MapBit (INPUT_Bit_P2Right, SDL_SCANCODE_D);
    INPUT_MapBit (INPUT_Bit_P2Left, SDL_SCANCODE_A);
    INPUT_MapBit (INPUT_Bit_P2Down, SDL_SCANCODE_S);
    INPUT_MapBit (INPUT_Bit_P2Up, SDL_SCANCODE_W);
    INPUT_MapBit (INPUT_Bit_P2Start, SDL_SCANCODE_RETURN);
    INPUT_MapBit (INPUT_Bit_P2Select, SDL_SCANCODE_BACKSLASH);
    INPUT_MapBit (INPUT_Bit_P2A, SDL_SCANCODE_J);
    INPUT_MapBit (INPUT_Bit_P2B, SDL_SCANCODE_K);
    INPUT_MapBit (INPUT_Bit_P2C, SDL_SCANCODE_L);
    INPUT_MapBit (INPUT_Bit_P2X, SDL_SCANCODE_U);
    INPUT_MapBit (INPUT_Bit_P2Y, SDL_SCANCODE_I);
    INPUT_MapBit (INPUT_Bit_P2Z, SDL_SCANCODE_O);
}


void update (struct IO *const Io)
{
    struct IO_KEYBOARD_SDL *const K = (struct IO_KEYBOARD_SDL *) Io;

    SDL_PumpEvents ();

    int numKeys = 0;
    const Uint8 *const KeyStates = SDL_GetKeyboardState (&numKeys);

    memset (K->inputStatus, 0, sizeof(K->inputStatus));

    for (int i = 0; i < numKeys; ++i)
    {
        BITFIELD_SetBit (&K->inputBf, i, KeyStates[i]);
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

    // In reality it depends on the keyboard. Besides, SDL_NUM_SCANCODES is a
    // reserved, maximum amount. There are like 200 undefined scancodes on the
    // higher side. Nevertheless, the driver supports them all.
    return SDL_NUM_SCANCODES;
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const uint16_t Index, const uint32_t InputSource)
{
    BOARD_AssertParams (IoType == IO_Type_Bit && Index < SDL_NUM_SCANCODES);
    
    (void) InputSource;

    struct IO_KEYBOARD_SDL *const K = (struct IO_KEYBOARD_SDL *) Io;

    return BITFIELD_GetBit (&K->inputBf, Index);
}


const char * inputName (struct IO *const Io,
                        const enum IO_Type IoType,
                        const uint16_t Index)
{
    BOARD_AssertParams (IoType == IO_Type_Bit && Index < SDL_NUM_SCANCODES);

    (void) Io;

    const SDL_Keycode Keycode = SDL_GetKeyFromScancode (Index);

    return SDL_GetKeyName (Keycode);
}

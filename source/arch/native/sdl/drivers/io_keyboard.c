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

#include "embedul.ar/source/arch/native/sdl/drivers/io_keyboard.h"
#include "embedul.ar/source/core/device/board.h"


static void         update              (struct IO *const Io);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_KEYBOARD_IFACE =
{
    IO_IFACE_DECLARE("sdl keyboard", KEYBOARD_SDL),
    .Update             = update,
    .GetInput           = getInput,
    .InputName          = inputName
};


void IO_KEYBOARD_Init (struct IO_KEYBOARD *const K)
{
    BOARD_AssertParams (K);

    DEVICE_IMPLEMENTATION_Clear (K);

    IO_INIT_STATIC_PORT_INFO (K, KEYBOARD_SDL);

    BITFIELD_Init (&K->inputBitfield, K->inputBuffer,
                   BITFIELD_COUNT(SDL_NUM_SCANCODES), NULL, 0);

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)K, &IO_KEYBOARD_IFACE, K->portInfo, 15);
}


void IO_KEYBOARD_Attach (struct IO_KEYBOARD *const K)
{
    INPUT_RegisterGateway ((struct IO *) K, 0);

    // Default keymappings
    INPUT_MAP_BIT (GP1, Right, SDL_SCANCODE_RIGHT);
    INPUT_MAP_BIT (GP1, Left, SDL_SCANCODE_LEFT);
    INPUT_MAP_BIT (GP1, Down, SDL_SCANCODE_DOWN);
    INPUT_MAP_BIT (GP1, Up, SDL_SCANCODE_UP);
    INPUT_MAP_BIT (GP1, Start, SDL_SCANCODE_KP_ENTER);
    INPUT_MAP_BIT (GP1, Select, SDL_SCANCODE_KP_PLUS);
    INPUT_MAP_BIT (GP1, A, SDL_SCANCODE_KP_1);
    INPUT_MAP_BIT (GP1, B, SDL_SCANCODE_KP_2);
    INPUT_MAP_BIT (GP1, C, SDL_SCANCODE_KP_3);
    INPUT_MAP_BIT (GP1, X, SDL_SCANCODE_KP_4);
    INPUT_MAP_BIT (GP1, Y, SDL_SCANCODE_KP_5);
    INPUT_MAP_BIT (GP1, Z, SDL_SCANCODE_KP_6);

    INPUT_MAP_BIT (GP2, Right, SDL_SCANCODE_D);
    INPUT_MAP_BIT (GP2, Left, SDL_SCANCODE_A);
    INPUT_MAP_BIT (GP2, Down, SDL_SCANCODE_S);
    INPUT_MAP_BIT (GP2, Up, SDL_SCANCODE_W);
    INPUT_MAP_BIT (GP2, Start, SDL_SCANCODE_RETURN);
    INPUT_MAP_BIT (GP2, Select, SDL_SCANCODE_BACKSLASH);
    INPUT_MAP_BIT (GP2, A, SDL_SCANCODE_J);
    INPUT_MAP_BIT (GP2, B, SDL_SCANCODE_K);
    INPUT_MAP_BIT (GP2, C, SDL_SCANCODE_L);
    INPUT_MAP_BIT (GP2, X, SDL_SCANCODE_U);
    INPUT_MAP_BIT (GP2, Y, SDL_SCANCODE_I);
    INPUT_MAP_BIT (GP2, Z, SDL_SCANCODE_O);
}


void update (struct IO *const Io)
{
    struct IO_KEYBOARD *const K = (struct IO_KEYBOARD *) Io;

    SDL_PumpEvents ();

    int numKeys = 0;
    const Uint8 *const KeyStates = SDL_GetKeyboardState (&numKeys);

    memset (K->inputBuffer, 0, sizeof(K->inputBuffer));

    for (int i = 0; i < numKeys; ++i)
    {
        BITFIELD_SetBit (&K->inputBitfield, i, KeyStates[i]);
    }
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) IoType;
    (void) Port;

    struct IO_KEYBOARD *const K = (struct IO_KEYBOARD *) Io;

    return BITFIELD_GetBit (&K->inputBitfield, DriverCode);
}


const char * inputName (struct IO *const Io,
                        const enum IO_Type IoType,
                        const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    const SDL_Keycode Keycode = SDL_GetKeyFromScancode (DriverCode);

    return SDL_GetKeyName (Keycode);
}

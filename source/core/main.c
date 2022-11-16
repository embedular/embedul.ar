/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] framework managed program entry point.

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

#include "embedul.ar/source/core/main.h"


// Optionally provided by the application
CC_Weak
struct BOARD_RIG * EMBEDULAR_Board_Rig (void)
{
    return NULL;
}


// XXXX__boot() provided by build-system selected drivers
struct BOARD * BOARD__boot (const int Argc, const char **const Argv,
                            struct BOARD_RIG *const R);
void OSWRAP__boot (void);

// BOARD device interface private definition
int BOARD__run (struct BOARD *const B);


int main (const int Argc, const char **const Argv)
{
    // The build system includes the proper definition of XXXX__boot() according
    // to the system's configuration. That function is in charge of initializing
    // the corresponding driver interface. All drivers initialized this way are
    // singletons.

    struct BOARD *const B = BOARD__boot (Argc, Argv, EMBEDULAR_Board_Rig());
    // -------------------------------------------------------------------------
    // Assert checks available, but still NO assert output or log messages.
    // -------------------------------------------------------------------------
    OSWRAP__boot ();

    // BOARD init sequence, main application launch, and board shutdown at exit.
    const int RetVal = BOARD__run (B);

    return RetVal;
}

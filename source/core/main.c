/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] managed program entry point.

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


// Provided by board driver
void BOARD_Boot (struct BOARD_RIG *const R);

// Board device "private" function
void BOARD__stage_shutdown (void);


int main (const int Argc, const char *const Argv[])
{
    BOARD_Boot (EMBEDULAR_Board_Rig());

    int retVal = 0;

    LOG_ContextBegin (NOBJ, LANG_APPLICATION);
    {
        LOG_Items (1, LANG_NAME, CC_AppNameStr);
        LOG_Items (1, LANG_VERSION, CC_VcsAppVersionStr);

        LOG (NOBJ, LANG_ENTERING_APP_MAIN);
        retVal = EMBEDULAR_Main (Argc, Argv);
        LOG (NOBJ, LANG_RETURNED_FROM_APP_MAIN);
        LOG_Items (1, LANG_RETURN_VALUE, (int64_t)retVal);
    }
    LOG_ContextEnd ();

    BOARD__stage_shutdown ();

    return retVal;
}

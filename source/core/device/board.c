/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD] platform device interface (singleton).

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

#include "embedul.ar/source/core/device/board.h"


#define BOARD_DEFAULT_SEED             0xbf8e18f1a02a2e49
#define BOARD_STRARG_FMT_MAX_SIZE      512


static struct BOARD * s_board = NULL;


const char * BOARD_Init (struct BOARD *const B,
                         const struct BOARD_IFACE *const Iface,
                         const int Argc, const char **const Argv,
                         struct BOARD_RIG *const R)
{
    // -------------------------------------------------------------------------
    // NO assert checks, assert output or log available.
    // -------------------------------------------------------------------------
    if (s_board)
    {
        return LANG_ASSERT_INVALID_STATE;
    }

    if (!B || !Iface)
    {
        return LANG_ASSERT_INVALID_PARAMS;
    }

    // Required interface elements
    if (!Iface->Description ||
        !Iface->StageChange ||
        !Iface->Assert)
    {
        return LANG_ASSERT_INVALID_INTERFACE;
    }

    OBJECT_Clear (B);

    s_board         = B;
    s_board->iface  = Iface;
    s_board->argc   = Argc;
    s_board->argv   = Argv;
    s_board->rig    = R;
    // -------------------------------------------------------------------------
    // Assert checks available, but still NO assert output or log messages.
    // -------------------------------------------------------------------------
    return NULL;
}


int BOARD_Argc (void)
{
    return s_board->argc;
}


const char ** BOARD_Argv (void)
{
    return s_board->argv;
}


int BOARD_ReturnValue (void)
{
    return s_board->returnValue;
}


// STREAM_Init() checks if the device instance being initialized is the special
// debug stream during BOARD_INIT_Sequence. In that case, it should not be able
// to log its own initialization.
enum BOARD_Stage BOARD_CurrentStage (void)
{
    return s_board->currentStage;
}


void LOG__assertFailed (struct STREAM *const S, const char *const Func,
                        const char *const File, const int Line,
                        const char *const Msg);


void BOARD_AssertContext (const char *const Func, const char *const File,
                           const int Line, const bool Condition,
                           const char *const Str)
{
    if (!Condition)
    {
        LOG__assertFailed (s_board->debugStream, Func, File, Line, Str);
    }

    s_board->iface->Assert (s_board, Condition);
}


struct BOARD_RealTimeClock BOARD_RealTimeClock (void)
{
    if (s_board->iface->Rtc)
    {
        return s_board->iface->Rtc (s_board);
    }

    return (struct BOARD_RealTimeClock) {};
}


bool BOARD_SoundDeviceOk (void)
{
    return s_board->sound? true : false;
}


void OSWRAP__closeMainTask (void);


void BOARD_Exit (const int ReturnValue)
{
    OSWRAP_SuspendScheduler ();
    {
        s_board->exit           = true;
        s_board->returnValue    = ReturnValue;
    }
    OSWRAP_ResumeScheduler ();

    OSWRAP__closeMainTask ();
}


const char * BOARD_Description (void)
{
    return s_board->iface->Description;
}


bool OSWRAP__syncNow (void);


void BOARD_Sync (void)
{
    if (!OSWRAP__syncNow ())
    {
        return;
    }

    OSWRAP_SuspendScheduler ();
    {
    #ifdef LIB_EMBEDULAR_HAS_SOUND
        if (BOARD_SoundDeviceOk ())
        {
            SOUND_Mix ();
        }
    #endif

        SCREEN_Update ();

        if (s_board->iface->Update)
        {
            s_board->iface->Update (s_board);
        }

        INPUT_Update ();
        OUTPUT_Update ();
    }
    OSWRAP_ResumeScheduler ();
}

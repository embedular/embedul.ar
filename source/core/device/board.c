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


// Private definitions
void BOARD_INIT_Sequence (struct BOARD *const B);


const char * BOARD_Init (struct BOARD *const B,
                         const struct BOARD_IFACE *const Iface,
                         struct BOARD_RIG *const R)
{
    // -------------------------------------------------------------------------
    // NO assert checks, assert output or log available
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
        !Iface->Assert ||
        !Iface->TicksNow ||
        !Iface->Delay)
    {
        return LANG_ASSERT_INVALID_INTERFACE;
    }

    OBJECT_Clear (B);

    s_board         = B;
    s_board->iface  = Iface;
    s_board->rig    = R;
    // -------------------------------------------------------------------------
    // Assert checks available, but still NO assert output or log messages
    // -------------------------------------------------------------------------

    // System components init sequence
    BOARD_INIT_Sequence (s_board);

    return NULL;
}


// STREAM_Init() checks if the device instance being initialized is the special
// debug stream during BOARD_INIT_Sequence. In that case, it can't log its own
// initialization.
enum BOARD_Stage BOARD_CurrentStage (void)
{
    BOARD_AssertParams (s_board);
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


void BOARD_SetTickFreq (const uint32_t Hz)
{
    if (s_board->iface->SetTickFreq)
    {
        s_board->iface->SetTickFreq (s_board, Hz);
        s_board->tickFrequency = Hz;
    }
}


uint32_t BOARD_TickFreq (void)
{
    return s_board->tickFrequency;
}


TIMER_TickHookFunc BOARD_SetTickHook (TIMER_TickHookFunc const Func,
                                      const enum BOARD_TickHookFuncSlot Slot)
{
    if (s_board->iface->SetTickHook)
    {
        const TIMER_TickHookFunc LastFunc = 
                        s_board->iface->SetTickHook (s_board, Func, Slot);
        return LastFunc;
    }

    return NULL;
}


/**
 * .. code-block:: c
 *
 *    TIMER_Ticks begin = BOARD_TicksNow ();
 *    // A hypothetical function that performs some work
 *    DoSomeWork ();
 *    TIMER_Ticks end = BOARD_TicksNow ();
 *    TIMER_Ticks elapsed = end - begin;
 */
TIMER_Ticks BOARD_TicksNow (void)
{
    return s_board->iface->TicksNow (s_board);
}


struct BOARD_RealTimeClock BOARD_RealTimeClock (void)
{
    if (s_board->iface->Rtc)
    {
        return s_board->iface->Rtc (s_board);
    }

    return (struct BOARD_RealTimeClock) {};
}


void BOARD_Delay (const TIMER_Ticks Ticks)
{
    s_board->iface->Delay (s_board, Ticks);
}


bool BOARD_VideoDeviceOk (void)
{
    return s_board->video? true : false;
}


bool BOARD_SoundDeviceOk (void)
{
    return s_board->sound? true : false;
}


const char * BOARD_Description (void)
{
    return s_board->iface->Description;
}


void BOARD_Update (void)
{
    if (s_board->iface->Update)
    {
        s_board->iface->Update (s_board);
    }

#ifdef LIB_EMBEDULAR_HAS_SOUND
    if (BOARD_SoundDeviceOk ())
    {
        SOUND_Mix ();
    }
#endif

#ifdef LIB_EMBEDULAR_HAS_VIDEO
    if (BOARD_VideoDeviceOk ())
    {
        VIDEO_NextFrame ();
    }
#endif

    INPUT_Update ();
    OUTPUT_Update ();
}


void BOARD__stage_shutdown (void)
{
    LOG_Warn (s_board, LANG_SHUTTING_DOWN);

    s_board->iface->StageChange (s_board, BOARD_Stage_Shutdown);

    if (s_board->video && s_board->video->iface->Shutdown)
    {
        s_board->video->iface->Shutdown (s_board->video);
    }
}

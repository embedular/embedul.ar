/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD driver] SDL hosted environment target.

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

#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl.h"
#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/io_keyboard_sdl.h"
#include "embedul.ar/source/core/device/board.h"


#define DEBUG_STREAM_FILE       "stderr"
#define DISK_FILENAME           "disk.bin"

#define BOARD_LOGO_1            "`F25.d88888b  888888ba  8b`L"
#define BOARD_LOGO_2            "`F2588.`P4\"' 88`P4``8b 88`L"
#define BOARD_LOGO_3            "`F25``Y88888b. 88`P0588 88`L"
#define BOARD_LOGO_4            "`F25`P6``8b 88`P0588 88`L"
#define BOARD_LOGO_5            "`F25d8'   .8P 88`P4.8P 88`L"
#define BOARD_LOGO_6            "`F25 Y88888P  8888888P  88888888P`L2"

#define BOARD_INFO_FMT          "`M40`0`L1" \
                                "`M40`1`L2"

#define BOARD_INFO_0_NAME       "simple directmedia layer"
#define BOARD_INFO_1_VER        "version " \
                                CC_ExpStr(SDL_MAJOR_VERSION) "." \
                                CC_ExpStr(SDL_MINOR_VERSION) "." \
                                CC_ExpStr(SDL_PATCHLEVEL)


static struct BOARD_HOSTED_SDL   s_System_host;


static void *       initComponent   (struct BOARD *const B,
                                     const enum BOARD_Stage Component);
static void         assertFunc      (struct BOARD *const B,
                                     const bool Condition);
static TIMER_Ticks  ticksNow        (struct BOARD *const B);
static void         delay           (struct BOARD *const B,
                                     const TIMER_Ticks Ticks);
static void         update          (struct BOARD *const B);


static const struct BOARD_IFACE BOARD_HOSTED_SDL_IFACE =
{
    .Description    = "SDL hosted",
    .StageChange    = initComponent,
    .Assert         = assertFunc,
    .SetTickFreq    = UNSUPPORTED,
    .SetTickHook    = UNSUPPORTED,
    .TicksNow       = ticksNow,
    .Rtc            = UNSUPPORTED,
    .Delay          = delay,
    .Update         = update
};


void BOARD_Boot (struct BOARD_RIG *const R)
{
    const char * ErrorMsg = BOARD_Init ((struct BOARD *)&s_System_host,
                                        &BOARD_HOSTED_SDL_IFACE, R);
    if (ErrorMsg)
    {
        fprintf (stderr, "BOARD_Init() failed: %s.", ErrorMsg);
        exit (1);
    }
}


static void greetings (struct STREAM *const S)
{
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_1);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_2);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_3);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_4);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_5);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_LOGO_6);
    STREAM_IN_FromParsedString (S, 0, 512, BOARD_INFO_FMT,
                                           BOARD_INFO_0_NAME,
                                           BOARD_INFO_1_VER);
}


void * initComponent (struct BOARD *const B,
                      const enum BOARD_Stage Component)
{
    struct BOARD_HOSTED_SDL *const H = (struct BOARD_HOSTED_SDL *) B;

    switch (Component)
    {
        case BOARD_Stage_InitHardware:
        {
            // SDL_GetTicks uses a fixed tick duration of 1 ms
            B->tickFrequency = 1000;

            // initialize SDL
            if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
            {
                // No log messages or assert output yet
                fprintf (stderr, "SDL_Init() failed: %s.", SDL_GetError());

                BOARD_AssertInitialized (false);
                break;
            }

            atexit (SDL_Quit);

            SDL_EventState (SDL_MOUSEMOTION, SDL_IGNORE);
            break;
        }

        case BOARD_Stage_InitDebugStreamDriver:
        {
            STREAM_FILE_Init (&H->streamDebugFile, DEBUG_STREAM_FILE);
            return &H->streamDebugFile;
        }

        case BOARD_Stage_Greetings:
        {
            greetings ((struct STREAM *)&H->streamDebugFile);
            break;
        }

        case BOARD_Stage_InitRandomDriver:
        {
            RANDOM_SFMT_Init (&H->randomSfmt, 0xCACACACACACACACA);
            return &H->randomSfmt;
        }

        case BOARD_Stage_InitIOLevel1Drivers:
        {
            IO_KEYBOARD_SDL_Init    (&H->ioKbSdl);
            IO_KEYBOARD_SDL_Attach  (&H->ioKbSdl);
            break;
        }

        case BOARD_Stage_InitCommDrivers:
        {
            break;
        }

        case BOARD_Stage_InitStorageDrivers:
        {
            RAWSTOR_FILE_Init (&H->rsImageFile, DISK_FILENAME);
            STORAGE_SetDevice ((struct RAWSTOR *)&H->rsImageFile);
            break;
        }

        case BOARD_Stage_InitIOLevel2Drivers:
        {
            break;
        }

        case BOARD_Stage_InitVideoDriver:
        {
            if (VIDEO_SDL_Init (&H->videoSdl))
            {
                return &H->videoSdl;
            }
            break;
        }

        case BOARD_Stage_InitSoundDriver:
        {
            SOUND_SDL_Init (&H->soundSdl);
            return &H->soundSdl;
        }

        case BOARD_Stage_Ready:
        {
            break;
        }

        case BOARD_Stage_Shutdown:
        {
            SDL_Quit ();
            break;
        }
    }

    return NULL;
}


void assertFunc (struct BOARD *const B, const bool Condition)
{
    (void) B;

    assert (Condition);
}


TIMER_Ticks ticksNow (struct BOARD *const B)
{
    (void) B;

    return (TIMER_Ticks) SDL_GetTicks ();
}


void delay (struct BOARD *const B, const TIMER_Ticks Ticks)
{
    (void) B;

    // 1 Tick = 1 millisecond
    SDL_Delay ((uint32_t)Ticks);
}


void update (struct BOARD *const B)
{
    (void) B;

    SDL_Event event;

    while (SDL_PollEvent (&event))
    {
        if (event.type == SDL_QUIT)
        {
            exit (0);
        }
    }
}

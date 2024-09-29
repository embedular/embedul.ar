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

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/drivers/random_sfmt.h"
#include "embedul.ar/source/arch/native/sdl/drivers/io_keyboard.h"
#include "embedul.ar/source/arch/native/sdl/drivers/io_gui.h"
#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332_adapter_sim.h"
#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332_vgafb.h"
#include "embedul.ar/source/arch/native/sdl/drivers/sound_sdlmixer.h"
#include "embedul.ar/source/arch/native/sdl/drivers/stream_file.h"
#include "embedul.ar/source/arch/native/sdl/drivers/rawstor_file.h"


#define DEBUG_STREAM_FILE       "stderr"
#define DISK_FILENAME           "disk.bin"

#define BOARD_LOGO_1            "`F25.d88888b  888888ba  8b`L"
#define BOARD_LOGO_2            "`F2588.`P4\"' 88`P4``8b 88`L"
#define BOARD_LOGO_3            "`F25``Y88888b. 88`P0588 88`L"
#define BOARD_LOGO_4            "`F25`P6``8b 88`P0588 88`L"
#define BOARD_LOGO_5            "`F25d8'   .8P 88`P4.8P 88`L"
#define BOARD_LOGO_6            "`F25 Y88888P  8888888P  88888888P`L1"

#define BOARD_INFO_FMT          "`M40`0`L1" \
                                "`M40`1`L1"

#define BOARD_INFO_0_NAME       "simple directmedia layer"
#define BOARD_INFO_1_VER        "version " LIB_SDL_VERSION_STR


struct BOARD_IO_PROFILES
{
    struct INPUT_PROFILE_CONTROL    inControl;
    struct INPUT_PROFILE_GP1        inGp1;
    struct INPUT_PROFILE_GP2        inGp2;
    struct INPUT_PROFILE_LIGHTDEV   inLightdev;
    struct INPUT_PROFILE_MAIN       inMain;
    struct OUTPUT_PROFILE_CONTROL   outControl;
    struct OUTPUT_PROFILE_LIGHTDEV  outLightdev;
    struct OUTPUT_PROFILE_MARQUEE   outMarquee;
    struct OUTPUT_PROFILE_SIGN      outSign;
};


struct BOARD_HOSTED
{
    struct BOARD                    device;
    struct RANDOM_SFMT              randomSfmt;
    struct VIDEO_RGB332_ADAPTER_SIM videoAdapterSim;
    struct VIDEO_RGB332_VGAFB       videoVgafbMenu;
    //struct VIDEO_RGB332_VGAFB       videoVgafbConsole;
    struct SOUND_SDLMIXER           soundSdlmixer;
    struct IO_KEYBOARD              ioKeyboard;
    struct IO_GUI                   ioGui;
    struct STREAM_FILE              streamDebugFile;
    struct RAWSTOR_FILE             rsImageFile;
    uint32_t                        screenToWindowId[SCREEN_Role__COUNT];
};


#ifdef BSS_SECTION_BOARD
BSS_SECTION_BOARD
#endif
static struct BOARD_HOSTED s_board_host;

#ifdef BSS_SECTION_IO_PROFILES
BSS_SECTION_IO_PROFILES
#endif
static struct BOARD_IO_PROFILES s_io_profiles;


static void *       initComponent   (struct BOARD *const B,
                                     const enum BOARD_Stage Component);
static void         assertFunc      (struct BOARD *const B,
                                     const bool Condition);
static void         update          (struct BOARD *const B);


static const struct BOARD_IFACE BOARD_HOSTED_SDL_IFACE =
{
    .Description    = "sdl hosted",
    .StageChange    = initComponent,
    .Assert         = assertFunc,
    .Update         = update
};


struct BOARD * BOARD__boot (const int Argc, const char **const Argv,
                            struct BOARD_RIG *const R)
{
    struct BOARD *const B = (struct BOARD *)&s_board_host;

    const char * ErrorMsg = BOARD_Init (B, &BOARD_HOSTED_SDL_IFACE,
                                        Argc, Argv, R);
    if (ErrorMsg)
    {
        fputs ("BOARD_Init() failed: ", stderr);
        fputs (ErrorMsg, stderr);
        exit (1);
    }

    return B;
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
    struct BOARD_HOSTED *const H = (struct BOARD_HOSTED *) B;

    switch (Component)
    {
        case BOARD_Stage_InitPreTicksHardware:
        {
            // initialize SDL
            if (SDL_InitSubSystem (SDL_INIT_EVENTS) < 0 || 
                SDL_InitSubSystem (SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
            {
                // No log messages or assert output yet
                fputs ("SDL_InitSubSystem() failed: ", stderr);
                fputs (SDL_GetError(), stderr);
                fputs ("\r\n", stderr);

                BOARD_AssertInitialized (false);
                break;
            }

            atexit (SDL_Quit);

            SDL_EventState (SDL_MOUSEMOTION, SDL_IGNORE);
            break;
        }

        case BOARD_Stage_InitPostTicksHardware:
        {
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

        case BOARD_Stage_InitIOProfiles:
        {
            INPUT_PROFILE_ATTACH    (CONTROL, B, s_io_profiles.inControl);
            INPUT_PROFILE_ATTACH    (GP1, B, s_io_profiles.inGp1);
            INPUT_PROFILE_ATTACH    (GP2, B, s_io_profiles.inGp2);
            INPUT_PROFILE_ATTACH    (LIGHTDEV, B, s_io_profiles.inLightdev);
            INPUT_PROFILE_ATTACH    (MAIN, B, s_io_profiles.inMain);

            OUTPUT_PROFILE_ATTACH   (CONTROL, B, s_io_profiles.outControl);
            OUTPUT_PROFILE_ATTACH   (LIGHTDEV, B, s_io_profiles.outLightdev);
            OUTPUT_PROFILE_ATTACH   (MARQUEE, B, s_io_profiles.outMarquee);
            OUTPUT_PROFILE_ATTACH   (SIGN, B, s_io_profiles.outSign);
            break;
        }

        case BOARD_Stage_InitRandomDriver:
        {
            RANDOM_SFMT_Init (&H->randomSfmt, 0xCACACACACACACACA);
            return &H->randomSfmt;
        }

        case BOARD_Stage_InitIOLevel1Drivers:
        {
            IO_KEYBOARD_Init    (&H->ioKeyboard);
            IO_KEYBOARD_Attach  (&H->ioKeyboard);
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

        case BOARD_Stage_InitScreenDrivers:
        {
            VIDEO_RGB332_ADAPTER_SIM_Init (&H->videoAdapterSim);
            SCREEN_RegisterDevice (SCREEN_Role_Primary,
                              (struct VIDEO *)&H->videoAdapterSim);
            
            VIDEO_RGB332_VGAFB_Init (&H->videoVgafbMenu);
            SCREEN_RegisterDevice (SCREEN_Role_Menu,
                              (struct VIDEO *)&H->videoVgafbMenu);

            //VIDEO_RGB332_VGAFB_Init (&H->videoVgafbConsole);
            //SCREEN_RegisterDevice (SCREEN_Role_Console,
            //                  (struct VIDEO *)&H->videoVgafbConsole);

            H->screenToWindowId[SCREEN_Role_Primary] = 
                H->videoAdapterSim.device.windowId;

            H->screenToWindowId[SCREEN_Role_Menu] = 
                H->videoVgafbMenu.device.windowId;

            //H->screenToWindowId[SCREEN_Role_Console] = 
            //    H->videoVgafbConsole.device.windowId;
            break;
        }

        case BOARD_Stage_InitSoundDriver:
        {
            SOUND_SDL_Init (&H->soundSdlmixer);
            return &H->soundSdlmixer;
        }

        case BOARD_Stage_InitIOLevel3Drivers:
        {
            MIO_CurrentMapAction (MIO_Dir_Input, IO_MapAction_NoRemap);
            MIO_CurrentMapAction (MIO_Dir_Output, IO_MapAction_NoRemap);

            IO_GUI_Init     (&H->ioGui, SCREEN_Role_Menu);
            IO_GUI_Attach   (&H->ioGui);
            break;
        }

        case BOARD_Stage_Ready:
        {
            break;
        }

        case BOARD_Stage_ShutdownHardware:
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


static void setMouseButtonAction (struct BOARD_HOSTED *const H,
                                  const SDL_Event *const Event)
{
    // Update OverScreenType only on DOWN (it is always 0 on UP)
    if (Event->type == SDL_MOUSEBUTTONDOWN)
    {
        // BUTTONDOWN always triggers from a window app, that is, a windowID
        // higher than 0.
        BOARD_AssertState (Event->button.windowID);

        bool found = false;

        for (enum SCREEN_Role r = 0; r < SCREEN_Role__COUNT; ++r)
        {
            if (SCREEN_IsAvailable(r) && 
                H->screenToWindowId[r] == Event->button.windowID)
            {
                IO_GUI__setInputRange (&H->ioGui,
                                       IO_GUI_INR_MainPointerOverScreenType, r);
                found = true;
                break;
            }
        }

        if (!found)
        {
            BOARD_AssertState (false);
        }
    }

    // Update window position of last event, either DOWN or UP
    IO_GUI__setInputRange (&H->ioGui, IO_GUI_INR_MainPointerX, Event->button.x);
    IO_GUI__setInputRange (&H->ioGui, IO_GUI_INR_MainPointerY, Event->button.y);

    // true any time its pressed, false otherwise
    IO_GUI__setInputBit (&H->ioGui, IO_GUI_INB_MainPointerPressed,
                         Event->type == SDL_MOUSEBUTTONDOWN);
}


void update (struct BOARD *const B)
{
    struct BOARD_HOSTED *const H = (struct BOARD_HOSTED *) B;

    SDL_Event event;

    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    exit (0);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                setMouseButtonAction (H, &event);
                break;
        }
    }
}

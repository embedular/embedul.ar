/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO virtual driver] rgb332-based sdl framebuffer.

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

#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332.h"
#include "embedul.ar/source/core/device/board.h"


#define CREATE_WINDOW_FAILED_STR        "SDL_CreateWindow() failed"
#define CREATE_SURFACE_FAILED_STR       "SDL_CreateRGBSurface() failed"
#define SET_SURFACE_COLORS_FAILED_STR   "SDL_SetPaletteColors() failed"
#define DRIVER_SIGNAL_STR               "sdl window"


// This should be 16.66 ms for 60 Hz. Unfortunately, a non-realtime task on a
// desktop operating system will be scheduled with 10 ms granularity at best.
#define FRAME_PERIOD_MS       1


void VIDEO_RGB332__hardwareInit (struct VIDEO *const V)
{
    struct VIDEO_RGB332 *const S = (struct VIDEO_RGB332 *) V;

    BOARD_AssertInitialized (S->displayWidth && !(S->displayWidth & 0x7) &&
                             S->displayHeight && !(S->displayHeight & 0x7) &&
                             S->updateSurface);

    {
        char windowName[80];

        snprintf (windowName, sizeof(windowName),
                                        "%s [embedul.ar] - %s%s%s",
                                        CC_AppNameStr,
                                        OBJECT_Type(V),
                                        OBJECT_TYPE_SEPARATOR,
                                        VIDEO_Description(V));

        S->displayWindow = SDL_CreateWindow (windowName,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        S->displayWidth,
                                        S->displayHeight,
                                        0);
    }

    S->displaySurface = SDL_GetWindowSurface (S->displayWindow);

    if (!S->displayWindow || !S->displaySurface)
    {
        LOG_WarnDebug (V, CREATE_WINDOW_FAILED_STR);
        LOG_Items (1, LANG_ERROR, SDL_GetError());

        BOARD_AssertInitialized (false);
    }

    S->windowId = SDL_GetWindowID (S->displayWindow);

    S->displayBuffer = SDL_CreateRGBSurface (SDL_SWSURFACE,
                                             S->displayWidth,
                                             S->displayHeight,
                                             8,
                                             0, 0, 0, 0);
    if (!S->displayBuffer)
    {
        LOG_WarnDebug (V, CREATE_SURFACE_FAILED_STR);
        LOG_Items (1, LANG_ERROR, SDL_GetError());

        BOARD_AssertInitialized (false);
    }

    SDL_Color rgb332[256];

    for (int i = 0; i < 256; ++i)
    {
        rgb332[i].r = (uint8_t) (((i >> 5) / 7.0) * 255.0);
        rgb332[i].g = (uint8_t) ((((i >> 2) & 0x7) / 7.0) * 255.0);
        rgb332[i].b = (uint8_t) (((i & 0x3) / 3.0) * 255.0);
    }

    if (SDL_SetPaletteColors (S->displayBuffer->format->palette,
                              rgb332, 0, 256))
    {
        LOG_WarnDebug (V, SET_SURFACE_COLORS_FAILED_STR);
        LOG_Items (1, LANG_ERROR, SDL_GetError());

        BOARD_AssertInitialized (false);  
    }

    V->adapterDescription   = NULL;
    V->adapterSignal        = DRIVER_SIGNAL_STR;
    V->adapterModeline      = NULL;
    V->adapterBuild         = NULL;

    S->vbiStartedTicks = SDL_GetTicks ();
}


static uint8_t * lockSurface (SDL_Surface *const Surface)
{
    if(SDL_MUSTLOCK (Surface))
    {
        if(SDL_LockSurface(Surface) < 0)
            return NULL;
    }
    return (uint8_t *) Surface->pixels;
}


static void unlockSurface (SDL_Surface *const Surface)
{
    if(SDL_MUSTLOCK (Surface))
    {
        SDL_UnlockSurface (Surface);
    }
}


static void updateScreen (struct VIDEO *const V)
{
    struct VIDEO_RGB332 *const S = (struct VIDEO_RGB332 *) V;

    uint8_t *const Surface = lockSurface (S->displayBuffer);
    // SDL_Surface lock failed
    BOARD_AssertState (Surface);

    const struct VIDEO_RGB332_UpdateInfo Ui =
    {
        .ShowAnd32      = (uint32_t)(V->showAnd << 24 | V->showAnd  << 16 |
                                     V->showAnd <<  8 | V->showAnd),
        .ShowOr32       = (uint32_t)(V->showOr  << 24 | V->showOr   << 16 |
                                     V->showOr  <<  8 | V->showOr),
        .ScanAnd32      = (uint32_t)(V->scanAnd << 24 | V->scanAnd  << 16 |
                                     V->scanAnd <<  8 | V->scanAnd),
        .ScanOr32       = (uint32_t)(V->scanOr  << 24 | V->scanOr   << 16 |
                                     V->scanOr  <<  8 | V->scanOr),
        .Surface        = Surface,
        .SurfacePitch   = S->displayBuffer->pitch
    };

    S->updateSurface (V, &Ui);

    unlockSurface (S->displayBuffer);

    SDL_BlitSurface (S->displayBuffer, NULL, S->displaySurface, NULL);
    
    const int Result = SDL_UpdateWindowSurface (S->displayWindow);

    BOARD_AssertState (!Result);
}


bool VIDEO_RGB332__reachedVBICount (struct VIDEO *const V)
{
    struct VIDEO_RGB332 *const S = (struct VIDEO_RGB332 *) V;

    const uint32_t Now          = SDL_GetTicks ();
    const uint32_t TimeoutTicks = S->vbiStartedTicks + 
                                        (V->waitVbiCount * FRAME_PERIOD_MS);

    return (TimeoutTicks <= Now)? true : false;
}


void VIDEO_RGB332__waitForVBI (struct VIDEO *const V)
{
    struct VIDEO_RGB332 *const S = (struct VIDEO_RGB332 *) V;

    // The frame buffer is continuously sent to the video signal port on a
    // hardware device. On simulated devices, the screen update happens before
    // waiting for the VBI.
    updateScreen (V);

    if (V->waitVbiCount)
    {
        const uint32_t Now = SDL_GetTicks ();

        TIMER_Ticks timeoutTicks = S->vbiStartedTicks + 
                                        (V->waitVbiCount * FRAME_PERIOD_MS);

        // Missed the Vertical Blanking Interrupt mark
        if (timeoutTicks < Now)
        {
            // How many VBI missed
            V->vbiCountMisses += timeoutTicks / FRAME_PERIOD_MS;
            // Wait just one more VBI to synchronize
            timeoutTicks += FRAME_PERIOD_MS - (timeoutTicks % FRAME_PERIOD_MS);
        }

        // Wait for the required number of "vertical blanking intervals"
        SDL_Delay (timeoutTicks - S->vbiStartedTicks);
    }

    S->vbiStartedTicks = SDL_GetTicks ();
}


void VIDEO_RGB332__shutdown (struct VIDEO *const V)
{
    struct VIDEO_RGB332 *const S = (struct VIDEO_RGB332 *) V;

    if (S->displayWindow)
    {
        SDL_DestroyWindow (S->displayWindow);
    }
}

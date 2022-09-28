/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO driver] SDL.

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

#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/video_sdl.h"
#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/sound_sdl.h"
#include "embedul.ar/source/core/device/board.h"
#include <assert.h>


#define CREATE_WINDOW_FAILED_STR        "SDL_CreateWindow() failed"
#define CREATE_SURFACE_FAILED_STR       "SDL_CreateRGBSurface() failed"
#define SET_SURFACE_COLORS_FAILED_STR   "SDL_SetPaletteColors() failed"
#define DRIVER_SIGNAL_STR               "desktop window"


// This should be 16.66 ms for 60 Hz. Unfortunately, a non-realtime task on a
// desktop operating system will be scheduled with 10 ms granularity at best.
#define FRAME_PERIOD_MS       10


static uint8_t  s_FramebufferA[256 * 144];
static uint8_t  s_FramebufferB[256 * 144];


static bool     hardwareInit    (struct VIDEO *const V);
static bool     reachedVBICount (struct VIDEO *const V);
static void     waitForVBI      (struct VIDEO *const V);
static void     shutdown        (struct VIDEO *const V);


static const struct VIDEO_IFACE VIDEO_IFACE_SDL =
{
    .Description        = "sdl",
    .HardwareInit       = hardwareInit,
    .ReachedVBICount    = reachedVBICount,
    .WaitForVBI         = waitForVBI,
    .FrameEnd           = UNSUPPORTED,
    .FrameTransition    = UNSUPPORTED,
    .FrameBegin         = UNSUPPORTED,
    .Shutdown           = shutdown
};


bool VIDEO_SDL_Init (struct VIDEO_SDL *const S)
{
    return VIDEO_Init ((struct VIDEO *)S, &VIDEO_IFACE_SDL);
}


static bool hardwareInit (struct VIDEO *const V)
{
    struct VIDEO_SDL *const S = (struct VIDEO_SDL *) V;

    memset (s_FramebufferA, 0, sizeof(s_FramebufferA));
    memset (s_FramebufferB, 0, sizeof(s_FramebufferB));

    V->width    = 256;
    V->height   = 144;
    V->bufferA  = s_FramebufferA;
    V->bufferB  = s_FramebufferB;

    {
        char windowName[80];

        snprintf (windowName, sizeof(windowName),
                                        "%s [embedul.ar] - %s%s%s",
                                        CC_AppNameStr,
                                        OBJECT_Type(V),
                                        OBJECT_TYPE_SEPARATOR,
                                        VIDEO_Description());

        S->displayWindow = SDL_CreateWindow (windowName,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        1280, 720, 0);
    }

    S->displaySurface = SDL_GetWindowSurface (S->displayWindow);

    if (!S->displayWindow || !S->displaySurface)
    {
        LOG_WarnDebug (V, CREATE_WINDOW_FAILED_STR);
        LOG_Items (1, LANG_ERROR, SDL_GetError());

        return false;
    }

    S->displayBuffer = SDL_CreateRGBSurface (SDL_SWSURFACE,
                                             1280, 720, 8, 0, 0, 0, 0);
    if (!S->displayBuffer)
    {
        LOG_WarnDebug (V, CREATE_SURFACE_FAILED_STR);
        LOG_Items (1, LANG_ERROR, SDL_GetError());

        return false;
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

        return false;    
    }

    V->adapterDescription   = NULL;
    V->adapterSignal        = DRIVER_SIGNAL_STR;
    V->adapterModeline      = NULL;
    V->adapterBuild         = NULL;

    S->vbiStartedTicks = SDL_GetTicks ();

    return true;
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
    struct VIDEO_SDL *const S = (struct VIDEO_SDL *) V;

    // Invalid display width and/or height
    BOARD_AssertState (V->width == 256 && V->height == 144);

    uint8_t *s = lockSurface (S->displayBuffer);
    // SDL_Surface lock failed
    BOARD_AssertState (s);

#ifdef VIDEO_UNSCALED
    for (int h = 0; h < 144; ++h)
    {
        memcpy (s, b, 256);
        b += 256;
        s += screenBuffer->pitch;
    }
#else
    const uint32_t ShowAnd4 = (uint32_t)
        (V->showAnd << 24 | V->showAnd << 16 | V->showAnd << 8 | V->showAnd);

    const uint32_t ShowOr4 = (uint32_t)
        (V->showOr << 24 | V->showOr << 16 | V->showOr << 8 | V->showOr);

    const uint32_t ScanAnd4 = (uint32_t)
        (V->scanAnd << 24 | V->scanAnd << 16 | V->scanAnd << 8 | V->scanAnd);

    const uint32_t ScanOr4 = (uint32_t)
        (V->scanOr << 24 | V->scanOr << 16 | V->scanOr << 8 | V->scanOr);

    const uint8_t Scanlines = 5 - V->scanlines;

    uint8_t *b = V->frontbuffer;
    uint8_t line[1280];

    for (int h = 0; h < 144; ++h)
    {
        for (int whd = 0; whd < 1280; whd += 5)
        {
            line[whd+0] = *b;
            line[whd+1] = *b;
            line[whd+2] = *b;
            line[whd+3] = *b;
            line[whd+4] = *b;
            b ++;
        }

        uint32_t lineShow[320];
        uint32_t lineScan[320];

        memcpy (lineShow, line, 1280);
        memcpy (lineScan, line, 1280);

        for (int i = 0; i < 320; ++i)
        {
            lineShow[i] &= ShowAnd4;
            lineShow[i] |= ShowOr4;
            lineScan[i] &= ScanAnd4;
            lineScan[i] |= ScanOr4;
        }

        for (int i = 0; i < 5; ++i)
        {
            memcpy (s, (i < Scanlines)? lineShow : lineScan, 1280);
            s += S->displayBuffer->pitch;
        }
    }
#endif

    unlockSurface (S->displayBuffer);

    SDL_BlitSurface         (S->displayBuffer, NULL, S->displaySurface, NULL);
    SDL_UpdateWindowSurface (S->displayWindow);
}


static bool reachedVBICount (struct VIDEO *const V)
{
    struct VIDEO_SDL *const S = (struct VIDEO_SDL *) V;

    const uint32_t Now          = SDL_GetTicks ();
    const uint32_t TimeoutTicks = S->vbiStartedTicks + 
                                        (V->waitVbiCount * FRAME_PERIOD_MS);

    return (TimeoutTicks <= Now)? true : false;
}


static void waitForVBI (struct VIDEO *const V)
{
    struct VIDEO_SDL *const S = (struct VIDEO_SDL *) V;

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


static void shutdown (struct VIDEO *const V)
{
    struct VIDEO_SDL *const S = (struct VIDEO_SDL *) V;

    if (S->displayWindow)
    {
        SDL_DestroyWindow (S->displayWindow);
    }
}

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  LPC4337 shared memory between software video adapter and application.

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

#pragma once

#include <stdint.h>


#ifndef VIDEO_FB_WIDTH
    #error Undefined video framebuffer width
#endif

#ifndef VIDEO_FB_HEIGHT
    #error Undefined video framebuffer height
#endif

#define VIDEO_FB_SIZE       (VIDEO_FB_WIDTH * VIDEO_FB_HEIGHT)


// 32 bytes MAX
struct VIDEO_DUALCORE_Exchange
{
    volatile uint32_t       framebuffer;    // framebuffer base address
    volatile uint32_t       frameNumber;    // frames displayed since reset
    volatile uint8_t        showAnd;        // display pixel AND value
    volatile uint8_t        showOr;         // display pixel OR value
    volatile uint8_t        scanAnd;        // scanlines pixel AND value
    volatile uint8_t        scanOr;         // scanlines pixel OR value
    volatile uint8_t        scanlines;      // number of dimmed video scanlines
                                            // from a repeated framebuffer line
    volatile uint8_t        reserved1;
    volatile uint8_t        reserved2;
    volatile uint8_t        reserved3;
    volatile uint8_t        reserved4;
    volatile uint8_t        reserved5;
    volatile uint8_t        reserved6;
    // adapter initialization error
    volatile uint8_t        errorCode;
    // Build information embedded into the adapter
    const char              * description;
    const char              * signal;
    const char              * modeline;
    const char              * build;
    // usage defined by the adapter
    union
    {
        struct
        {
            volatile uint8_t    d0;
            volatile uint8_t    d1;
            volatile uint8_t    d2;
            volatile uint8_t    d3;
            volatile uint8_t    d4;
            volatile uint8_t    d5;
            volatile uint8_t    d6;
            volatile uint8_t    d7;
        } io;
        volatile uint64_t   d;
    };
};


// Defined by target linker script
extern struct VIDEO_DUALCORE_Exchange   g_videoExchange;
extern uint8_t                          g_framebufferA [VIDEO_FB_SIZE];
extern uint8_t                          g_framebufferB [VIDEO_FB_SIZE];

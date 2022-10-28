/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO] video device interface (singleton).

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

#include "embedul.ar/source/core/timer.h"
#include <stdbool.h>


#define VIDEO_SOP_DEFAULT_SHOW_AND    0xFF
#define VIDEO_SOP_DEFAULT_SHOW_OR     0x00
#define VIDEO_SOP_DEFAULT_SCAN_AND    0x24
#define VIDEO_SOP_DEFAULT_SCAN_OR     0x00


struct VIDEO;


typedef void (* VIDEO_HardwareInitFunc)(struct VIDEO *const V);
typedef bool (* VIDEO_ReachedVBICountFunc)(struct VIDEO *const V);
typedef void (* VIDEO_WaitForVBIFunc)(struct VIDEO *const V);
typedef void (* VIDEO_FrameEndFunc)(struct VIDEO *const V);
typedef void (* VIDEO_FrameTransitionFunc)(struct VIDEO *const V);
typedef void (* VIDEO_FrameBeginFunc)(struct VIDEO *const V);
typedef void (* VIDEO_ShutdownFunc)(struct VIDEO *const V);


struct VIDEO_IFACE
{
    const char                          * const Description;
    const uint16_t                      Width;
    const uint16_t                      Height;
    const VIDEO_HardwareInitFunc        HardwareInit;
    const VIDEO_ReachedVBICountFunc     ReachedVBICount;
    const VIDEO_WaitForVBIFunc          WaitForVBI;
    const VIDEO_FrameEndFunc            FrameEnd;
    const VIDEO_FrameTransitionFunc     FrameTransition;
    const VIDEO_FrameBeginFunc          FrameBegin;
    const VIDEO_ShutdownFunc            Shutdown;
};


enum VIDEO_SOP
{
    VIDEO_SOP_ShowAnd,
    VIDEO_SOP_ShowOr,
    VIDEO_SOP_ScanAnd,
    VIDEO_SOP_ScanOr
};


struct VIDEO
{
    const struct VIDEO_IFACE        * iface;
    TIMER_Ticks                     frameStartTicks;
    TIMER_Ticks                     lastFrameBusy;
    TIMER_Ticks                     lastFramePeriod;
    uint8_t                         * frontbuffer;
    uint8_t                         * backbuffer;
    uint32_t                        frameNumber;
    uint32_t                        waitVbiCount;
    uint32_t                        vbiCountMisses;
    bool                            swapOverride;
    bool                            copyFrame;
    uint8_t                         scanlines;
    uint8_t                         showAnd;
    uint8_t                         showOr;
    uint8_t                         scanAnd;
    uint8_t                         scanOr;
    const char                      * adapterDescription;
    const char                      * adapterSignal;
    const char                      * adapterModeline;
    const char                      * adapterBuild;
};


void        VIDEO_Init                  (struct VIDEO *const V,
                                         const struct VIDEO_IFACE *const Iface,
                                         uint8_t *const FramebufferA,
                                         uint8_t *const FramebufferB);
bool        VIDEO_IsValid               (struct VIDEO *const V);
uint8_t *   VIDEO_Frontbuffer           (struct VIDEO *const V);
uint8_t *   VIDEO_Backbuffer            (struct VIDEO *const V);
uint8_t *   VIDEO_BackbufferXY          (struct VIDEO *const V,
                                         const int32_t X, const int32_t Y);
uint16_t    VIDEO_Width                 (struct VIDEO *const V);
uint16_t    VIDEO_Height                (struct VIDEO *const V);
uint32_t    VIDEO_GetBufferOctets       (struct VIDEO *const V);
void        VIDEO_SetWaitVBICount       (struct VIDEO *const V,
                                         const uint32_t Count);
void        VIDEO_SetScanlines          (struct VIDEO *const V,
                                         const uint8_t Scanlines);
void        VIDEO_SetScanlineOp         (struct VIDEO *const V,
                                         const enum VIDEO_SOP Op,
                                         const uint8_t Value);
void        VIDEO_ResetScanlineOp       (struct VIDEO *const V,
                                         const enum VIDEO_SOP Op);
void        VIDEO_ResetAllScanlineOps   (struct VIDEO *const V);
void        VIDEO_SwapOverride          (struct VIDEO *const V);
void        VIDEO_CopyFrame             (struct VIDEO *const V);
bool        VIDEO_ReachedVBICount       (struct VIDEO *const V);
void        VIDEO_WaitForVBI            (struct VIDEO *const V);
void        VIDEO_NextFrame             (struct VIDEO *const V);
uint32_t    VIDEO_FrameNumber           (struct VIDEO *const V);
void        VIDEO_Shutdown              (struct VIDEO *const V);
const char *
            VIDEO_Description           (struct VIDEO *const V);

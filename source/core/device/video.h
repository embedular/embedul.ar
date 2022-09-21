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
#include "embedul.ar/source/core/device/video/font.h"
#include "embedul.ar/source/core/device/video/rgb332.h"
#include <stdint.h>
#include <stdbool.h>


#define VIDEO_SOP_DEFAULT_SHOW_AND    0xFF
#define VIDEO_SOP_DEFAULT_SHOW_OR     0x00
#define VIDEO_SOP_DEFAULT_SCAN_AND    0x24
#define VIDEO_SOP_DEFAULT_SCAN_OR     0x00

#define VIDEO_CLIP_LEFT               0x01
#define VIDEO_CLIP_RIGHT              0x02
#define VIDEO_CLIP_TOP                0x04
#define VIDEO_CLIP_BOTTOM             0x08


typedef uint32_t        VIDEO_CLIP_Flags;


struct VIDEO;


typedef bool (* VIDEO_HardwareInitFunc)(struct VIDEO *const V);
typedef bool (* VIDEO_ReachedVBICountFunc)(struct VIDEO *const V);
typedef void (* VIDEO_WaitForVBIFunc)(struct VIDEO *const V);
typedef void (* VIDEO_FrameEndFunc)(struct VIDEO *const V);
typedef void (* VIDEO_FrameTransitionFunc)(struct VIDEO *const V);
typedef void (* VIDEO_FrameBeginFunc)(struct VIDEO *const V);


struct VIDEO_IFACE
{
    const char                          * const Description;
    const VIDEO_HardwareInitFunc        HardwareInit;
    const VIDEO_ReachedVBICountFunc     ReachedVBICount;
    const VIDEO_WaitForVBIFunc          WaitForVBI;
    const VIDEO_FrameEndFunc            FrameEnd;
    const VIDEO_FrameTransitionFunc     FrameTransition;
    const VIDEO_FrameBeginFunc          FrameBegin;
};


enum VIDEO_SOP
{
    VIDEO_SOP_ShowAnd,
    VIDEO_SOP_ShowOr,
    VIDEO_SOP_ScanAnd,
    VIDEO_SOP_ScanOr
};


struct VIDEO_Clip
{
    int32_t                     cx;
    int32_t                     cy;
    int32_t                     xLeft;
    int32_t                     xRight;
    int32_t                     yTop;
    int32_t                     yBottom;
    uint16_t                    cWidth;
    uint16_t                    cHeight;
    VIDEO_CLIP_Flags            flags;
};


struct VIDEO
{
    const struct VIDEO_IFACE        * iface;
    TIMER_Ticks                     frameStartTicks;
    TIMER_Ticks                     lastFrameBusy;
    TIMER_Ticks                     lastFramePeriod;
    uint8_t                         * bufferA;
    uint8_t                         * bufferB;
    uint8_t                         * frontbuffer;
    uint8_t                         * backbuffer;
    uint32_t                        frameNumber;
    uint32_t                        waitVbiCount;
    uint32_t                        vbiCountMisses;
    struct VIDEO_FONT               font;
    struct VIDEO_RGB332_Gradient    gradient;
    bool                            swapOverride;
    bool                            copyFrame;
    uint16_t                        width;
    uint16_t                        height;
    uint16_t                        clipX1;
    uint16_t                        clipY1;
    uint16_t                        clipX2;
    uint16_t                        clipY2;
    uint16_t                        clipWidth;
    uint16_t                        clipHeight;
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


bool        VIDEO_Init                  (struct VIDEO *const V,
                                         const struct VIDEO_IFACE *iface);
uint8_t *   VIDEO_Frontbuffer           (void);
uint8_t *   VIDEO_Backbuffer            (void);
uint8_t *   VIDEO_BackbufferXY          (const int32_t X, const int32_t Y);
uint32_t    VIDEO_BackbufferOctets      (void);
uint16_t    VIDEO_Width                 (void);
uint16_t    VIDEO_Height                (void);
void        VIDEO_SetWaitVBICount       (const uint32_t Count);
void        VIDEO_SetScanlines          (const uint8_t Scanlines);
void        VIDEO_SetScanlineOp         (const enum VIDEO_SOP Op,
                                         const uint8_t Value);
void        VIDEO_ResetScanlineOp       (const enum VIDEO_SOP Op);
void        VIDEO_ResetAllScanlineOps   (void);
void        VIDEO_SwapOverride          (void);
void        VIDEO_CopyFrame             (void);
void        VIDEO_CopyCachedFrame       (const uint32_t CachedElement);
bool        VIDEO_ReachedVBICount       (void);
void        VIDEO_WaitForVBI            (void);
void        VIDEO_NextFrame             (void);
uint32_t    VIDEO_FrameNumber           (void);
void        VIDEO_ClearBack             (const uint8_t Color);
void        VIDEO_ClearFront            (const uint8_t Color);
void        VIDEO_Zap                   (const uint8_t Color);
const struct VIDEO_FONT *
            VIDEO_Font                  (void);
const struct VIDEO_RGB332_Gradient *
            VIDEO_Gradient              (void);
void        VIDEO_GradientShift         (const int8_t Delta);
void        VIDEO_ClippingRect          (const uint16_t X1, const uint16_t Y1,
                                         const uint16_t X2, const uint16_t Y2);
uint16_t    VIDEO_ClipWidth             (void);
uint16_t    VIDEO_ClipHeight            (void);
uint16_t    VIDEO_ClipX1                (void);
uint16_t    VIDEO_ClipX2                (void);
uint16_t    VIDEO_ClipY1                (void);
uint16_t    VIDEO_ClipY2                (void);
bool        VIDEO_ScreenPointClipped    (const int32_t X, const int32_t Y);
int32_t     VIDEO_ScreenToClipX         (const int32_t X);
int32_t     VIDEO_ScreenToClipY         (const int32_t Y);
int32_t     VIDEO_ClipToScreenX         (const int32_t Cx);
int32_t     VIDEO_ClipToScreenY         (const int32_t Cy);
bool        VIDEO_ClipIsInside          (const int32_t Cx, const int32_t Cy,
                                         const uint16_t Width, 
                                         const uint16_t Height);
bool        VIDEO_Clip                  (const int32_t Cx, const int32_t Cy,
                                         const uint16_t Width,
                                         const uint16_t Height,
                                         struct VIDEO_Clip *c);
const char *
            VIDEO_Description           (void);

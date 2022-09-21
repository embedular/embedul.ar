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

#include "embedul.ar/source/core/device/video.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/manager/storage/cache.h"


static struct VIDEO *s_v = NULL;


static void clearBuffer (uint8_t *buffer, const uint8_t Color)
{
    buffer += s_v->width * s_v->clipY1 + s_v->clipX1;

    for (uint32_t i = 0; i < s_v->clipHeight; ++i)
    {
        memset (buffer, Color, s_v->clipWidth);
        buffer += s_v->clipWidth;
    }
}


bool VIDEO_Init (struct VIDEO *const V, const struct VIDEO_IFACE *iface)
{
    BOARD_AssertState  (!s_v);
    BOARD_AssertParams (V && iface);

    // Required interface elements
    BOARD_AssertInterface (iface->Description
                            && iface->HardwareInit
                            && iface->ReachedVBICount
                            && iface->WaitForVBI);
    OBJECT_Clear (V);

    V->iface            = iface;
    V->frameStartTicks  = BOARD_TicksNow ();
    V->scanlines        = 0;

    // Default font
    VIDEO_FONT_Init (&V->font, NULL);

    // Default color gradient for drawing primitives
    VIDEO_RGB332_GradientCopy (&V->gradient, &VIDEO_RGB332_GRADIENT_DEFAULT);

    // Scanlines effect off
    V->scanlines = 0;

    // Default raster lines op values
    V->showAnd  = VIDEO_SOP_DEFAULT_SHOW_AND;
    V->showOr   = VIDEO_SOP_DEFAULT_SHOW_OR;
    V->scanAnd  = VIDEO_SOP_DEFAULT_SCAN_AND;
    V->scanOr   = VIDEO_SOP_DEFAULT_SCAN_OR;

    // Full frame rate; wait for one vertical blanking interrupt
    V->waitVbiCount = 1;

    // Instance required by interface methods
    s_v = V;

    LOG_ContextBegin (V, LANG_INIT);
    {
        if (!V->iface->HardwareInit (V))
        {
            LOG_Warn (V, LANG_HARDWARE_INIT_FAILED);

            s_v = NULL;
            OBJECT_Clear (V);

            return false;
        }

        BOARD_AssertInitialized (V->bufferA && V->bufferB);

        // width and height must have been initialized with a size multiple of 8
        BOARD_AssertInitialized (V->width && V->height
                                  && !(V->width & 0x7) && !(V->height & 0x7));

        // Clipping rect to whole screen
        V->clipX1       = 0;
        V->clipY1       = 0;
        V->clipX2       = V->width - 1;
        V->clipY2       = V->height - 1;
        V->clipWidth    = V->width;
        V->clipHeight   = V->height;

        // Initial front and back buffer
        V->frontbuffer  = V->bufferA;
        V->backbuffer   = V->bufferB;

        LOG_Items (2,
                        "framebuffer width" ,V->width,
                        "height"            ,V->height);

        if (V->adapterSignal)
        {
            LOG_Items (1, LANG_SIGNAL, V->adapterSignal);
        }

        if (V->adapterModeline)
        {
            LOG_Items (1, LANG_MODELINE, V->adapterModeline);
        }

        if (V->adapterDescription)
        {
            LOG_Items (1, LANG_ADAPTER, V->adapterDescription);
        }

        if (V->adapterBuild)
        {
            LOG_Items (1, LANG_BUILD, V->adapterBuild);
        }
    }
    LOG_ContextEnd ();

    return true;
}


uint8_t * VIDEO_Frontbuffer (void)
{
    BOARD_AssertState (s_v);
    return s_v->frontbuffer;
}


uint8_t * VIDEO_Backbuffer (void)
{
    BOARD_AssertState (s_v);
    return s_v->backbuffer;
}


uint8_t * VIDEO_BackbufferXY (const int32_t X, const int32_t Y)
{
    BOARD_AssertState  (s_v);
    BOARD_AssertParams (X >= 0 && X < s_v->width && Y >= 0 && Y < s_v->height);

    return &s_v->backbuffer[Y * s_v->width + X];
}


uint32_t VIDEO_BackbufferOctets (void)
{
    BOARD_AssertState (s_v);
    return s_v->width * s_v->height;
}


uint16_t VIDEO_Width (void)
{
    BOARD_AssertState (s_v);
    return s_v->width;
}


uint16_t VIDEO_Height (void)
{
    BOARD_AssertState (s_v);
    return s_v->height;
}


void VIDEO_SetWaitVBICount (const uint32_t Count)
{
    BOARD_AssertState (s_v);
    s_v->waitVbiCount = Count;
}


void VIDEO_SetScanlines (const uint8_t Scanlines)
{
    BOARD_AssertState (s_v);
    s_v->scanlines = Scanlines;
}


void VIDEO_SetScanlineOp (const enum VIDEO_SOP Op, const uint8_t Value)
{
    BOARD_AssertState (s_v);

    switch (Op)
    {
        case VIDEO_SOP_ShowAnd:
            s_v->showAnd = Value;
            break;

        case VIDEO_SOP_ShowOr:
            s_v->showOr = Value;
            break;

        case VIDEO_SOP_ScanAnd:
            s_v->scanAnd = Value;
            break;

        case VIDEO_SOP_ScanOr:
            s_v->scanOr = Value;
            break;
    }
}


void VIDEO_ResetScanlineOp (const enum VIDEO_SOP Op)
{
    switch (Op)
    {
        case VIDEO_SOP_ShowAnd:
            VIDEO_SetScanlineOp (Op, VIDEO_SOP_DEFAULT_SHOW_AND);
            break;

        case VIDEO_SOP_ShowOr:
            VIDEO_SetScanlineOp (Op, VIDEO_SOP_DEFAULT_SHOW_OR);
            break;

        case VIDEO_SOP_ScanAnd:
            VIDEO_SetScanlineOp (Op, VIDEO_SOP_DEFAULT_SCAN_AND);
            break;

        case VIDEO_SOP_ScanOr:
            VIDEO_SetScanlineOp (Op, VIDEO_SOP_DEFAULT_SCAN_OR);
            break;
    }
}


void VIDEO_ResetAllScanlineOps (void)
{
    VIDEO_ResetScanlineOp (VIDEO_SOP_ShowAnd);
    VIDEO_ResetScanlineOp (VIDEO_SOP_ShowOr);
    VIDEO_ResetScanlineOp (VIDEO_SOP_ScanAnd);
    VIDEO_ResetScanlineOp (VIDEO_SOP_ScanOr);
}


// Dont swap buffers (on current frame only)
void VIDEO_SwapOverride (void)
{
    BOARD_AssertState (s_v);
    s_v->swapOverride = true;
}


// Copy last backbuffer to the next backbuffer (on current frame only)
void VIDEO_CopyFrame (void)
{
    BOARD_AssertState (s_v);
    s_v->copyFrame = true;
}


/*
    replaces the entire backbuffer with an equally-sized cached element.
    TODO: restrict drawing to the screen clipping region?
*/
void VIDEO_CopyCachedFrame (const uint32_t CachedElement)
{
    BOARD_AssertState (s_v);

    s_v->copyFrame = true;

    if (!STORAGE_ValidVolume (STORAGE_Role_LinearCache))
    {
        LOG_WarnDebug (s_v, LANG_NO_LINEAR_CACHE);

        // Yellow = no valid volume
        clearBuffer (s_v->backbuffer, 0xFC);
        return;
    }

    // No element to fulfill the request
    if (CachedElement >= STORAGE_CachedElementsCount())
    {
        LOG_WarnDebug (s_v, LANG_CACHED_ELEMENT_NOT_FOUND);
        LOG_Items (1, LANG_CACHED_ELEMENT, CachedElement);

        // Violet = invalid element
        clearBuffer (s_v->backbuffer, 0xE3);
        return;
    }

    // Using backbuffer as a sector buffer. Its entire contents should be
    // overwritten by the cached element.
    struct STORAGE_CACHE_ElementInfo info;

    if (STORAGE_CACHE_ElementInfo(&info, CachedElement, s_v->backbuffer, 1)
        == RAWSTOR_Status_Result_Ok)
    {
        BOARD_AssertParams (info.octets == s_v->width * s_v->height);

        if (STORAGE_CACHE_ElementData(&info, 0, 0, s_v->backbuffer, 1)
            == RAWSTOR_Status_Result_Ok)
        {
            return;
        }
    }

    // Red = cache read error
    clearBuffer (s_v->backbuffer, 0xE0);
}


bool VIDEO_ReachedVBICount (void)
{
    BOARD_AssertState (s_v);
    return s_v->iface->ReachedVBICount (s_v);
}


void VIDEO_WaitForVBI (void)
{
    BOARD_AssertState (s_v);
    s_v->iface->WaitForVBI (s_v);
}


void VIDEO_NextFrame (void)
{
    BOARD_AssertState (s_v);

    // frameEnd/Transition/Begin are interface optional notifications
    if (s_v->iface->FrameEnd)
    {
        s_v->iface->FrameEnd (s_v);
    }

    s_v->lastFrameBusy = BOARD_TicksNow() - s_v->frameStartTicks;

    // Required interface implementation
    s_v->iface->WaitForVBI (s_v);

    const uint8_t * LastFront = s_v->frontbuffer;
    const uint8_t * LastBack = s_v->backbuffer;
    (void) LastFront;

    // Swap buffers
    if (s_v->swapOverride)
    {
        // Swap override and copy valid on requested frame only
        s_v->swapOverride = false;
    }
    else
    {
        uint8_t *const B = s_v->frontbuffer;
        s_v->frontbuffer = s_v->backbuffer;
        s_v->backbuffer = B;
    }

    s_v->lastFramePeriod = BOARD_TicksNow() - s_v->frameStartTicks;
    s_v->frameStartTicks = BOARD_TicksNow();

    if (s_v->iface->FrameTransition)
    {
        s_v->iface->FrameTransition (s_v);
    }
    else 
    {
        ++ s_v->frameNumber;
    }

    if (s_v->copyFrame)
    {
        memcpy (s_v->backbuffer, LastBack, s_v->width * s_v->height);
        s_v->copyFrame = false;
    }

    if (s_v->iface->FrameBegin)
    {
        s_v->iface->FrameBegin (s_v);
    }
}


uint32_t VIDEO_FrameNumber (void)
{
    BOARD_AssertState (s_v);
    return s_v->frameNumber;
}


void VIDEO_ClearBack (const uint8_t Color)
{
    BOARD_AssertState (s_v);
    clearBuffer (s_v->backbuffer, Color);
}


void VIDEO_ClearFront (const uint8_t Color)
{
    BOARD_AssertState (s_v);
    clearBuffer (s_v->frontbuffer, Color);
}


void VIDEO_Zap (const uint8_t Color)
{
    VIDEO_ClearFront (Color);
    VIDEO_ClearBack  (Color);
}


const struct VIDEO_FONT * VIDEO_Font (void)
{
    BOARD_AssertState (s_v);
    return &s_v->font;
}


const struct VIDEO_RGB332_Gradient * VIDEO_Gradient (void)
{
    BOARD_AssertState (s_v);
    return &s_v->gradient;
}


void VIDEO_GradientShift (const int8_t Delta)
{
    BOARD_AssertState (s_v);
    VIDEO_RGB332_GradientShift (&s_v->gradient, Delta);
}


void VIDEO_ClippingRect (const uint16_t X1, const uint16_t Y1,
                         const uint16_t X2, const uint16_t Y2)
{
    BOARD_AssertState  (s_v);
    BOARD_AssertParams (X1 <= X2 && X2 < s_v->width &&
                         Y1 <= Y2 && Y2 < s_v->height);
    
    s_v->clipX1     = X1;
    s_v->clipY1     = Y1;
    s_v->clipX2     = X2;
    s_v->clipY2     = Y2;
    s_v->clipWidth  = X2 - X1 + 1;
    s_v->clipHeight = Y2 - Y1 + 1;
}


uint16_t VIDEO_ClipWidth (void)
{
    BOARD_AssertState (s_v);
    return s_v->clipWidth;
}


uint16_t VIDEO_ClipHeight (void)
{
    BOARD_AssertState (s_v);
    return s_v->clipHeight;
}


uint16_t VIDEO_ClipX1 (void)
{
    BOARD_AssertState (s_v);
    return s_v->clipX1;
}


uint16_t VIDEO_ClipX2 (void)
{
    BOARD_AssertState (s_v);
    return s_v->clipX2;
}


uint16_t VIDEO_ClipY1 (void)
{
    BOARD_AssertState (s_v);
    return s_v->clipY1;
}


uint16_t VIDEO_ClipY2 (void)
{
    BOARD_AssertState (s_v);
    return s_v->clipY2;
}


bool VIDEO_ScreenPointClipped (const int32_t X, const int32_t Y)
{
    BOARD_AssertState (s_v);

    if (X < s_v->clipX1 || X > s_v->clipX2 ||
        Y < s_v->clipY1 || Y > s_v->clipY2)
    {
        return true;
    }

    return false;
}


int32_t VIDEO_ScreenToClipX (const int32_t X)
{
    BOARD_AssertState (s_v);
    return (X - s_v->clipX1);
}


int32_t VIDEO_ScreenToClipY (const int32_t Y)
{
    BOARD_AssertState (s_v);
    return (Y - s_v->clipY1);
}


int32_t VIDEO_ClipToScreenX (const int32_t Cx)
{
    BOARD_AssertState (s_v);
    return (Cx + s_v->clipX1);
}


int32_t VIDEO_ClipToScreenY (const int32_t Cy)
{
    BOARD_AssertState (s_v);
    return (Cy + s_v->clipY1);
}


bool VIDEO_ClipIsInside (const int32_t Cx, const int32_t Cy,
                         const uint16_t Width, const uint16_t Height)
{
    BOARD_AssertState (s_v);

    if (Cx <= -(int32_t)Width  || Cx >= s_v->clipWidth ||
        Cy <= -(int32_t)Height || Cy >= s_v->clipHeight)
    {
        return false;
    }

    return true;
}


bool VIDEO_Clip (const int32_t Cx, const int32_t Cy,
                 const uint16_t Width, const uint16_t Height,
                 struct VIDEO_Clip *c)
{
    c->cx           = Cx;
    c->cy           = Cy;
    c->xLeft        = 0;
    c->xRight       = Width;
    c->yTop         = 0;
    c->yBottom      = Height;
    c->flags        = 0;

    if (Cx < 0)
    {
        c->xLeft    = -Cx;
        c->cx       = 0;
        c->flags    |= VIDEO_CLIP_LEFT;
    }

    if (Cx > VIDEO_ClipWidth() - Width)
    {
        c->xRight   = VIDEO_ClipWidth() - Cx;
        c->flags    |= VIDEO_CLIP_RIGHT;
    }

    if (Cy < 0)
    {
        c->yTop     = -Cy;
        c->cy       = 0;
        c->flags    |= VIDEO_CLIP_TOP;
    }

    if (Cy > VIDEO_ClipHeight() - Height)
    {
        c->yBottom  = VIDEO_ClipHeight() - Cy;
        c->flags    |= VIDEO_CLIP_BOTTOM;
    }

    c->cWidth  = c->xRight - c->xLeft;
    c->cHeight = c->yBottom - c->yTop;

    return c->flags? true : false;
}


const char * VIDEO_Description (void)
{
    BOARD_AssertState (s_v && s_v->iface && s_v->iface->Description);
    return s_v->iface->Description;
}

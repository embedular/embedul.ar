/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO] video device interface.

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


inline static bool validIface (const struct VIDEO_IFACE *const Iface)
{
    // Required interface elements
    // Width and height must be multiple of 8
    return (Iface && 
            Iface->Width && !(Iface->Width & 0x7) &&
            Iface->Height && !(Iface->Height & 0x7) &&
            Iface->Description &&
            Iface->HardwareInit &&
            Iface->ReachedVBICount &&
            Iface->WaitForVBI);
}


static uint32_t framebufferOctets (struct VIDEO *const V)
{
    return (V->iface->Width * V->iface->Height);
}


void VIDEO_Init (struct VIDEO *const V, const struct VIDEO_IFACE *const Iface,
                 uint8_t *const FramebufferA, uint8_t *const FramebufferB)
{
    BOARD_AssertParams      (V && FramebufferA);
    BOARD_AssertInterface   (validIface(Iface));

    OBJECT_Clear (V);

    V->iface            = Iface;
    V->frameStartTicks  = TICKS_Now ();
    // Scanlines effect off
    V->scanlines        = 0;
    // Default raster lines op values
    V->showAnd          = VIDEO_SOP_DEFAULT_SHOW_AND;
    V->showOr           = VIDEO_SOP_DEFAULT_SHOW_OR;
    V->scanAnd          = VIDEO_SOP_DEFAULT_SCAN_AND;
    V->scanOr           = VIDEO_SOP_DEFAULT_SCAN_OR;
    // Full frame rate; wait for one vertical blanking interrupt
    V->waitVbiCount     = 1;
    // Initial front and back buffer
    V->frontbuffer      = FramebufferA;
    V->backbuffer       = FramebufferB? FramebufferB : FramebufferA;

    {
        LOG_AutoContext (V, LANG_INIT);

        memset (FramebufferA, 0, framebufferOctets(V));

        if (FramebufferB)
        {
            memset (FramebufferB, 0, framebufferOctets(V));
        }

        V->iface->HardwareInit (V);

        LOG_Items (2,
                        "framebuffer width" ,V->iface->Width,
                        "height"            ,V->iface->Height);

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
}


bool VIDEO_IsValid (struct VIDEO *const V)
{
    return (V && validIface(V->iface))? true : false;
}


uint8_t * VIDEO_Frontbuffer (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return V->frontbuffer;
}


uint8_t * VIDEO_Backbuffer (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return V->backbuffer;
}


uint8_t * VIDEO_BackbufferXY (struct VIDEO *const V, const int32_t X,
                              const int32_t Y)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    BOARD_AssertParams (X >= 0 && X < V->iface->Width &&
                        Y >= 0 && Y < V->iface->Height);

    return &V->backbuffer[Y * V->iface->Width + X];
}


uint16_t VIDEO_Width (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return V->iface->Width;
}


uint16_t VIDEO_Height (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return V->iface->Height;
}


uint32_t VIDEO_GetBufferOctets (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return framebufferOctets (V);
}


void VIDEO_SetWaitVBICount (struct VIDEO *const V, const uint32_t Count)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    V->waitVbiCount = Count;
}


void VIDEO_SetScanlines (struct VIDEO *const V, const uint8_t Scanlines)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    V->scanlines = Scanlines;
}


void VIDEO_SetScanlineOp (struct VIDEO *const V, const enum VIDEO_SOP Op,
                          const uint8_t Value)
{
    BOARD_AssertParams (VIDEO_IsValid(V));

    switch (Op)
    {
        case VIDEO_SOP_ShowAnd:
            V->showAnd = Value;
            break;

        case VIDEO_SOP_ShowOr:
            V->showOr = Value;
            break;

        case VIDEO_SOP_ScanAnd:
            V->scanAnd = Value;
            break;

        case VIDEO_SOP_ScanOr:
            V->scanOr = Value;
            break;
    }
}


void VIDEO_ResetScanlineOp (struct VIDEO *const V, const enum VIDEO_SOP Op)
{
    switch (Op)
    {
        case VIDEO_SOP_ShowAnd:
            VIDEO_SetScanlineOp (V, Op, VIDEO_SOP_DEFAULT_SHOW_AND);
            break;

        case VIDEO_SOP_ShowOr:
            VIDEO_SetScanlineOp (V, Op, VIDEO_SOP_DEFAULT_SHOW_OR);
            break;

        case VIDEO_SOP_ScanAnd:
            VIDEO_SetScanlineOp (V, Op, VIDEO_SOP_DEFAULT_SCAN_AND);
            break;

        case VIDEO_SOP_ScanOr:
            VIDEO_SetScanlineOp (V, Op, VIDEO_SOP_DEFAULT_SCAN_OR);
            break;
    }
}


void VIDEO_ResetAllScanlineOps (struct VIDEO *const V)
{
    VIDEO_ResetScanlineOp (V, VIDEO_SOP_ShowAnd);
    VIDEO_ResetScanlineOp (V, VIDEO_SOP_ShowOr);
    VIDEO_ResetScanlineOp (V, VIDEO_SOP_ScanAnd);
    VIDEO_ResetScanlineOp (V, VIDEO_SOP_ScanOr);
}


// Dont swap buffers (on current frame only)
void VIDEO_SwapOverride (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    V->swapOverride = true;
}


// Copy last backbuffer to the next backbuffer (on current frame only)
void VIDEO_CopyFrame (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    V->copyFrame = true;
}


bool VIDEO_ReachedVBICount (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return V->iface->ReachedVBICount (V);
}


void VIDEO_WaitForVBI (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    V->iface->WaitForVBI (V);
}


void VIDEO_NextFrame (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));

    // frameEnd/Transition/Begin are interface optional notifications
    if (V->iface->FrameEnd)
    {
        V->iface->FrameEnd (V);
    }

    V->lastFrameBusy = TICKS_Now() - V->frameStartTicks;

    // Required interface implementation
    V->iface->WaitForVBI (V);

    const uint8_t * LastFront = V->frontbuffer;
    const uint8_t * LastBack = V->backbuffer;
    (void) LastFront;

    // Swap buffers override
    if (V->swapOverride)
    {
        // Request valid on current frame only
        V->swapOverride = false;
    }
    else
    {
        // Swap buffers
        uint8_t *const B = V->frontbuffer;
        V->frontbuffer = V->backbuffer;
        V->backbuffer = B;
    }

    V->lastFramePeriod = TICKS_Now() - V->frameStartTicks;
    V->frameStartTicks = TICKS_Now();

    if (V->iface->FrameTransition)
    {
        V->iface->FrameTransition (V);
    }
    else 
    {
        ++ V->frameNumber;
    }

    if (V->copyFrame)
    {
        V->copyFrame = false;

        if (V->backbuffer != LastBack)
        {
            memcpy (V->backbuffer, LastBack, framebufferOctets(V));
        }
    }


    if (V->iface->FrameBegin)
    {
        V->iface->FrameBegin (V);
    }
}


uint32_t VIDEO_FrameNumber (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return V->frameNumber;
}


void VIDEO_Shutdown (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));

    if (V->iface->Shutdown)
    {
        V->iface->Shutdown (V);
    }

    V->iface = NULL;
}


const char * VIDEO_Description (struct VIDEO *const V)
{
    BOARD_AssertParams (VIDEO_IsValid(V));
    return V->iface->Description;
}

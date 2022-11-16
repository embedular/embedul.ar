/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] video devices manager.

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

#include "embedul.ar/source/core/manager/screen.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/manager/storage/cache.h"


static struct SCREEN * s_s = NULL;


const char *const s_ScreenRoleNames[SCREEN_Role__COUNT] =
{
    [SCREEN_Role_Primary]   = "primary",
    [SCREEN_Role_Secondary] = "secondary",
    [SCREEN_Role_Menu]      = "menu",
    [SCREEN_Role_Console]   = "console"
};


inline static struct SCREEN_Context * screenContext (const enum SCREEN_Role Role)
{
    return &s_s->context[Role];
}


inline static uint32_t driverBufferOctets (struct SCREEN_Context *const C)
{
    return (C->driver->iface->Width * C->driver->iface->Height);
}


static void clearDriverBuffer (struct SCREEN_Context *const C,
                               uint8_t *const Buffer, const uint8_t Color)
{
    uint8_t *p = Buffer + C->driver->iface->Width * C->clip.y1 + C->clip.x1;

    for (uint32_t i = 0; i < C->clip.height; ++i)
    {
        memset (p, Color, C->clip.width);
        p += C->clip.width;
    }
}


static void resetContext (struct SCREEN_Context *const C)
{
    const uint16_t W = C->driver->iface->Width;
    const uint16_t H = C->driver->iface->Height;

    // Default color gradient for drawing primitives
    RGB332_GradientCopy (&C->gradient, &RGB332_GRADIENT_DEFAULT);

    // Clipping rect to whole screen
    C->clip.x1      = 0;
    C->clip.y1      = 0;
    C->clip.x2      = W - 1;
    C->clip.y2      = H - 1;
    C->clip.width   = W;
    C->clip.height  = H;
}


inline static bool isAvailable (const enum SCREEN_Role Role)
{
    return (s_s->context[Role].driver)? true : false;
}


void SCREEN_Init (struct SCREEN *const S)
{
    BOARD_AssertState  (!s_s);
    BOARD_AssertParams (S);

    OBJECT_Clear (S);

    {
        LOG_AutoContext (S, LANG_INIT);

    #ifdef LIB_EMBEDULAR_HAS_VIDEO
        SCREEN_FONT_Init (&S->defaultFont, NULL);
    #endif

        s_s = S;
    }
}


const char * SCREEN_RoleName (const enum SCREEN_Role Role)
{
    BOARD_AssertParams (Role < SCREEN_Role__COUNT);

    return s_ScreenRoleNames[Role];
}


void SCREEN_RegisterDevice (const enum SCREEN_Role Role,
                            struct VIDEO *const Driver)
{
    BOARD_AssertParams (Role < SCREEN_Role__COUNT && VIDEO_IsValid(Driver));

    struct SCREEN_Context *const C = screenContext (Role);

    if (!C->driver)
    {
        // New device, not a redefinition
        ++ s_s->registeredDevices;
    }

    C->driver   = Driver;
    C->font     = &s_s->defaultFont;

    resetContext (C);
}


uint32_t SCREEN_RegisteredDevices (void)
{
    return s_s->registeredDevices;
}


bool SCREEN_IsAvailable (const enum SCREEN_Role Role)
{
    BOARD_AssertParams (Role < SCREEN_Role__COUNT);
    return isAvailable(Role);
}


const struct SCREEN_Context * SCREEN_GetContext (const enum SCREEN_Role Role)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));
    return screenContext(Role);
}


void SCREEN_ResetContext (const enum SCREEN_Role Role)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);

    resetContext (C);
}


uint16_t SCREEN_Width (const enum SCREEN_Role Role)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    return C->driver->iface->Width;
}


uint16_t SCREEN_Height (const enum SCREEN_Role Role)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    return C->driver->iface->Height;
}


uint32_t SCREEN_FrameNumber (const enum SCREEN_Role Role)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    return VIDEO_FrameNumber (C->driver);
}


void SCREEN_ClearBack (const enum SCREEN_Role Role, const uint8_t Color)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    clearDriverBuffer (C, C->driver->backbuffer, Color);
}


void SCREEN_ClearFront (const enum SCREEN_Role Role, const uint8_t Color)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    clearDriverBuffer (C, C->driver->frontbuffer, Color);
}


void SCREEN_Zap (const enum SCREEN_Role Role, const uint8_t Color)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    clearDriverBuffer (C, C->driver->frontbuffer, Color);
    clearDriverBuffer (C, C->driver->backbuffer, Color);
}


void SCREEN_BlitCachedElement (const enum SCREEN_Role Role,
                               const uint32_t CachedElement)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);

    C->driver->copyFrame = true;

    if (!STORAGE_ValidVolume (STORAGE_Role_LinearCache))
    {
        LOG_WarnDebug (s_s, LANG_NO_LINEAR_CACHE);

        // Yellow = no valid volume
        clearDriverBuffer (C, C->driver->backbuffer, 0xFC);
        return;
    }

    // No element to fulfill the request
    if (CachedElement >= STORAGE_CachedElementsCount())
    {
        LOG_WarnDebug (s_s, LANG_CACHED_ELEMENT_NOT_FOUND);
        LOG_Items (1, LANG_CACHED_ELEMENT, CachedElement);

        // Violet = invalid element
        clearDriverBuffer (C, C->driver->backbuffer, 0xE3);
        return;
    }

    // Using backbuffer as a sector buffer. Its entire contents should be
    // overwritten by the cached element.
    struct STORAGE_CACHE_ElementInfo info;

    if (STORAGE_CACHE_ElementInfo(&info, CachedElement,
                                        C->driver->backbuffer, 1)
        == RAWSTOR_Status_Result_Ok)
    {
        BOARD_AssertParams (info.octets == driverBufferOctets(C));

        if (STORAGE_CACHE_ElementData(&info, 0, 0, C->driver->backbuffer, 1)
            == RAWSTOR_Status_Result_Ok)
        {
            return;
        }
    }

    // Red = cache read error
    clearDriverBuffer (C, C->driver->backbuffer, 0xE0);
}


const struct SCREEN_FONT * SCREEN_GetFont (const enum SCREEN_Role Role)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    return C->font;
}


struct RGB332_Gradient * SCREEN_GetGradient (const enum SCREEN_Role Role)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);
    return &C->gradient;
}


void SCREEN_SetClippingRect (const enum SCREEN_Role Role,
                             const uint16_t X1, const uint16_t Y1,
                             const uint16_t X2, const uint16_t Y2)
{
    BOARD_AssertState (SCREEN_IsAvailable(Role));

    struct SCREEN_Context * const C = screenContext (Role);

    BOARD_AssertParams (X1 <= X2 && X2 < C->driver->iface->Width &&
                        Y1 <= Y2 && Y2 < C->driver->iface->Height);
    
    C->clip.x1      = X1;
    C->clip.y1      = Y1;
    C->clip.x2      = X2;
    C->clip.y2      = Y2;
    C->clip.width   = X2 - X1 + 1;
    C->clip.height  = Y2 - Y1 + 1;
}


bool SCREEN_Clip (const enum SCREEN_Role Role,
                  const int32_t Cx, const int32_t Cy,
                  const uint16_t Width, const uint16_t Height,
                  struct SCREEN_ClippedRegion *const Creg)
{
    BOARD_AssertState   (SCREEN_IsAvailable(Role));
    BOARD_AssertParams  (Creg);

    struct SCREEN_Context * const C = screenContext (Role);

    return SCREEN_Context__clip (C, Cx, Cy, Width, Height, Creg);
}


void SCREEN_Update (void)
{
#ifdef LIB_EMBEDULAR_HAS_VIDEO
    for (enum SCREEN_Role r = 0; r < SCREEN_Role__COUNT; ++r)
    {
        if (SCREEN_IsAvailable (r))
        {
            VIDEO_NextFrame (s_s->context[r].driver);
        }
    }
#endif
}


void SCREEN_Shutdown (void)
{
#ifdef LIB_EMBEDULAR_HAS_VIDEO
    for (enum SCREEN_Role r = 0; r < SCREEN_Role__COUNT; ++r)
    {
        if (SCREEN_IsAvailable (r))
        {
            VIDEO_Shutdown (s_s->context[r].driver);
        }
    }
#endif
}


bool SCREEN_Context__clip (const struct SCREEN_Context * const C,
                           const int32_t Cx, const int32_t Cy,
                           const uint16_t Width, const uint16_t Height,
                           struct SCREEN_ClippedRegion *const Creg)
{
    Creg->cx            = Cx;
    Creg->cy            = Cy;
    Creg->xLeft         = 0;
    Creg->xRight        = Width;
    Creg->yTop          = 0;
    Creg->yBottom       = Height;
    Creg->clipped       = 0;

    if (Cx < 0)
    {
        Creg->xLeft     = -Cx;
        Creg->cx        = 0;
        Creg->clipped   |= SCREEN_ClippedFlags_Left;
    }

    if (Cx > C->clip.width - Width)
    {
        Creg->xRight    = C->clip.width - Cx;
        Creg->clipped   |= SCREEN_ClippedFlags_Right;
    }

    if (Cy < 0)
    {
        Creg->yTop      = -Cy;
        Creg->cy        = 0;
        Creg->clipped   |= SCREEN_ClippedFlags_Top;
    }

    if (Cy > C->clip.height - Height)
    {
        Creg->yBottom   = C->clip.height - Cy;
        Creg->clipped   |= SCREEN_ClippedFlags_Bottom;
    }

    Creg->cWidth  = Creg->xRight - Creg->xLeft;
    Creg->cHeight = Creg->yBottom - Creg->yTop;

    return Creg->clipped? true : false;
}


bool SCREEN_Context__isPointOut (const struct SCREEN_Context * const C,
                                 const int32_t X, const int32_t Y)
{
    return (X < C->clip.x1 || X > C->clip.x2 ||
            Y < C->clip.y1 || Y > C->clip.y2)?
            true : false;
}


bool SCREEN_Context__isClipRectOut (const struct SCREEN_Context * const C,
                                    const int32_t Cx, const int32_t Cy,
                                    const uint16_t Width, const uint16_t Height)
{
    return (Cx <= -(int32_t)Width  || Cx >= C->clip.width ||
            Cy <= -(int32_t)Height || Cy >= C->clip.height)?
            true : false;
}


int32_t SCREEN_Context__toClipX (const struct SCREEN_Context * const C,
                                 const int32_t X)
{
    return (X - C->clip.x1);
}


int32_t SCREEN_Context__toClipY (const struct SCREEN_Context * const C,
                                 const int32_t Y)
{
    return (Y - C->clip.y1);
}


int32_t SCREEN_Context__fromClipX (const struct SCREEN_Context * const C,
                                   const int32_t Cx)
{
    return (Cx + C->clip.x1);
}


int32_t SCREEN_Context__fromClipY (const struct SCREEN_Context * const C,
                                   const int32_t Cy)
{
    return (Cy + C->clip.y1);
}


uint8_t * SCREEN_Context__backbufferXY (const struct SCREEN_Context * const C,
                                        const int32_t X, const int32_t Y)
{
    BOARD_AssertParams (X >= 0 && X < C->driver->iface->Width &&
                        Y >= 0 && Y < C->driver->iface->Height);

    return &C->driver->backbuffer[Y * C->driver->iface->Width + X];
}

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SCREEN MANAGER] font drawing for utf-8 encoded character strings.

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

#include "embedul.ar/source/core/manager/screen/font.h"
#include "embedul.ar/source/core/manager/screen/font_std.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/utf8.h"


static const uint8_t s_MissingGlyph[8] =
{
    0xFE, 0xC6, 0xAA, 0x92, 0xAA, 0xC6, 0xFE, 0x00  // An 'X' in a box
};


static const uint8_t * selectGlyph (const struct SCREEN_FONT *f,
                                    uint16_t codepoint)
{
    if (codepoint <= 0x007F && codepoint >= 0x0020)
    {
        return f->basicLatin? (* f->basicLatin)[codepoint - 0x0020] :
                              (* f->missing);
    }
    else if (codepoint <= 0x00FF && codepoint >= 0x00A0)
    {
        return f->latin1? (* f->latin1)[codepoint - 0x00A0] :
                          (* f->missing);
    }
    else if (codepoint <= 0x03C9 && codepoint >= 0x0390)
    {
        return f->greek? (* f->greek)[codepoint - 0x0390] :
                         (* f->missing);
    }
    else if (codepoint <= 0x257F && codepoint >= 0x2500)
    {
        return f->boxDrawing? (* f->boxDrawing)[codepoint - 0x2500] :
                              (* f->missing);
    }
    else if (codepoint <= 0x259F && codepoint >= 0x2580)
    {
        return f->blockElements? (* f->blockElements)[codepoint - 0x2580] :
                                 (* f->missing);
    }
    else if (codepoint <= 0x303F && codepoint >= 0x3000)
    {
        return f->cjkSymbols? (* f->cjkSymbols)[codepoint - 0x3000] :
                              (* f->missing);
    }
    else if (codepoint <= 0x309F && codepoint >= 0x3040)
    {
        return f->hiragana? (* f->hiragana)[codepoint - 0x3040] :
                            (* f->missing);
    }
    else if (codepoint <= 0x30FF && codepoint >= 0x30A0)
    {
        return f->katakana? (* f->katakana)[codepoint - 0x30A0] :
                            (* f->missing);
    }
    else if (f->assorted && f->assorted(codepoint))
    {
        const uint8_t (* glyph)[8] = f->assorted (codepoint);
        return glyph[0];
    }

    return (* f->missing);
}


void SCREEN_FONT_Init (struct SCREEN_FONT *const F, 
                       SCREEN_FONT_SetGlyphsFunc const SetGlyphs)
{
    BOARD_AssertParams (F);
    
    OBJECT_Clear (F);
    
    if (SetGlyphs)
    {
        SetGlyphs (F);
    }
    else
    {
        SCREEN_FONT_STD (F);
    }
    
    if (!F->missing)
    {
        F->missing = &s_MissingGlyph;
    }
}


// Function originally meant to draw a series of glyphs on a textline (equal
// v0 and v1 through every glyph on the line) and so assumes the following
// precalculated conditions:
//
// 1) 'f' is valid.
// 2) The glyph is fully or partially inside of the clipping region.
// 3) Correct vertical clipping values passed in 'v0' and 'v1'.
static void drawClippedGlyph (const struct SCREEN_Context *const C, 
                              int32_t x, int32_t y,
                              uint8_t v0, uint8_t v1, 
                              const RGB332_Select ColorSel,
                              const uint16_t Codepoint)
{
    uint8_t clipMask = 0xFF;
    
    int32_t cx = SCREEN_Context__toClipX (C, x);

    if (cx < 0)
    {
        clipMask >>= -cx;
    }
    else if (cx > C->clip.width - 8)
    {
        clipMask <<= cx - (C->clip.width - 8);
    }
    
    x = SCREEN_Context__fromClipX (C, cx);
    
    const uint8_t *const    Glyph   = selectGlyph (C->font, Codepoint);
    const uint16_t          Width   = C->driver->iface->Width;
    uint8_t *               bb      = SCREEN_Context__backbufferXY
                                                (C, (x >= 0)? x : 0, y);

    if (x < 0)
    {
        // clipMask start/end is adjusted according to cx, but x is not adjusted
        // to clipping limits since each glyph line is drawn by using clipMask
        // alone as the clipping points. Nevertheless, bb[start] must still
        // point to "illegal" negative x framebuffer offsets to account for
        // clipMask left clipped pixels. Negative x offsets allows to
        // correctly position the clipped glyph on screen, and clipMask
        // guarantees that negative x offsets will not be written to.
        bb += x;
    }

    // Glyphs are drawn MSB to LSB, top to bottom
    if (RGB332_SelectIsGradientAuto (ColorSel))
    {
        while (v0 <= v1)
        {
            const uint8_t Color     = C->gradient.c[v0];
            const uint8_t GlyphLine = Glyph[v0] & clipMask;
            if (GlyphLine & 0x80) bb[0] = Color;
            if (GlyphLine & 0x40) bb[1] = Color;
            if (GlyphLine & 0x20) bb[2] = Color;
            if (GlyphLine & 0x10) bb[3] = Color;
            if (GlyphLine & 0x08) bb[4] = Color;
            if (GlyphLine & 0x04) bb[5] = Color;
            if (GlyphLine & 0x02) bb[6] = Color;
            if (GlyphLine & 0x01) bb[7] = Color;
            ++ v0;
            bb += Width;
        }            
    }
    else 
    {
        const uint8_t Color = RGB332_GetSelectedColor (&C->gradient, 
                                                            ColorSel, 0);

        while (v0 <= v1)
        {
            const uint8_t GlyphLine = Glyph[v0] & clipMask;
            if (GlyphLine & 0x80) bb[0] = Color;
            if (GlyphLine & 0x40) bb[1] = Color;
            if (GlyphLine & 0x20) bb[2] = Color;
            if (GlyphLine & 0x10) bb[3] = Color;
            if (GlyphLine & 0x08) bb[4] = Color;
            if (GlyphLine & 0x04) bb[5] = Color;
            if (GlyphLine & 0x02) bb[6] = Color;
            if (GlyphLine & 0x01) bb[7] = Color;
            ++ v0;
            bb += Width;
        }
    }
}


// Performs vertical clipping calculations for drawSemiclippedGlyph()
// The glyph must be fully or partially inside of the clipping region.
static void glyphClipV (const struct SCREEN_Context *const C, 
                        int32_t *const Cy, uint8_t *const V0, uint8_t *const V1)
{
    *V0 = (*Cy < 0)? (uint8_t) -(*Cy) : 0;
    *V1 = (*Cy > C->clip.height - 8)?
                        (uint8_t) (7 - (*Cy - (C->clip.height - 8)))
                            : 7;

    BOARD_AssertParams (*V0 <= *V1 && *V1 <= 7);

    if (*V0)
    {
        *Cy = 0;
    }
}


bool SCREEN_FONT_DrawGlyph (const enum SCREEN_Role Role,
                            const int32_t X, const int32_t Y,
                            const RGB332_Select ColorSel,
                            const uint16_t Codepoint)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    int32_t cx = SCREEN_Context__toClipX (C, X);
    int32_t cy = SCREEN_Context__toClipY (C, Y);

    if (SCREEN_Context__isClipRectOut (C, cx, cy, 8, 8))
    {
        return false;
    }

    uint8_t v0, v1;
    glyphClipV (C, &cy, &v0, &v1);

    const int32_t Sx = SCREEN_Context__fromClipX (C, cx);
    const int32_t Sy = SCREEN_Context__fromClipY (C, cy);

    drawClippedGlyph (C, Sx, Sy, v0, v1, ColorSel, Codepoint);

    return true;
}


uint32_t SCREEN_FONT_DrawStringSegment (const enum SCREEN_Role Role,
                                        const int32_t X, const int32_t Y, 
                                        const RGB332_Select ColorSel,
                                        const char *const Str,
                                        const uint32_t Octets)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (Role);

    BOARD_AssertParams (Str);

    int32_t cx = SCREEN_Context__toClipX (C, X);
    int32_t cy = SCREEN_Context__toClipY (C, Y);

    if (cy <= -8 || cy >= C->clip.height)
    {
        // Line is outside of the clipping region
        return 0;
    }

    uint32_t        glyphsDrawn = 0;
    const char      * sp        = Str;
    uint32_t        octetsLeft  = Octets;
    uint32_t        skipGlyphs  = (cx < 0)? (uint32_t) (-cx >> 3) : 0;

    uint8_t v0, v1;
    glyphClipV (C, &cy, &v0, &v1);
    
    int32_t x = SCREEN_Context__fromClipX (C, cx);
    int32_t y = SCREEN_Context__fromClipY (C, cy);
    
    while (octetsLeft)
    {
        struct UTF8_GetCodePointResult r =
                        UTF8_GetCodePoint ((const uint8_t *)sp, octetsLeft);

        if (r.dataLength)
        {
            sp += r.dataLength;
            // Octets can't be negative
            BOARD_AssertState (octetsLeft >= r.dataLength);
            octetsLeft -= r.dataLength;
        }
        // Invalid byte, draw a replacement char
        else
        {
            r.codepoint = SCREEN_FONT_REPLACEMENT_CHAR_CODEPOINT;
            sp += 1;
            octetsLeft -= 1;
        }

        if (skipGlyphs)
        {
            -- skipGlyphs;
        }
        else if (x <= C->clip.x2)
        {
            drawClippedGlyph (C, x, y, v0, v1, ColorSel, r.codepoint);
            ++ glyphsDrawn;
        }

        x += 8;
    }
    
    return glyphsDrawn;
}


static int32_t setDpAlignH (const enum SCREEN_FONT_DP Dp, const int32_t X,
                            const char *currentLine, const size_t LineOctets)
{
    const uint32_t GlyphsToDraw =
                        UTF8_Count ((uint8_t *)currentLine, LineOctets);

    int32_t alignedX = X;

    if (Dp & SCREEN_FONT_DP_AlingHCenter)
    {
        // Line centered at X.
        alignedX -= (GlyphsToDraw >> 1) << 3;
        // Odd number, adjust centering by half a glyph.
        if (GlyphsToDraw & 0x01)
        {
            alignedX -= 4;
        }
    }
    else if (Dp & SCREEN_FONT_DP_AlignHLeft)
    {
        // Line ends at X.
        alignedX -= GlyphsToDraw << 3;
    }

    // SCREEN_FONT_DP_AlignHRight: Each line starts at X. Default alignment since X
    // increments to the right. Nothing to adjust.

    return alignedX;
}


uint32_t SCREEN_FONT_DrawString (const enum SCREEN_Role Role,
                                 const int32_t X, const int32_t Y,
                                 const RGB332_Select ColorSel,
                                 const enum SCREEN_FONT_DP Dp,
                                 const int8_t RowHeightAdjust,
                                 const size_t MaxOctets,
                                 const char *const Str)
{
    BOARD_AssertParams (Str);
    
    uint32_t    glyphsDrawn = 0;
    const char  * lineBegin = Str;
    const char  * sp        = Str;
    uint32_t    lineOctets  = 0;
    uint32_t    maxOctets   = MaxOctets;
    int32_t     yCurrent    = Y;
    
    while (maxOctets --)
    {
        if (*sp == '\n' || *sp == '\0')
        {
            const int32_t XAligned = 
                            (Dp & SCREEN_FONT_DP_AlignMask)?
                                setDpAlignH(Dp, X, lineBegin, lineOctets) : X;

            if (Dp & SCREEN_FONT_DP_Shadow)
            {
                // Glyphs drawn as shadows are not taken into account. 
                SCREEN_FONT_DrawStringSegment (Role, XAligned+1, yCurrent+1,
                                               0x00, lineBegin, lineOctets);
            }

            glyphsDrawn += SCREEN_FONT_DrawStringSegment 
                                        (Role, XAligned, yCurrent, ColorSel,
                                         lineBegin, lineOctets);
            if (*sp == '\0')
            {
                break;
            }
            
            lineBegin   = sp + 1;
            lineOctets  = 0;
            yCurrent   += 8 + RowHeightAdjust;
        }
        else 
        {
            ++ lineOctets;
        }
        
        sp += 1;
    }
    
    return glyphsDrawn;
}


uint32_t SCREEN_FONT_AutoMaxOctets (const enum SCREEN_Role Role)
{
    // Maximum number of octets required to fill an entire line of non-clipped
    // glyphs using the longest encoding supported of three octets each.
    return (SCREEN_Width(Role) >> 3) * 3;
}


uint32_t SCREEN_FONT_DrawStringLite (const enum SCREEN_Role Role,
                                     const int32_t X, const int32_t Y,
                                     const RGB332_Select ColorSel, 
                                     const char *const Str)
{
    return SCREEN_FONT_DrawString (Role, X, Y, ColorSel, 
                                   SCREEN_FONT_DP_None, 0,
                                   SCREEN_FONT_AutoMaxOctets(Role), Str);    
}


struct StringArgsData
{
    const enum SCREEN_Role      Role;
    const int32_t               StartX;
    int32_t                     currentX;
    int32_t                     currentY;
    const RGB332_Select         ColorSel;
    const enum SCREEN_FONT_DP   Dp;
    uint32_t                    totalGlyphsDrawn;
    const int8_t                RowHeightAdjust;
};


static void stringArgsDrawSegment (struct StringArgsData *const D,
                                   const char *const Str,
                                   const uint32_t Octets)
{
    if (D->Dp & SCREEN_FONT_DP_Shadow)
    {
        // Glyphs drawn as shadow are not taken into account. 
        SCREEN_FONT_DrawStringSegment (D->Role, D->currentX+1, D->currentY+1,
                                       0x00, Str, Octets);
    }

    const uint32_t GlyphsDrawn =
            SCREEN_FONT_DrawStringSegment (D->Role, D->currentX, D->currentY,
                                           D->ColorSel, Str, Octets);

    D->currentX += 8 * GlyphsDrawn;
    D->totalGlyphsDrawn += GlyphsDrawn;
}


static void stringArgsOutProc (void *const Param, const uint8_t *const Data,
                               const uint32_t Count)
{
    struct StringArgsData *const D = (struct StringArgsData *) Param;

    const uint8_t * segment = Data;
    uint32_t        begin   = 0;

    for (uint32_t i = 0; i < Count; ++i)
    {
        if (Data[i] == '\n')
        {
            stringArgsDrawSegment (D, (char *)segment, Count - begin);

            D->currentY += 8 + D->RowHeightAdjust;
            D->currentX  = D->StartX;

            segment = &Data[i + 1];
            begin   = i + 1;
        }
    }

    if (begin < Count)
    {
        stringArgsDrawSegment (D, (char *)segment, Count - begin);
    }
}


uint32_t SCREEN_FONT_DrawParsedStringArgs (const enum SCREEN_Role Role,
                                           const int32_t X, const int32_t Y,
                                           const RGB332_Select ColorSel, 
                                           const enum SCREEN_FONT_DP Dp,
                                           const int8_t RowHeightAdjust,
                                           const size_t MaxOctets,
                                           const char *const Str,
                                           struct VARIANT *const ArgValues,
                                           const uint32_t ArgCount)
{
    // There is no way to know the string length without buffering all
    // chunks and then drawing the entire line.
    BOARD_AssertParams (!(Dp & SCREEN_FONT_DP_AlignMask));

    struct StringArgsData d =
    {
        .Role               = Role,
        .StartX             = X,
        .currentX           = X,
        .currentY           = Y,
        .ColorSel           = ColorSel,
        .Dp                 = Dp,
        .totalGlyphsDrawn   = 0,
        .RowHeightAdjust    = RowHeightAdjust
    };

    VARIANT_ParseStringArgs (0, MaxOctets, Str, stringArgsOutProc, (void *)&d,
                             ArgValues, ArgCount);

    return d.totalGlyphsDrawn;
}


uint32_t SCREEN_FONT_DrawParsedStringArgsLite (const enum SCREEN_Role Role,
                                               const int32_t X, const int32_t Y,
                                               const RGB332_Select ColorSel,
                                               const char *const Str, 
                                               struct VARIANT *const ArgValues,
                                               const uint32_t ArgCount)
{
    return SCREEN_FONT_DrawParsedStringArgs (Role, X, Y, ColorSel, 
                                             SCREEN_FONT_DP_None, 0,
                                             SCREEN_FONT_AutoMaxOctets(Role),
                                             Str, ArgValues, ArgCount);
}

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] nyan-cat splash screen theme as seen on youtube.
  Nyan Cat character copyright prguitarman, daniwell, saraj00n
  https://www.youtube.com/watch?v=Ywv-9CQ-DfY

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

#include "embedul.ar/source/core/sequence.h"
#include "embedul.ar/source/core/utf8.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/manager/screen/rect.h"
#include "embedul.ar/source/core/manager/screen/tile.h"
#include "embedul.ar/source/core/manager/screen/tilemap.h"
#include "embedul.ar/assets/splash_themes/nyancat/source/TILEMAPS_nyancat.h"
#include <stddef.h>


struct NYANCAT_Data
{
    TIMER_Ticks     lastElapsed;
    uint32_t        frame;
    const char      *const Msg;
    const uint32_t  msgGlyphs;
};


#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_HeadOffsetX[12] = 
{
    17, 18, 18, 18, 17, 17, 17, 18, 18, 18, 17, 17
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_HeadOffsetY[12] = 
{
    5, 5, 6, 6, 6, 5, 5, 5, 6, 6, 6, 5
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_BodyOffsetX[12] =
{
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_BodyOffsetY[12] =
{
    0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_TailOffsetX[12] =
{
    1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_TailOffsetY[12] =
{
    7, 8, 11, 11, 9, 8, 7, 8, 11, 11, 9, 8 
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg0OffsetX[12] =
{
    5, 6, 7, 6, 4, 4, 5, 6, 7, 6, 4, 4
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg0OffsetY[12] =
{
    16, 17, 18, 18, 16, 16, 16, 16, 17, 17, 16, 16
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg1OffsetX[12] =
{
    11, 11, 12, 11, 9, 9, 11, 11, 12, 11, 9, 9
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg1OffsetY[12] =
{
    18, 18, 19, 19, 19, 19, 18, 18, 19, 19, 19, 19
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg2OffsetX[12] =
{
    20, 21, 22, 21, 19, 19, 20, 21, 22, 21, 19, 19
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg2OffsetY[12] =
{
    18, 18, 19, 19, 19, 19, 18, 18, 19, 19, 19, 19
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg3OffsetX[12] =
{
    25, 26, 27, 26, 24, 24, 25, 26, 27, 26, 24, 24
};

#ifdef RODATA_SECTION_RETROCIAA_SPLASH_THEME
RODATA_SECTION_RETROCIAA_SPLASH_THEME
#endif
static const uint8_t s_Leg3OffsetY[12] =
{
    18, 18, 19, 19, 19, 18, 18, 18, 19, 19, 19, 18
};


static void nyanHead (const int32_t X, const int32_t Y)
{
    SCREEN_TILEMAP_DrawLite (SCREEN_SPLASH_THEME_ROLE, &TILEMAP_nyancat, X, Y,
                             TILEMAP_nyancat_head_INDEX, 0);
}


static void nyanBody (const int32_t X, const int32_t Y)
{
    SCREEN_TILEMAP_DrawLite (SCREEN_SPLASH_THEME_ROLE, &TILEMAP_nyancat, X, Y,
                             TILEMAP_nyancat_body_INDEX, 0);
}


static void nyanTail (const int32_t X, const int32_t Y, const uint32_t Frame)
{
    SCREEN_TILEMAP_DrawLite (&TILEMAP_nyancat, X, Y,
                             TILEMAP_nyancat_tail_INDEX, Frame);
}


static void nyanLeg (const int32_t X, const int32_t Y, const uint16_t Leg,
                         const uint32_t Frame)
{
    SCREEN_TILEMAP_DrawLite (&TILEMAP_nyancat, X, Y,
                             TILEMAP_nyancat_leg0_INDEX + Leg, Frame);
}


static void rainbow (const int32_t X, const int32_t Y, uint32_t frame)
{
    int32_t x0 = X;
    int32_t y0 = Y;
    int32_t x1 = X;
    int32_t y1 = Y;

    if ((frame % 4) > 1)
    {
        x0 += 1;
        x1 += 1;
        y0 += -1;
        y1 += 1;
    }

    for (int32_t lr = 0; (x0+8)-(lr) > 0; lr += 16)
    {
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE,
                          x0-lr, y0+2, 8, 3, 0xe0);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE, 
                          x0-lr, y0+5, 8, 3, 0xf4);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE, 
                          x0-lr, y0+8, 8, 3, 0xfc);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE, 
                          x0-lr, y0+11, 8, 3, 0x5c);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE, 
                          x0-lr, y0+14, 8, 3, 0x57);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE, 
                          x0-lr, y0+17, 8, 3, 0x6b);

        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE,
                          x1-(lr+8), y1+1, 8, 3, 0xe0);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE,
                          x1-(lr+8), y1+4, 8, 3, 0xf4);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE,
                          x1-(lr+8), y1+7, 8, 3, 0xfc);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE,
                          x1-(lr+8), y1+10, 8, 3, 0x5c);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE,
                          x1-(lr+8), y1+13, 8, 3, 0x57);
        SCREEN_RECT_Draw (SCREEN_SPLASH_THEME_ROLE,
                          x1-(lr+8), y1+16, 8, 3, 0x6b);
    }
}


static void star (const int32_t X, const int32_t Y, uint32_t img)
{
    img = img % 6;
    
    SCREEN_TILEMAP_DrawLite (SCREEN_SPLASH_THEME_ROLE, &TILEMAP_nyancat, X, Y,
                             TILEMAP_nyancat_star_INDEX, img);
}


static void draw (const int32_t X, const int32_t Y, const uint32_t Frame)
{
    const uint32_t F12 = Frame % 12;
    const uint32_t FX  = SCREEN_FrameNumber(SCREEN_SPLASH_THEME_ROLE) << 2;

    rainbow (X, Y, Frame);

    star ((4-FX) & 0xFF, 4, Frame + 1);
    star ((90-FX) & 0xFF, 24, Frame + 3);
    star ((140-FX) & 0xFF, 40, Frame + 1);
    star ((240-FX) & 0xFF, 80, Frame + 4);
    star ((90-FX) & 0xFF, 110, Frame + 1);

    nyanBody (X + s_BodyOffsetX[F12], Y + s_BodyOffsetY[F12]);
    nyanTail (X + s_TailOffsetX[F12], Y + s_TailOffsetY[F12], F12);
    nyanLeg  (X + s_Leg0OffsetX[F12], Y + s_Leg0OffsetY[F12], 0, F12);
    nyanLeg  (X + s_Leg1OffsetX[F12], Y + s_Leg1OffsetY[F12], 1, F12);
    nyanLeg  (X + s_Leg2OffsetX[F12], Y + s_Leg2OffsetY[F12], 2, F12);
    nyanLeg  (X + s_Leg3OffsetX[F12], Y + s_Leg3OffsetY[F12], 3, F12);
    nyanHead (X + s_HeadOffsetX[F12], Y + s_HeadOffsetY[F12]);
}


static void title (struct SEQUENCE *sequence, void *param,
                   const enum SEQUENCE_Action Action,
                   const TIMER_Ticks Elapsed)
{
    (void) sequence;
    (void) param;

    switch (Action)
    {
        case SEQUENCE_Action_Entry:
            LOG (NOBJ, "entering nyan cat splash screen");
            SOUND_PlayCachedBgm (1, 2);
            break;

        case SEQUENCE_Action_Run:
            SCREEN_ClearBack (SCREEN_SPLASH_THEME_ROLE, 0x00);

            if (Elapsed > 400)
            {
                SCREEN_FONT_DrawStringLite (SCREEN_SPLASH_THEME_ROLE, 
                                            6*8, 6*8, 0xFF,
                                            "RETRO-CIAA FRAMEWORK");
            }

            if (Elapsed > 600)
            {
                SCREEN_FONT_DrawString (SCREEN_SPLASH_THEME_ROLE, 128, 7*8,
                                        0x0A, SCREEN_FONT_DP_AlingHCenter,
                                        0, 96, CC_VcsFwkVersionStr);
            }

            if (Elapsed > 2200)
            {
                SCREEN_FONT_DrawStringLite (SCREEN_SPLASH_THEME_ROLE, 
                                            12*8, 11*8, 0xFF, "Presents");
            }

            break;

        case SEQUENCE_Action_Exit:
            break;
    }
}


static void nyancat (struct SEQUENCE *sequence, void *param,
                     const enum SEQUENCE_Action Action,
                     const TIMER_Ticks Elapsed)
{
    (void) sequence;

    struct NYANCAT_Data * data = (struct NYANCAT_Data *) param;

    switch (Action)
    {
        case SEQUENCE_Action_Entry:
        {
            data->lastElapsed = 0;
            data->frame = 0;
            break;
        }

        case SEQUENCE_Action_Run:
        {
            SCREEN_ClearBack (SCREEN_SPLASH_THEME_ROLE, 0x05);

            if (Elapsed - data->lastElapsed >= 60)
            {
                data->lastElapsed = Elapsed;
                ++ data->frame;

                RGB332_GradientShift (SCREEN_Gradient(SCREEN_SPLASH_THEME_ROLE),
                                      1);
            }

            // Nyan cat centered on screen
            draw (128-(32>>1), 72-(21>>1), data->frame);

            SCREEN_FONT_DrawString (SCREEN_SPLASH_THEME_ROLE, 128, 8,
                                    RGB332_SelectGradientAuto,
                                    SCREEN_FONT_DP_Shadow | 
                                    SCREEN_FONT_DP_AlingHCenter,
                                    0, 96, CC_AppNameStr);

            SCREEN_FONT_DrawString (SCREEN_SPLASH_THEME_ROLE, 128, 16, 0x0A,
                                    SCREEN_FONT_DP_Shadow |
                                    SCREEN_FONT_DP_AlingHCenter,
                                    0, 96, BOARD_Description());

            SCREEN_FONT_DrawString (SCREEN_SPLASH_THEME_ROLE, 128, 24, 0x0A,
                                    SCREEN_FONT_DP_Shadow |
                                    SCREEN_FONT_DP_AlingHCenter,
                                    0, 96, CC_VcsAppVersionStr);

            if (SCREEN_FrameNumber(SCREEN_SPLASH_THEME_ROLE) & 0x20)
            {
                SCREEN_FONT_DrawString (SCREEN_SPLASH_THEME_ROLE, 11*8, 15*8,
                                        0xFF, SCREEN_FONT_DP_Shadow, 0, 96,
                                        "PRESS START");
            }

            // Nyan Cat scrolling copyright info
            SCREEN_FONT_DrawString (SCREEN_SPLASH_THEME_ROLE,
                             -((Elapsed >> 5) % (data->msgGlyphs * 8)),
                             136, 0x0A, SCREEN_FONT_DP_None, 0,
                             data->msgGlyphs * 3, data->Msg);
            SCREEN_FONT_DrawString (SCREEN_SPLASH_THEME_ROLE,
                             -((Elapsed >> 5) % (data->msgGlyphs * 8)) +
                             data->msgGlyphs * 8,
                             136, 0x0A, SCREEN_FONT_DP_None, 0,
                             data->msgGlyphs * 3, data->Msg);
            break;
        }

        case SEQUENCE_Action_Exit:
        {
            LOG (NOBJ, "leaving nyan cat splash screen");
            break;
        }
    }
}


static const struct SEQUENCE_Stage s_SplashTheme[] =
{
    {
        .func       = title,
        .timeout    = 3700
    },
    {
        .func       = nyancat,
        .input      = SEQUENCE_INPUT_ANY_BIT(INPUT_PROFILE_SelectFlag__BUTTONS)
    }
};


void SPLASH_THEME_nyan_cat (void)
{
    struct NYANCAT_Data data =
    {
        .Msg        = "Nyan Cat © prguitarman, daniwell, saraj00n" 
                      " ★ https://youtu.be/QH2-TGUlwu4 ★ ",
        .msgGlyphs  = UTF8_Count ((const uint8_t *const)data.Msg,
                                                            strlen(data.Msg))
    };

    struct SEQUENCE seq;

    SEQUENCE_Init (&seq, s_SplashTheme, 
                   sizeof(s_SplashTheme) / sizeof(s_SplashTheme[0]), &data);

    while (SEQUENCE_Update (&seq))
    {
        BOARD_Sync ();
    }
}

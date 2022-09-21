/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] c64 splash screen theme.

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

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/sequence.h"
#include "embedul.ar/source/core/device/video/font.h"
#include "embedul.ar/source/core/device/video/rect.h"


static void c64 (struct SEQUENCE *const Sequence, void *const Param,
                 const enum SEQUENCE_Action Action,
                 const TIMER_Ticks Elapsed)
{
    (void) Sequence;
    (void) Param;
    (void) Elapsed;

    switch (Action)
    {
        case SEQUENCE_Action_Entry:
            LOG (NOBJ, "entering c64 splash screen");

            VIDEO_ClearBack (0x46);

            for (int16_t y = 0; y < VIDEO_Height(); y += 8)
            {
                VIDEO_FONT_DrawGlyphLite (0, y, 0x93, 0x2588);
                VIDEO_FONT_DrawGlyphLite (VIDEO_Width()-8, y, 0x93, 0x2588);
            }

            for (int16_t x = 0; x < VIDEO_Width(); x += 8)
            {
                VIDEO_FONT_DrawGlyphLite (x, 0, 0x93, 0x2588);
                VIDEO_FONT_DrawGlyphLite (x, VIDEO_Height()-8, 0x93, 0x2588);
            }

            VIDEO_FONT_DrawString (VIDEO_Font(), VIDEO_Width() / 2, 2*8,
                                   0x93, VIDEO_FONT_DP_AlingHCenter, 0,
                                   VIDEO_FONT_AutoMaxOctets(),
                                   "** EMBEDUL.AR FRAMEWORK **");

            VIDEO_FONT_DrawStringLite (1*8, 5*8, 0x93, "READY.");
            VIDEO_FONT_DrawParsedStringLite (1*8, 6*8, 0x93,
                                       "LOAD \"`0\",8,1", CC_AppNameStr);
            VIDEO_FONT_DrawParsedStringLite (1*8, 7*8, 0x93,
                                       CC_VcsAppVersionStr);
            VIDEO_FONT_DrawStringLite (1*8, 9*8, 0x93, "READY.");

            VIDEO_CopyFrame ();
            break;

        case SEQUENCE_Action_Run:
            VIDEO_FONT_DrawGlyphLite (1*8, 10*8, (VIDEO_FrameNumber() & 0x20)?
                                        0x46 : 0x93, 0x2588);
            break;

        case SEQUENCE_Action_Exit:
            LOG (NOBJ, "leaving c64 splash screen");
            break;
    }
}


static const struct SEQUENCE_Stage s_SplashTheme[] =
{
    {
        .func       = c64,
        .timeout    = 3000
    }
};


void SPLASH_THEME_c64 (void)
{
    struct SEQUENCE seq;

    SEQUENCE_Init (&seq, s_SplashTheme, 
                   sizeof(s_SplashTheme) / sizeof(s_SplashTheme[0]), NULL);

    while (SEQUENCE_Update (&seq))
    {
        BOARD_Update ();
    }
}

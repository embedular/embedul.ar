/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [VIDEO subsystem] glyph test splash screen theme.

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


struct LOCK_GLYPH_Data
{
    uint16_t    current;
    uint16_t    end;
    uint16_t    last;
};


// Outputs an entirely white screen to aid in monitor signal lock when
// running on a software-generated signal on dual-core targets.
// It has no practical use on simulated targets (SDL).
static void monitorSignalLock (struct SEQUENCE *sequence, void *param,
                               const enum SEQUENCE_Action Action,
                               const TIMER_Ticks Elapsed)
{
    (void) sequence;
    (void) param;
    (void) Elapsed;

    switch (Action)
    {
        case SEQUENCE_Action_Entry:
            LOG (NOBJ, "entering monitor signal lock");
            VIDEO_Zap (0xFF);
            break;

        case SEQUENCE_Action_Run:
            break;

        case SEQUENCE_Action_Exit:
            LOG (NOBJ, "leaving monitor signal lock");
            break;
    }
}


static void fillScreenWithCodepoint (uint16_t codepoint)
{
    for (int16_t y = 0; y < VIDEO_Height(); y += 8)
    {
        for (int16_t x = 0; x < VIDEO_Width(); x += 8)
        {
            VIDEO_FONT_DrawGlyph (VIDEO_Font(), x, y, VIDEO_RGB332_SelectColor(0xFF), 
                            codepoint);
        }
    }
}


static void glyphStressTest (struct SEQUENCE *sequence, void *param,
                             const enum SEQUENCE_Action Action,
                             const TIMER_Ticks Elapsed)
{
    (void) sequence;
    (void) param;
    (void) Elapsed;

    struct LOCK_GLYPH_Data * data = (struct LOCK_GLYPH_Data *) param;

    switch (Action)
    {
        case SEQUENCE_Action_Entry:
            LOG (NOBJ, "entering glyph stress test");
            VIDEO_ClearBack (0x00);
            break;

        case SEQUENCE_Action_Run:
            if (VIDEO_FrameNumber() % 2 == 0)
            {
                VIDEO_ClearBack (0x00);

                if (data->current < data->end)
                {
                    fillScreenWithCodepoint (data->current);
                    ++ data->current;
                }
                else
                {
                    fillScreenWithCodepoint (data->last);
                    SEQUENCE_ExitStage (sequence);
                }

                VIDEO_CopyFrame ();
            }
            break;

        case SEQUENCE_Action_Exit:
            LOG (NOBJ, "leaving glyph stress test");
            break;
    }
}


static const struct SEQUENCE_Stage s_VideoSignalInit[] =
{
    {
        .func       = monitorSignalLock,
        .timeout    = 5000
    },
    {
        .func       = glyphStressTest,
    },
    {
        .timeout    = 400
    }
};


void SPLASH_THEME_lock_glyph (void)
{
    // Repeating codepoint
    struct LOCK_GLYPH_Data data =
    {
        .current    = 0x2580,
        .end        = 0x259F,
        .last       = 0x25A1
    };

    struct SEQUENCE seq;

    SEQUENCE_Init (&seq, s_VideoSignalInit, 
                   sizeof(s_VideoSignalInit) / sizeof(s_VideoSignalInit[0]),
                   &data);

    while (SEQUENCE_Update (&seq))
    {
        BOARD_Update ();
    }
}

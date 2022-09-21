/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  CVT 0.92M9-R 1280x720 59.74 hz display modeline.

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

/*
================================================================================
Modeline determined by the cvt command
================================================================================
$ cvt -r -v 1280 720 60
# 1280x720 59.74 Hz (CVT 0.92M9-R) hsync: 44.27 kHz; pclk: 63.75 MHz
Modeline "1280x720R"   63.75  1280 1328 1360 1440  720 723 728 741 +hsync -vsync
================================================================================
                       HSYNC
                    +---------+
               FP   |         |    FP    v        Visible Pixels
Line       *--------+         +----------|---------------------------------*
Pixels         48       32         80                 1280
nS           752.94   501.96    1254.90             20078.43   (Total 22.58 uS)
================================================================================
                          VSYNC
                      +-*-*-*...*-+
                FP    |           |    BP     v     Visible Lines
           *-*-*-...*-*           *-*-*-*-*...|-*-*-*-*-*-*-*-*-*...*-*-*-*-
Lines            3          5          13               720
uS             67.76     112.94      293.64          16263.53  (Total 16.73 mS)
================================================================================
*/

// Active resolution/FPS/Signal standard
#define MODELINE_SIGNAL_STR     "1280x720 59.74 Hz (CVT 0.92M9-R)"

#define MODELINE_PCLK_KHZ       63750

#define MODELINE_HDISP          1280   // HSYNCSTART-HDISP = Front Porch
#define MODELINE_HSYNCSTART     1328   // HSYNCEND-HSYNCSTART = HSYNC pulse
#define MODELINE_HSYNCEND       1360   // HTOTAL-HSYNCEND = Back Porch
#define MODELINE_HTOTAL         1440

#define MODELINE_VDISP          720
#define MODELINE_VSYNCSTART     723
#define MODELINE_VSYNCEND       728
#define MODELINE_VTOTAL         741

#define MODELINE_HSYNC_ON       1      // +HSYNC
#define MODELINE_VSYNC_ON       0      // -VSYNC

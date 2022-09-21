/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  Generic EDID 1280x720 60 hz display modeline.

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
Common HDTV screen modeline (according to their EDID)
================================================================================
74.25 1280 1390 1420 1650 720 725 730 750 +hsync +vsync
================================================================================
                       HSYNC
                    +---------+
               FP   |         |    BP    v        Visible Pixels
Line       *--------+         +----------|---------------------------------*
Pixels         110       30       230                 1280
ns           1481.48    404     3097.64              17239    (Total 22.22 us)
================================================================================
                          VSYNC
                      +-*-*-*...*-+
                FP    |           |    BP     v     Visible Lines
           *-*-*-...*-*           *-*-*-*-*...|-*-*-*-*-*-*-*-*-*...*-*-*-*-
Lines            5          5          20               720
us            111,11     111,11      444,44            16000   (Total 16.66 ms)
================================================================================
*/

// Active resolution, Frames per second, Signal standard
#define MODELINE_SIGNAL_STR     "1280x720 60 Hz (EDID)"

#define MODELINE_PCLK_KHZ       74250

#define MODELINE_HDISP          1280   // HSYNCSTART-HDISP = Front Porch
#define MODELINE_HSYNCSTART     1390   // HSYNCEND-HSYNCSTART = HSYNC pulse
#define MODELINE_HSYNCEND       1420   // HTOTAL-HSYNCEND = Back Porch
#define MODELINE_HTOTAL         1650

#define MODELINE_VDISP          720
#define MODELINE_VSYNCSTART     725
#define MODELINE_VSYNCEND       730
#define MODELINE_VTOTAL         750

#define MODELINE_HSYNC_ON       1      // +HSYNC
#define MODELINE_VSYNC_ON       1      // +VSYNC

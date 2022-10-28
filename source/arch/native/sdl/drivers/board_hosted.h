/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD driver] SDL hosted environment target.

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

#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/drivers/random_sfmt.h"
#include "embedul.ar/source/arch/native/sdl/drivers/io_keyboard.h"
#include "embedul.ar/source/arch/native/sdl/drivers/io_gui.h"
#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332_adapter_sim.h"
#include "embedul.ar/source/arch/native/sdl/drivers/video_rgb332_vgafb.h"
#include "embedul.ar/source/arch/native/sdl/drivers/sound_sdlmixer.h"
#include "embedul.ar/source/arch/native/sdl/drivers/stream_file.h"
#include "embedul.ar/source/arch/native/sdl/drivers/rawstor_file.h"


struct BOARD_IO_PROFILES
{
    struct INPUT_PROFILE_CONTROL    inControl;
    struct INPUT_PROFILE_GP1        inGp1;
    struct INPUT_PROFILE_GP2        inGp2;
    struct INPUT_PROFILE_LIGHTDEV   inLightdev;
    struct INPUT_PROFILE_MAIN       inMain;
    struct OUTPUT_PROFILE_CONTROL   outControl;
    struct OUTPUT_PROFILE_LIGHTDEV  outLightdev;
    struct OUTPUT_PROFILE_MARQUEE   outMarquee;
    struct OUTPUT_PROFILE_SIGN      outSign;
};


struct BOARD_HOSTED
{
    struct BOARD                    device;
    struct RANDOM_SFMT              randomSfmt;
    struct VIDEO_RGB332_ADAPTER_SIM videoAdapterSim;
    struct VIDEO_RGB332_VGAFB       videoVgafbMenu;
    //struct VIDEO_RGB332_VGAFB       videoVgafbConsole;
    struct SOUND_SDLMIXER           soundSdlmixer;
    struct IO_KEYBOARD              ioKeyboard;
    struct IO_GUI                   ioGui;
    struct STREAM_FILE              streamDebugFile;
    struct RAWSTOR_FILE             rsImageFile;
    uint32_t                        screenToWindowId[SCREEN_Role__COUNT];
};

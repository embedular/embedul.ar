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
#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/io_keyboard_sdl.h"
#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/video_sdl.h"
#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/sound_sdl.h"
#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/stream_file.h"
#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/rawstor_file.h"


struct BOARD_HOSTED_SDL
{
    struct BOARD                device;
    struct RANDOM_SFMT          randomSfmt;
    struct VIDEO_SDL            videoSdl;
    struct SOUND_SDL            soundSdl;
    struct IO_KEYBOARD_SDL      ioKbSdl;
    struct STREAM_FILE          streamDebugFile;
    struct RAWSTOR_FILE         rsImageFile;
};


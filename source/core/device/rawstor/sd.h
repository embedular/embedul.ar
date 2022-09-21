/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR] common sdcard protocol states and media types.

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

#include <stdint.h>


// MMC/SDC specific ioctl command (compatible with FatFs)
#define RAWSTOR_SD_IOCTL_GET_TYPE       	    10
#define RAWSTOR_SD_IOCTL_GET_CSD        	    11
#define RAWSTOR_SD_IOCTL_GET_CID        	    12
#define RAWSTOR_SD_IOCTL_GET_OCR        	    13
#define RAWSTOR_SD_IOCTL_GET_SDSTAT     	    14

// Card type flags (queried by RAWSTOR_SD_IOCTL_GET_TYPE)
#define RAWSTOR_SD_TYPE_UNKNOWN                 0x00
#define RAWSTOR_SD_TYPE_MMC_V3                  0x01
#define RAWSTOR_SD_TYPE_SD_V1                   0x02
#define RAWSTOR_SD_TYPE_SD_V2                   0x04
#define RAWSTOR_SD_TYPE_SD_ANY                  (RAWSTOR_SD_TYPE_SD_V1 | \
                                                 RAWSTOR_SD_TYPE_SD_V2)
#define RAWSTOR_SD_TYPE_BLOCK_ADDRESSING        0x08

typedef uint8_t RAWSTOR_SD_TYPE;

#define RAWSTOR_SD_STATUS_MEDIA_INITIALIZING__P1_POWER_ON       0
#define RAWSTOR_SD_STATUS_MEDIA_INITIALIZING__P1_NATIVE_MODE    1
#define RAWSTOR_SD_STATUS_MEDIA_INITIALIZING__P1_IDLE           2

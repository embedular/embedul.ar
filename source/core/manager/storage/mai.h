/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STORAGE subsystem] fatfs-compatible media access interface.

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

#include "embedul.ar/source/core/manager/storage.h"


RAWSTOR_Status_Disk     STORAGE_MAI_InitVolume  (const enum STORAGE_Role Role);
struct RAWSTOR_Status   STORAGE_MAI_VolumeStats (const enum STORAGE_Role Role);
RAWSTOR_Status_Result   STORAGE_MAI_ReadVolume  (const enum STORAGE_Role Role,
                                                 uint8_t *data, uint32_t sector,
                                                 uint32_t count);
RAWSTOR_Status_Result   STORAGE_MAI_WriteVolume (const enum STORAGE_Role Role,
                                                 const uint8_t *data,
                                                 uint32_t sector,
                                                 uint32_t count);
RAWSTOR_Status_Result   STORAGE_MAI_VolumeIoctl (const enum STORAGE_Role Role,
                                                 uint8_t cmd, void *data);

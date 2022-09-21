/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SOUND] sound device interface (singleton).

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

#include "embedul.ar/source/core/timer.h"
#include "embedul.ar/source/core/device/sound/mixer.h"
#include "embedul.ar/source/core/device/sound/bgm.h"
#include "embedul.ar/source/core/device/rawstor.h"


#define SOUND_MIXER_BUFFERS     8
#define SOUND_SFX_SLOTS         8


struct SOUND;


typedef void (* SOUND_HardwareInitFunc)(struct SOUND *const S);
typedef void (* SOUND_MuteFunc)(struct SOUND *const S, const bool Enable);
typedef void (* SOUND_BufferFilledFunc)(struct SOUND *const S);
typedef void (* SOUND_BufferConsumedFunc)(struct SOUND *const S);
typedef bool (* SOUND_MixBegin)(struct SOUND *const S);
typedef void (* SOUND_MixEnd)(struct SOUND *const S);


struct SOUND_IFACE
{
    const char                      * const Description;
    const SOUND_HardwareInitFunc    HardwareInit;
    const SOUND_MuteFunc            Mute;
    const SOUND_BufferFilledFunc    BufferFilled;
    const SOUND_BufferConsumedFunc  BufferConsumed;
    const SOUND_MixBegin            MixBegin;
    const SOUND_MixEnd              MixEnd;
};


struct SOUND_BGM_Cached
{
    uint32_t                        sectorBegin;
    uint32_t                        sectorCount;
    uint32_t                        sectorCurrent;
    uint32_t                        repeat;
};


typedef bool (* SOUND_ProcBGMFunc)(struct SOUND *const S, uint8_t *const Buffer,
                                   void *const ProcData);


struct SOUND
{
    const struct SOUND_IFACE        * iface;
    uint8_t                         buffer[SOUND_MIXER_BUFFERS]
                                            [SOUND_MIXER_BUFFER_SIZE];
    bool                            muted;
    bool                            mixOverride;
    struct SOUND_BGM_Cached         bgmCached;
    enum SOUND_BGM_Status           bgmStatus;
    SOUND_ProcBGMFunc               bgmProc;
    void                            * bgmProcData;
    struct SOUND_FX_Slot            sfxSlots[SOUND_SFX_SLOTS];
    uint32_t                        bufferFreeIndex;
    volatile uint32_t               bufferFreeCount;
    volatile uint32_t               bufferFilledIndex;
    volatile uint32_t               bufferFilledAvailable;
    uint32_t                        bufferClears;
    uint32_t                        bufferFills;
    uint32_t                        bufferConsumptions;
    TIMER_Ticks                     bufferBgmReadTicks;
    uint32_t                        bufferBgmReadErrors;
};


void        SOUND_Init              (struct SOUND *const S,
                                     const struct SOUND_IFACE *const Iface);
void        SOUND_Mute              (const bool Enable);
bool        SOUND_MixOneBuffer      (void);
void        SOUND_Mix               (void);
void        SOUND_MixOverride       (void);
uint8_t *   SOUND_GetBuffer         (void);
void        SOUND_BufferFilled      (void);
void        SOUND_BufferConsumed    (void);
void        SOUND_PlayCachedBgm     (const uint32_t CachedElement,
                                     const uint32_t Repeat);
void        SOUND_SetProcBGM        (SOUND_ProcBGMFunc const Proc,
                                     void *const ProcData);
void        SOUND_StopBGM           (void);
void        SOUND_PauseBGM          (void);
void        SOUND_ResumeBGM         (void);
void        SOUND_ReserveSFX        (const SOUND_FX_SlotIndex Index);
SOUND_FX_SlotIndex
            SOUND_PlaySFX           (const struct SOUND_FX *const Sfx,
                                     const SOUND_FX_SlotIndex Index);
void        SOUND_SetSFXVolume      (const SOUND_FX_SlotIndex Index,
                                     const uint8_t VolLeft,
                                     const uint8_t VolRight);
bool        SOUND_StopSFXBySlot     (const SOUND_FX_SlotIndex Index);
void        SOUND_StopSFXBySample   (const struct SOUND_FX *const Sfx);
void        SOUND_StopSFX           (void);
const char *
            SOUND_Description       (void);

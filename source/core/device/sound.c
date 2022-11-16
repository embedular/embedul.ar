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

#include "embedul.ar/source/core/device/sound.h"
#include "embedul.ar/source/core/device/rawstor.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/manager/storage/cache.h"


static struct SOUND * s_a = NULL;


void SOUND_Init (struct SOUND *const S, const struct SOUND_IFACE *const Iface)
{
    BOARD_AssertState  (!s_a);
    BOARD_AssertParams (S && Iface);

    // Required interface elements
    BOARD_AssertInterface (Iface->Description);

    OBJECT_Clear (S);

    S->iface            = Iface;
    S->bufferFreeCount  = SOUND_MIXER_BUFFERS;

    s_a = S;

    LOG_ContextBegin (S, LANG_INIT);
    {
        LOG_Items (4,   
                    LANG_SOUND_MIXER_SAMPLE_RATE, (uint32_t)SOUND_MIXER_SAMPLE_RATE,
                    LANG_BITS,              (uint32_t)SOUND_MIXER_BITS,
                    LANG_CHANNELS,          (uint32_t)SOUND_MIXER_CHANNELS,
                    LANG_BUFFER_SIZE,       (uint32_t)SOUND_MIXER_BUFFER_SIZE);

        if (s_a->iface->HardwareInit)
        {
            s_a->iface->HardwareInit (s_a);
        }
    }
    LOG_ContextEnd ();
}


void SOUND_Mute (const bool Enable)
{
    BOARD_AssertState (s_a);

    if (s_a->iface->Mute)
    {
        s_a->iface->Mute (s_a, Enable);
    }
    
    s_a->muted = Enable;
}


bool SOUND_MixOneBuffer (void)
{
    BOARD_AssertState (s_a);

    uint8_t *buffer = SOUND_GetBuffer ();

    if (!buffer)
    {
        return false;
    }

    // BGM
    if (!s_a->bgmProc || !s_a->bgmProc(s_a, buffer, s_a->bgmProcData))
    {
        // No BGM available, clear buffer
        memset (buffer, 0, SOUND_MIXER_BUFFER_SIZE);
        ++ s_a->bufferClears;
    }

    // SFX mixing over BGM or blank buffer
    for (uint32_t i = 0; i < SOUND_SFX_SLOTS; ++i)
    {
        struct SOUND_FX_Slot *slot = &s_a->sfxSlots[i];
        if (slot->status == SOUND_FX_SlotStatus_Idle)
        {
            continue;
        }

        BOARD_AssertState (slot->sfx);

        SOUND_MIXER_FXSlotToBuffer (slot, buffer);

        if (slot->offset >= slot->sfx->size)
        {
            slot->offset = 0;
            slot->status = SOUND_FX_SlotStatus_Idle;
        }
    }

    SOUND_BufferFilled ();

    return true;
}


void SOUND_Mix (void)
{
    BOARD_AssertState (s_a);

    if (s_a->iface->MixBegin)
    {
        if (!s_a->iface->MixBegin (s_a))
        {
            return;
        }
    }

    if (s_a->mixOverride)
    {
        // Mixer override is valid on the current frame only
        s_a->mixOverride = false;
    }

    while (SOUND_MixOneBuffer ());

    /*
    if (s_a->bufferFreeAvailable)
    {
        // Process only available buffers at this time
        for (uint32_t i = s_a->bufferFreeAvailable; i > 0; --i)
        {
            if (!SOUND_MixOneBuffer ())
            {
                break;
            }
        }
    }
    */

    if (s_a->iface->MixEnd)
    {
        s_a->iface->MixEnd (s_a);
    }
}


void SOUND_MixOverride (void)
{
    BOARD_AssertState (s_a);

    s_a->mixOverride = true;
}


uint8_t * SOUND_GetBuffer (void)
{
    BOARD_AssertState (s_a);
\
    if (!s_a->bufferFreeCount)
    {
        return NULL;
    }
 
    return s_a->buffer[s_a->bufferFreeIndex];
}


void SOUND_BufferFilled (void)
{
    BOARD_AssertState (s_a);
        
    // A call to this function means a buffer was filled.
    // Then, at least a buffer must have been available.
    BOARD_AssertState (s_a->bufferFreeCount);
    
    -- s_a->bufferFreeCount;
    ++ s_a->bufferFilledAvailable;

    if (++ s_a->bufferFreeIndex >= SOUND_MIXER_BUFFERS)
    {
        s_a->bufferFreeIndex = 0;
    }

    ++ s_a->bufferFills;

    if (s_a->iface->BufferFilled)
    {
        s_a->iface->BufferFilled (s_a);
    }
}


void SOUND_BufferConsumed (void)
{
    BOARD_AssertState (s_a);

    -- s_a->bufferFilledAvailable;
    ++ s_a->bufferFreeCount;

    if (++ s_a->bufferFilledIndex >= SOUND_MIXER_BUFFERS)
    {
        s_a->bufferFilledIndex = 0;
    }

    ++ s_a->bufferConsumptions;

    if (s_a->iface->BufferConsumed)
    {
        s_a->iface->BufferConsumed (s_a);
    }
}


// Cached BGMs have always the same format as the mixer and are streamed
// directly to the mixer buffers.
static bool bgmCachedToBuffer (struct SOUND *const S, uint8_t *const Buffer,
                               void *const ProcData)
{
    (void) ProcData;

    BOARD_AssertParams (S && Buffer);

    if (s_a->bgmStatus == SOUND_BGM_Status_Stopped ||
        s_a->bgmStatus == SOUND_BGM_Status_Paused)
    {
        return false;
    }

    s_a->bufferBgmReadTicks = TICKS_Now ();

    RAWSTOR_Status_Result r =
        STORAGE_LinearRead (STORAGE_Role_LinearCache, Buffer,
                            s_a->bgmCached.sectorCurrent, 1, 1);

    if (r != RAWSTOR_Status_Result_Ok)
    {
        ++ s_a->bufferBgmReadErrors;

        LOG_WarnDebug (S, LANG_BGM_CACHE_READ_FAILED);
        LOG_Items (1, LANG_ERROR, r);

        if (r != RAWSTOR_Status_Result_ReadWriteError)
        {
            LOG_Debug (S, LANG_BGM_PLAYBACK_STOP_NOW);
            SOUND_StopBGM ();

            return false;
        }

        // In the event of a ReadWriteError status, BGM playback will continue
        // with undefined data in this sound buffer (At 32 Khz this is hardly
        // noticeable).
    }

    if (++ s_a->bgmCached.sectorCurrent >= s_a->bgmCached.sectorCount)
    {
        s_a->bgmCached.sectorCurrent = s_a->bgmCached.sectorBegin;
        LOG (S, LANG_BGM_REWIND);

        if (s_a->bgmCached.repeat)
        {
            LOG (S, LANG_BGM_REPEAT);
            if (s_a->bgmCached.repeat != SOUND_BGM_REPEAT_FOREVER)
            {
                -- s_a->bgmCached.repeat;
            }
        }
        else
        {
            LOG (S, LANG_BGM_STOP);
            SOUND_StopBGM ();
        }
    }

    ++ s_a->bufferFills;

    return true;
}


void SOUND_PlayCachedBgm (const uint32_t CachedElement, const uint32_t Repeat)
{
    BOARD_AssertState (s_a && STORAGE_ValidVolume(STORAGE_Role_LinearCache));

    SOUND_StopBGM ();

    uint8_t sectorData[512];

    struct STORAGE_CACHE_ElementInfo info;
    
    if (STORAGE_CACHE_ElementInfo(&info, CachedElement, sectorData, 1)
        == RAWSTOR_Status_Result_Ok)
    {
        SOUND_SetProcBGM (bgmCachedToBuffer, NULL);

        s_a->bgmCached.sectorBegin      = info.sectorBegin;
        s_a->bgmCached.sectorCount      = info.sectorCount;
        s_a->bgmCached.sectorCurrent    = info.sectorBegin;
        s_a->bgmCached.repeat           = Repeat;

        s_a->bgmStatus = SOUND_BGM_Status_Playing;
    }
    else
    {
        LOG_WarnDebug (s_a, LANG_BGM_CACHE_READ_FAILED);
        LOG_Items (1, LANG_CACHED_ELEMENT, CachedElement);
    }
}


void SOUND_SetProcBGM (SOUND_ProcBGMFunc const Proc, void *const ProcData)
{
    BOARD_AssertState (s_a);

    SOUND_StopBGM ();

    s_a->bgmProc      = Proc;
    s_a->bgmProcData  = ProcData;
}


void SOUND_StopBGM (void)
{
    BOARD_AssertState (s_a);
    s_a->bgmStatus = SOUND_BGM_Status_Stopped;
}


void SOUND_PauseBGM (void)
{
    BOARD_AssertState (s_a);
    s_a->bgmStatus = SOUND_BGM_Status_Paused;
}


void SOUND_ResumeBGM (void)
{
    BOARD_AssertState (s_a);
    s_a->bgmStatus = SOUND_BGM_Status_Playing;
}


void SOUND_ReserveSFX (const SOUND_FX_SlotIndex Index)
{
    BOARD_AssertState  (s_a);
    BOARD_AssertParams (Index < SOUND_SFX_SLOTS);

    s_a->sfxSlots[Index].flags |= SFX_SlotFlag_Reserved;
}


SOUND_FX_SlotIndex SOUND_PlaySFX (const struct SOUND_FX *const Sfx,
                             const SOUND_FX_SlotIndex Index)
{
    BOARD_AssertState  (s_a);
    BOARD_AssertParams (Sfx && SOUND_MIXER_ValidateFXFormat(Sfx->format));
    BOARD_AssertParams (Index < SOUND_SFX_SLOTS ||
                        Index == SOUND_FX_SlotIndexAny ||
                        Index == SOUND_FX_SlotIndexNoOverlap);

    struct SOUND_FX_Slot * slot      = NULL;
    SOUND_FX_SlotIndex   retIndex    = Index;
    
    if (Index < SOUND_SFX_SLOTS)
    {
        slot = &s_a->sfxSlots[Index];

        // If the requested slot index is reserved and currently playing, it
        // can only be used to playback the same sfx from the beginning.
        if ((slot->flags & SFX_SlotFlag_Reserved)
            && slot->status == SOUND_FX_SlotStatus_Playing
            && slot->sfx != Sfx)
        {
            return SOUND_FX_SlotIndexNone;
        }
    }
    else
    {
        if (Index == SOUND_FX_SlotIndexNoOverlap)
        {
            // If the requested sfx is now being played then search for that
            // corresponding slot (do not overlap the currently playing sound
            // with another instance)
            for (SOUND_FX_SlotIndex i = 0; i < SOUND_SFX_SLOTS; ++i)
            {
                struct SOUND_FX_Slot *s = &s_a->sfxSlots[i];
                if (s->sfx == Sfx)
                {
                    slot = s;
                    retIndex = i;
                    break;
                }
            }
        }

        // sfx is not overlapping or index == SOUND_FX_SlotIndexAny
        if (!slot || Index == SOUND_FX_SlotIndexAny)
        {
            // Search for any non-reserved idle slot
            for (SOUND_FX_SlotIndex i = 0; i < SOUND_SFX_SLOTS; ++i)
            {
                struct SOUND_FX_Slot *s = &s_a->sfxSlots[i];
                if (!(s->flags & SFX_SlotFlag_Reserved)
                    && s->status == SOUND_FX_SlotStatus_Idle)
                {
                    slot = s;
                    retIndex = i;
                    break;
                }
            }
        }
    }

    // No slot found
    if (!slot)
    {   
        return SOUND_FX_SlotIndexNone;
    }

    // Playing back "sfx" from the beginning and at maximum volume by default.
    slot->sfx       = Sfx;
    slot->offset    = 0;
    slot->status    = SOUND_FX_SlotStatus_Playing;
    slot->volLeft   = 255;
    slot->volRight  = 255;

    return retIndex;
}


void SOUND_SetSFXVolume (const SOUND_FX_SlotIndex Index, const uint8_t VolLeft,
                         const uint8_t VolRight)
{
    BOARD_AssertState  (s_a);
    BOARD_AssertParams (Index < SOUND_SFX_SLOTS);

    s_a->sfxSlots[Index].volLeft  = VolLeft;
    s_a->sfxSlots[Index].volRight = VolRight;
}


static void freeSfxSlot (struct SOUND_FX_Slot *const Slot)
{
    Slot->offset = 0;
    Slot->status = SOUND_FX_SlotStatus_Idle;
}


bool SOUND_StopSFXBySlot (const SOUND_FX_SlotIndex Index)
{
    BOARD_AssertState  (s_a);
    BOARD_AssertParams (Index < SOUND_SFX_SLOTS);


    if (Index >= SOUND_SFX_SLOTS)
    {
        return false;
    }

    freeSfxSlot (&s_a->sfxSlots[Index]);

    return true;
}


void SOUND_StopSFXBySample (const struct SOUND_FX *const Sfx)
{
    BOARD_AssertState  (s_a);
    BOARD_AssertParams (Sfx);

    for (SOUND_FX_SlotIndex i = 0; i < SOUND_SFX_SLOTS; ++i)
    {
        if (s_a->sfxSlots[i].sfx == Sfx)
        {
            freeSfxSlot (&s_a->sfxSlots[i]);
            break;
        }
    }
}


void SOUND_StopSFX (void)
{
    BOARD_AssertState (s_a);

    for (SOUND_FX_SlotIndex i = 0; i < SOUND_SFX_SLOTS; ++i)
    {
        freeSfxSlot (&s_a->sfxSlots[i]);
    }
}


const char * SOUND_Description (void)
{
    BOARD_AssertState (s_a && s_a->iface && s_a->iface->Description);
    return s_a->iface->Description;
}

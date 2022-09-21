/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SOUND subsystem] sound mixer.

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

#include "embedul.ar/source/core/device/sound/mixer.h"
#include "embedul.ar/source/core/device/board.h"


// These sample rate conversion functions assume a mixer buffer of 16 bits
// in stereo
#if (SOUND_MIXER_BITS != 16)
    #error Mixer bits expected to be 16
#endif

#if (SOUND_MIXER_CHANNELS != 2)
    #error Mixer channels expected to be 2
#endif

#if (SOUND_MIXER_BUFFER_SIZE != RAWSTOR_SECTOR_SIZE)
    #error AUDIO_BUFFER_SIZE must be equal to a rawstor SECTOR (512 B)
#endif


static inline int16_t ssat16 (const int16_t A, const int16_t B)
{
    const int32_t X = A + B;
    return (int16_t)((X < INT16_MIN)? INT16_MIN
                                    : (X > INT16_MAX)? INT16_MAX : X);
}


static void mix8BitMono (void *const Buffer, struct SOUND_FX_Slot *const Slot)
{
    const int8_t *const DS8 = Slot->sfx->ds8 + Slot->offset;
    // Amount of samples not yet played: 1 byte for each sample.
    const uint32_t SSL = (Slot->sfx->size - Slot->offset);
    // Mixer iteration count: 2 samples of 2 bytes on each iteration.
    const uint32_t HRC = ((SOUND_MIXER_BUFFER_SIZE >> 2) < SSL)?
                                (SOUND_MIXER_BUFFER_SIZE >> 2) : SSL;
    // 256 to 128 volume levels per channel
    const uint8_t VL = Slot->volLeft >> 1;
    const uint8_t VR = Slot->volRight >> 1;

    int16_t *mixer16 = Buffer;
    for (uint32_t i = 0; i < HRC; ++i)
    {
        const int8_t S8I = DS8[i];
        const int16_t L16 = S8I * VL;
        const int16_t R16 = S8I * VR;

        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
    }

    // Slot bytes added: 1 sample of 1 byte for each mixer iteration.
    Slot->offset += HRC;
}


static void mix8BitMonoHSR (void *const Buffer, struct SOUND_FX_Slot *const Slot)
{
    const int8_t *const DS8 = Slot->sfx->ds8 + Slot->offset;
    // Amount of samples not yet played: 1 byte for each sample.
    const uint32_t SSL = (Slot->sfx->size - Slot->offset);
    // Mixer iteration count: 4 samples of 2 bytes on each iteration.
    const uint32_t HRC = ((SOUND_MIXER_BUFFER_SIZE >> 3) < SSL)?
                                (SOUND_MIXER_BUFFER_SIZE >> 3) : SSL;
    // 256 to 128 volume levels per channel
    const uint8_t VL = Slot->volLeft >> 1;
    const uint8_t VR = Slot->volRight >> 1;

    int16_t *mixer16 = Buffer;
    for (uint32_t i = 0; i < HRC; ++i)
    {
        const int8_t S8I = DS8[i];
        const int16_t L16 = S8I * VL;
        const int16_t R16 = S8I * VR;

        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
    }

    // Slot bytes added: 1 sample of 1 byte for each mixer iteration.
    Slot->offset += HRC;
}


static void mix8BitMonoQSR (void *const Buffer, struct SOUND_FX_Slot *const Slot)
{
    const int8_t *const DS8 = Slot->sfx->ds8 + Slot->offset;
    // Amount of samples not yet played: 1 byte for each sample.
    const uint32_t SSL = (Slot->sfx->size - Slot->offset);
    // Mixer iteration count: 8 samples of 2 bytes on each iteration.
    const uint32_t HRC = ((SOUND_MIXER_BUFFER_SIZE >> 4) < SSL)?
                                (SOUND_MIXER_BUFFER_SIZE >> 4) : SSL;
    // 256 to 128 volume levels per channel
    const uint8_t VL = Slot->volLeft >> 1;
    const uint8_t VR = Slot->volRight >> 1;

    int16_t *mixer16 = Buffer;
    for (uint32_t i = 0; i < HRC; ++i)
    {
        const int8_t S8I = DS8[i];
        const int16_t L16 = S8I * VL;
        const int16_t R16 = S8I * VR;

        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
    }

    // Slot bytes added: 1 sample of 1 byte for each mixer iteration.
    Slot->offset += HRC;
}


static void mix16BitStereo (void *const Buffer, struct SOUND_FX_Slot *const Slot)
{
    const uint16_t *const DU1 = Slot->sfx->du16 + (Slot->offset >> 1);
    // Amount of samples not yet played: 2 bytes for each sample.
    const uint32_t  SSL = (Slot->sfx->size - Slot->offset) >> 1;
    // Mixer iteration count: 2 samples of 2 bytes on each iteration.
    const uint32_t  HRC = ((SOUND_MIXER_BUFFER_SIZE >> 2) < SSL >> 1)?
                                (SOUND_MIXER_BUFFER_SIZE >> 2) : SSL >> 1;

    int16_t *mixer16 = Buffer;
    for (uint32_t i = 0; i < HRC; i += 2)
    {
        // TODO: 16 bit channel volume control
        const int16_t L16 = (int16_t)DU1[i + 0];
        const int16_t R16 = (int16_t)DU1[i + 1];

        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
    }

    // Slot bytes added: 2 samples of 2 bytes for each mixer iteration.
    Slot->offset += HRC << 2;
}


static void mix16BitMonoHSR (void *const Buffer, struct SOUND_FX_Slot *const Slot)
{
    const uint16_t *const DU1 = Slot->sfx->du16 + (Slot->offset >> 1);
    // Amount of samples not yet played: 2 bytes for each sample.
    const uint32_t SSL  = (Slot->sfx->size - Slot->offset) >> 1;
    // Mixer iteration count: 4 samples of 2 bytes on each iteration.
    const uint32_t HRC  = ((SOUND_MIXER_BUFFER_SIZE >> 3) < SSL)?
                                (SOUND_MIXER_BUFFER_SIZE >> 3) : SSL;

    int16_t *mixer16 = Buffer;
    for (uint32_t i = 0; i < HRC; ++i)
    {
        const uint16_t U16I = DU1[i];
        // TODO: 16 bit channel volume control
        const int16_t L16 = (int16_t)U16I;
        const int16_t R16 = (int16_t)U16I;

        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
    }

    // Slot bytes added: 1 sample of 2 bytes for each mixer iteration.
    Slot->offset += HRC << 1;
}


static void mix16BitMonoQSR (void *const Buffer, struct SOUND_FX_Slot *const Slot)
{
    const uint16_t *const DU1 = Slot->sfx->du16 + (Slot->offset >> 1);
    // Amount of samples not yet played: 2 bytes for each sample.
    const uint32_t SSL  = (Slot->sfx->size - Slot->offset) >> 1;
    // Mixer iteration count: 8 samples of 2 bytes on each iteration.
    const uint32_t HRC  = ((SOUND_MIXER_BUFFER_SIZE >> 4) < SSL)?
                                (SOUND_MIXER_BUFFER_SIZE >> 4) : SSL;

    int16_t *mixer16 = Buffer;
    for (uint32_t i = 0; i < HRC; ++i)
    {
        const uint16_t U16I = DU1[i];
        // TODO: 16 bit channel volume control
        const int16_t L16 = (int16_t)U16I;
        const int16_t R16 = (int16_t)U16I;

        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, L16); ++ mixer16;
        *mixer16 = ssat16 (*mixer16, R16); ++ mixer16;
    }

    // Slot bytes added: 1 sample of 2 bytes for each mixer iteration.
    Slot->offset += HRC << 1;
}



bool SOUND_MIXER_ValidateFXFormat (const uint32_t Format)
{
    switch (Format)
    {
        case SOUND_FX_FORMAT_8BIT_MONO:
        case SOUND_FX_FORMAT_8BIT_MONO_HSR:
        case SOUND_FX_FORMAT_8BIT_MONO_QSR:
        case SOUND_FX_FORMAT_16BIT_STEREO:
        case SOUND_FX_FORMAT_16BIT_MONO_HSR:
        case SOUND_FX_FORMAT_16BIT_MONO_QSR:
            break;

        default:
            return false;
    }

    return true;
}


bool SOUND_MIXER_FXSlotToBuffer (struct SOUND_FX_Slot *const Slot, 
                                 void *const Buffer)
{
    switch (Slot->sfx->format)
    {
        case SOUND_FX_FORMAT_8BIT_MONO:
            mix8BitMono (Buffer, Slot);
            break;

        case SOUND_FX_FORMAT_8BIT_MONO_HSR:
            mix8BitMonoHSR (Buffer, Slot);
            break;

        case SOUND_FX_FORMAT_8BIT_MONO_QSR:
            mix8BitMonoQSR (Buffer, Slot);
            break;

        case SOUND_FX_FORMAT_16BIT_STEREO:
            mix16BitStereo (Buffer, Slot);
            break;

        case SOUND_FX_FORMAT_16BIT_MONO_HSR:
            mix16BitMonoHSR (Buffer, Slot);
            break;

        case SOUND_FX_FORMAT_16BIT_MONO_QSR:
            mix16BitMonoQSR (Buffer, Slot);
            break;

        default:
            return false;
    }

    return true;
}

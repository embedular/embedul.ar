/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SOUND driver] SDL mixer.

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

#include "embedul.ar/source/arch/native/sdl/drivers/board_hosted_sdl/sound_sdl.h"
#include "embedul.ar/source/core/device/board.h"
#include "SDL_mixer.h"


#define OPENAUDIO_FAILED_STR        "Mix_OpenAudio() failed"


static void     hardwareInit    (struct SOUND *const S);
static bool     mixBegin        (struct SOUND *const S);
static void     mixEnd          (struct SOUND *const S);


static const struct SOUND_IFACE SOUND_SDL_IFACE =
{
    .Description    = "sdl mixer",
    .HardwareInit   = hardwareInit,
    .Mute           = UNSUPPORTED,
    .BufferFilled   = UNSUPPORTED,
    .BufferConsumed = UNSUPPORTED,
    .MixBegin       = mixBegin,
    .MixEnd         = mixEnd
};


static void musicPlayer (void *udata, Uint8 *stream, int len)
{
    struct SOUND *const S = (struct SOUND *) udata;
    struct SOUND_SDL *const D = (struct SOUND_SDL *) S;

    if (mixBegin (S))
    {
        while (S->bufferFilledAvailable && len)
        {
            uint8_t     *cvData     = S->buffer[S->bufferFilledIndex];
            uint32_t    cvAvailable = SOUND_MIXER_BUFFER_SIZE - D->currentBufferUsed;

            // Current buffer has more or same bytes than required by SDL mixer
            if (cvAvailable >= (size_t)len)
            {
                memcpy (stream, cvData, (size_t)len);
                D->currentBufferUsed += (size_t)len;
                stream += len;
                len = 0;
            }
            // Current buffer has less bytes
            else
            {
                memcpy (stream, cvData, cvAvailable);
                D->currentBufferUsed = SOUND_MIXER_BUFFER_SIZE;
                stream += cvAvailable;
                len -= cvAvailable;
            }

            if (D->currentBufferUsed >= SOUND_MIXER_BUFFER_SIZE)
            {
                SOUND_BufferConsumed ();
                D->currentBufferUsed = 0;
            }
        }

        mixEnd (S);
    }

    if (len)
    {
        memset (stream, 0, (size_t)len);
    }
}


void SOUND_SDL_Init (struct SOUND_SDL *const D)
{
    BOARD_AssertParams (D);

    DEVICE_IMPLEMENTATION_Clear (D);

    D->mutex = SDL_CreateMutex ();

    SOUND_Init ((struct SOUND *)D, &SOUND_SDL_IFACE);
}


void hardwareInit (struct SOUND *const S)
{
    if (Mix_OpenAudio (SOUND_MIXER_SAMPLE_RATE, AUDIO_S16LSB, SOUND_MIXER_CHANNELS,
                       SOUND_MIXER_BUFFER_SIZE))
    {
        LOG_Warn (S, OPENAUDIO_FAILED_STR);
        LOG_Items (1, LANG_ERROR, Mix_GetError());

        BOARD_AssertInitialized (false);
    }

    Mix_HookMusic (musicPlayer, (void *)S);
}


bool mixBegin (struct SOUND *const S)
{
    struct SOUND_SDL *const D = (struct SOUND_SDL *) S;

    return (SDL_mutexP(D->mutex) != -1)? true : false;
}


void mixEnd (struct SOUND *const S)
{
    struct SOUND_SDL *const D = (struct SOUND_SDL *) S;

    SDL_mutexV (D->mutex);
}

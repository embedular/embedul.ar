/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [SOUND driver] pcm5100 stereo DAC through lpcopen I2S.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/sound_pcm5100.h"
#include "embedul.ar/source/core/device/board.h"


// Required on the DMA IRQhandler
static struct SOUND_PCM5100 *s_p = NULL;


static bool startDMAtransfer (struct SOUND_PCM5100 *p)
{
    if (!p->device.bufferFilledAvailable || p->dmaXferInProgress)
    {
        ++ p->dmaXferError;
        return false;
    }

    p->dmaXferInProgress = true;

    p->gpdmaChannel = Chip_GPDMA_GetFreeChannel (LPC_GPDMA, p->gpdmaConnTx);

    Chip_GPDMA_Transfer (LPC_GPDMA, p->gpdmaChannel,
                    (uintptr_t)p->device.buffer[p->device.bufferFilledIndex],
                    p->gpdmaConnTx,
                    GPDMA_TRANSFERTYPE_M2P_CONTROLLER_DMA,
                    SOUND_MIXER_BUFFER_SIZE >> 2);

    ++ p->dmaXferStarted;

    return true;
}


void DMA_IRQHandler (void)
{
    BOARD_AssertState (s_p);

    if (Chip_GPDMA_IntGetStatus (LPC_GPDMA, GPDMA_STAT_INT, s_p->gpdmaChannel))
    {
        ++ s_p->dmaXferFinished;

        if (Chip_GPDMA_Interrupt (LPC_GPDMA, s_p->gpdmaChannel) != SUCCESS)
        {
            ++ s_p->dmaXferError;
        }

        s_p->dmaXferInProgress = false;

        SOUND_BufferConsumed ();
    }
}


static void     hardwareInit    (struct SOUND *const S);
static void     mute            (struct SOUND *const S, const bool Enable);
static void     bufferFilled    (struct SOUND *const S);
static void     bufferConsumed  (struct SOUND *const S);


static const struct SOUND_IFACE SOUND_PCM5100_I2S_IFACE =
{
    .Description    = "lpcopen pcm5100 stereo dac",
    .HardwareInit   = hardwareInit,
    .Mute           = mute,
    .BufferFilled   = bufferFilled,
    .BufferConsumed = bufferConsumed
};


void SOUND_PCM5100_Init (struct SOUND_PCM5100 *const P,
                             LPC_I2S_T *const I2s, const uint32_t GpdmaConnTx)
{
    BOARD_AssertState  (!s_p);
    BOARD_AssertParams (P && I2s);

    DEVICE_IMPLEMENTATION_Clear (P);

    P->i2s          = I2s;
    P->gpdmaConnTx  = GpdmaConnTx;

    s_p = P;

    SOUND_Init ((struct SOUND *)P, &SOUND_PCM5100_I2S_IFACE);
}


void hardwareInit (struct SOUND *const S)
{
    struct SOUND_PCM5100 *const P = (struct SOUND_PCM5100 *) S;

    I2S_AUDIO_FORMAT_T i2sAudioFmt;

    i2sAudioFmt.SampleRate      = SOUND_MIXER_SAMPLE_RATE;
	i2sAudioFmt.ChannelNumber   = SOUND_MIXER_CHANNELS;
	i2sAudioFmt.WordWidth       = SOUND_MIXER_BITS;

    Chip_I2S_Init           (P->i2s);
	Chip_I2S_TxConfig       (P->i2s, &i2sAudioFmt);
	Chip_I2S_TxStop         (P->i2s);
	Chip_I2S_DisableMute    (P->i2s);
	Chip_I2S_TxStart        (P->i2s);

    Chip_I2S_DMA_TxCmd      (P->i2s, I2S_DMA_REQUEST_CHANNEL_1, ENABLE, 4);
    Chip_GPDMA_Init         (LPC_GPDMA);
    NVIC_DisableIRQ         (DMA_IRQn);
	NVIC_SetPriority        (DMA_IRQn, 7);
	NVIC_EnableIRQ          (DMA_IRQn);

    mute (S, false);
}


void mute (struct SOUND *const S, const bool Enable)
{
    (void) S;

    MIO_SET_OUTPUT_BIT_NOW (CONTROL, SoundMute, Enable? 1 : 0);
}


void bufferFilled (struct SOUND *const S)
{
    struct SOUND_PCM5100 *const P = (struct SOUND_PCM5100 *) S;

    if (!P->dmaXferInProgress)
    {
        startDMAtransfer (P);
    }
}


void bufferConsumed (struct SOUND *const S)
{
    struct SOUND_PCM5100 *const P = (struct SOUND_PCM5100 *) S;

    if (S->bufferFilledAvailable)
    {
        startDMAtransfer (P);
    }
}

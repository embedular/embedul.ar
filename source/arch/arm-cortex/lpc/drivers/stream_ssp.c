/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] lpc43xx spi communication.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/stream_ssp.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/device.h"


// Common IO interface
static void         hardwareInit    (struct STREAM *const S);
static enum DEVICE_CommandResult
                    command         (struct STREAM *const S,
                                     const char *const Name,
                                     struct VARIANT *const Value);
static uint32_t     dataIn          (struct STREAM *const S, 
                                     const uint8_t *const Data,
                                     const uint32_t Octets);
static uint32_t     dataOut         (struct STREAM *const S,
                                     uint8_t *const Buffer,
                                     const uint32_t Octets);
static struct STREAM_CompResult
                    dataComp       (struct STREAM *const S,
                                     const uint8_t *const InData,
                                     const uint32_t InOctets,
                                     uint8_t *const OutBuffer,
                                     const uint32_t OutOctets);


static const struct STREAM_IFACE STREAM_SSP_IFACE =
{
    .Description    = "lpc43xx ssp",
    .HardwareInit   = hardwareInit,
    .Command        = command,
    .DataIn         = dataIn,
    .DataOut        = dataOut,
    .DataComp       = dataComp
};


static bool checkBits (const uint8_t Bits)
{
    return (Bits >= 4 && Bits <= 16);
}


static bool checkFrameFormat (const enum STREAM_SSP_FrameFmt FrameFmt)
{
    return (FrameFmt < STREAM_SSP_FrameFmt__COUNT);
}


static bool checkClockFormat (const enum STREAM_SSP_ClockFmt ClockFmt)
{
    return (ClockFmt <= 3);
}


void STREAM_SSP_Init (struct STREAM_SSP *const P,
                      LPC_SSP_T *const Ssp, 
                      const enum STREAM_SSP_Role Role,
                      const enum STREAM_SSP_FrameFmt FrameFmt,
                      const uint32_t Speed,
                      const uint8_t Bits,
                      const enum STREAM_SSP_ClockFmt ClockFmt)
{
    // LPC43xx devices only have two spi interfaces: SPI0 and SP1
    BOARD_AssertParams (P && Ssp && Speed && checkFrameFormat(FrameFmt) &&
                        checkBits(Bits) && checkClockFormat(ClockFmt));

    DEVICE_IMPLEMENTATION_Clear (P);

    // Immutable settings: Role, FrameFmt, ClockFmt.
    P->ssp      = Ssp;
    P->role     = Role;
    P->frameFmt = FrameFmt;
    P->speed    = Speed;
    P->bits     = Bits;
    P->clockFmt = ClockFmt;

    STREAM_Init ((struct STREAM *)P, &STREAM_SSP_IFACE);
}


static void sspSetFormat (struct STREAM_SSP *const S)
{
    // See CHIP_SSP_BITS, CHIP_SSP_FRAME_FORMAT, and CHIP_SSP_CLOCK_FORMAT
    const uint32_t SSP_Bits     = S->bits - 1;
    const uint32_t SSP_Format   = S->frameFmt << 4;
    const uint32_t SSP_Clock    = S->clockFmt << 6;

    Chip_SSP_SetFormat (S->ssp, SSP_Bits, SSP_Format, SSP_Clock);
}


static void hardwareInit (struct STREAM *const S)
{
    struct STREAM_SSP *const P = (struct STREAM_SSP *) S;

    Chip_SSP_Init       (P->ssp);
    Chip_SSP_Set_Mode   (P->ssp, P->role == STREAM_SSP_Role_Controller?
                         SSP_MODE_MASTER : SSP_MODE_SLAVE);
    sspSetFormat        (P);
    Chip_SSP_SetBitRate (P->ssp, P->speed);
    Chip_SSP_Enable     (P->ssp);
}


static enum DEVICE_CommandResult command (struct STREAM *const S,
                                          const char *const Name,
                                          struct VARIANT *const Value)
{
    struct STREAM_SSP *const P = (struct STREAM_SSP *) S;

    if (DEVICE_COMMAND_CHECK(STREAM_SET_SPEED))
    {
        const uint32_t Speed = VARIANT_ToUint (Value);
        Chip_SSP_SetBitRate (P->ssp, Speed);
    }
    else if (DEVICE_COMMAND_CHECK(STREAM_SET_FRAME_BITS))
    {
        const uint32_t FrameBits = VARIANT_ToUint (Value);
        if (!checkBits(FrameBits))
        {
            return DEVICE_CommandResult_Failed;
        }

        P->bits = FrameBits;
        sspSetFormat (P);
    }
    else
    {
        return DEVICE_CommandResult_NotHandled;
    }

    return DEVICE_CommandResult_Ok;
}


static uint32_t dataIn (struct STREAM *const S, const uint8_t *const Data,
                        const uint32_t Octets)
{
    struct STREAM_SSP *const P = (struct STREAM_SSP *) S;

    const uint32_t SentOctets =
                    Chip_SSP_WriteFrames_Blocking (P->ssp, Data, Octets);
    return SentOctets;
}


static uint32_t dataOut (struct STREAM *const S, uint8_t *const Buffer,
                         const uint32_t Octets)
{
    struct STREAM_SSP *const P = (struct STREAM_SSP *) S;

    const uint32_t RecvOctets =
                    Chip_SSP_ReadFrames_Blocking (P->ssp, Buffer, Octets);

    return RecvOctets;
}


static struct STREAM_CompResult dataComp (struct STREAM *const S,
                                            const uint8_t *const InData,
                                            const uint32_t InOctets,
                                            uint8_t *const OutBuffer,
                                            const uint32_t OutOctets)
{
    BOARD_AssertParams (InOctets == OutOctets);

    struct STREAM_SSP *const P = (struct STREAM_SSP *) S;

    P->ds.tx_data   = (void *) InData;
	P->ds.tx_cnt    = 0;
	P->ds.rx_data   = (void *) OutBuffer;
	P->ds.rx_cnt    = 0;
	P->ds.length    = InOctets;

    Chip_SSP_RWFrames_Blocking (P->ssp, &P->ds);

    BOARD_AssertState ((P->ds.tx_cnt == P->ds.rx_cnt) == InOctets);

    return (struct STREAM_CompResult) 
    {
        .inCount    = P->ds.tx_cnt,
        .outCount   = P->ds.rx_cnt
    };
}

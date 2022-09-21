/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [PACKET driver] lpc43xx spi communication.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/packet_ssp.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/device.h"


// Common IO interface
static void         hardwareInit    (struct PACKET *const P);
static enum DEVICE_CommandResult
                    command         (struct PACKET *const P,
                                     const char *const Name,
                                     struct VARIANT *const Value);
static uint32_t     send            (struct PACKET *const P, 
                                     const uint8_t *const Data,
                                     const uint32_t Octets);
static uint32_t     recvSize        (struct PACKET *const P,
                                     const uint32_t AvailableOctets);
static uint32_t     recv            (struct PACKET *const P,
                                     uint8_t *const Buffer,
                                     const uint32_t Octets);
static struct PACKET_BidirResult
                    bidir           (struct PACKET *const P,
                                     const uint8_t *const SendData,
                                     const uint32_t SendOctets,
                                     uint8_t *const RecvData,
                                     const uint32_t RecvOctets);


static const struct PACKET_IFACE PACKET_SSP_IFACE =
{
    .Description    = "lpc43xx ssp",
    .HardwareInit   = hardwareInit,
    .Command        = command,
    .Send           = send,
    .RecvSize       = recvSize,
    .Recv           = recv,
    .Bidir          = bidir
};


static bool checkBits (const uint8_t Bits)
{
    return (Bits >= 4 && Bits <= 16);
}


static bool checkFrameFormat (const enum PACKET_SSP_FrameFmt FrameFmt)
{
    return (FrameFmt < PACKET_SSP_FrameFmt__COUNT);
}


static bool checkClockFormat (const enum PACKET_SSP_ClockFmt ClockFmt)
{
    return (ClockFmt <= 3);
}


void PACKET_SSP_Init (struct PACKET_SSP *const S,
                      LPC_SSP_T *const Ssp, 
                      const enum PACKET_SSP_Role Role,
                      const enum PACKET_SSP_FrameFmt FrameFmt,
                      const uint32_t Speed,
                      const uint8_t Bits,
                      const enum PACKET_SSP_ClockFmt ClockFmt)
{
    // LPC43xx devices only have two spi interfaces: SPI0 and SP1
    BOARD_AssertParams (S && Ssp && Speed && checkFrameFormat(FrameFmt) &&
                        checkBits(Bits) && checkClockFormat(ClockFmt));

    DEVICE_IMPLEMENTATION_Clear (S);

    // Immutable settings: Role, FrameFmt, ClockFmt.
    S->ssp      = Ssp;
    S->role     = Role;
    S->frameFmt = FrameFmt;
    S->speed    = Speed;
    S->bits     = Bits;
    S->clockFmt = ClockFmt;

    PACKET_Init ((struct PACKET *)S, &PACKET_SSP_IFACE);
}


static void sspSetFormat (struct PACKET_SSP *const S)
{
    // See CHIP_SSP_BITS, CHIP_SSP_FRAME_FORMAT, and CHIP_SSP_CLOCK_FORMAT
    const uint32_t SSP_Bits     = S->bits - 1;
    const uint32_t SSP_Format   = S->frameFmt << 4;
    const uint32_t SSP_Clock    = S->clockFmt << 6;

    Chip_SSP_SetFormat (S->ssp, SSP_Bits, SSP_Format, SSP_Clock);
}


static void hardwareInit (struct PACKET *const P)
{
    struct PACKET_SSP *const S = (struct PACKET_SSP *) P;

    Chip_SSP_Init       (S->ssp);
    Chip_SSP_Set_Mode   (S->ssp, S->role == PACKET_SSP_Role_Controller?
                         SSP_MODE_MASTER : SSP_MODE_SLAVE);
    sspSetFormat        (S);
    Chip_SSP_SetBitRate (S->ssp, S->speed);
    Chip_SSP_Enable     (S->ssp);
}


static enum DEVICE_CommandResult command (struct PACKET *const P,
                                          const char *const Name,
                                          struct VARIANT *const Value)
{
    struct PACKET_SSP *const S = (struct PACKET_SSP *) P;

    if (DEVICE_COMMAND_CHECK(SET_SPEED))
    {
        const uint32_t Speed = VARIANT_ToUint (Value);
        Chip_SSP_SetBitRate (S->ssp, Speed);
    }
    else if (DEVICE_COMMAND_CHECK(SET_FRAME_BITS))
    {
        const uint32_t FrameBits = VARIANT_ToUint (Value);
        if (!checkBits(FrameBits))
        {
            return DEVICE_CommandResult_Failed;
        }
        S->bits = FrameBits;
        sspSetFormat (S);
    }
    else
    {
        return DEVICE_CommandResult_NotHandled;
    }

    return DEVICE_CommandResult_Ok;
}


static uint32_t send (struct PACKET *const P, const uint8_t *const Data,
                      const uint32_t Octets)
{
    struct PACKET_SSP *const S = (struct PACKET_SSP *) P;

    const uint32_t SentOctets = Chip_SSP_WriteFrames_Blocking (S->ssp, 
                                                                Data, Octets);
    return SentOctets;
}


static uint32_t recvSize (struct PACKET *const P, const uint32_t MaxOctets)
{
    (void) P;

    return MaxOctets;
}


static uint32_t recv (struct PACKET *const P, uint8_t *const Buffer,
                      const uint32_t Octets)
{
    struct PACKET_SSP *const S = (struct PACKET_SSP *) P;

    const uint32_t RecvOctets = Chip_SSP_ReadFrames_Blocking (S->ssp,
                                                                Buffer, Octets);
    return RecvOctets;
}


static struct PACKET_BidirResult bidir (struct PACKET *const P,
                       const uint8_t *const SendData, const uint32_t SendOctets,
                       uint8_t *const RecvData, const uint32_t RecvOctets)
{
    BOARD_AssertParams (SendOctets == RecvOctets);

    struct PACKET_SSP *const S = (struct PACKET_SSP *) P;

    S->ds.tx_data   = (void *) SendData;
	S->ds.tx_cnt    = 0;
	S->ds.rx_data   = (void *) RecvData;
	S->ds.rx_cnt    = 0;
	S->ds.length    = SendOctets;

    Chip_SSP_RWFrames_Blocking (S->ssp, &S->ds);

    BOARD_AssertState ((S->ds.tx_cnt == S->ds.rx_cnt) == SendOctets);

    return (struct PACKET_BidirResult) {
                        .sentOctets = S->ds.tx_cnt,
                        .recvOctets = S->ds.rx_cnt
                    };
}

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] lpc43xx i2c controller.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/stream_i2c_controller.h"
#include "embedul.ar/source/core/device/board.h"


// Common IO interface
static void         hardwareInit    (struct STREAM *const S);
static enum DEVICE_CommandResult
                    command         (struct STREAM *const S,
                                     const char *const Name,
                                     struct VARIANT *const Value);
static uint32_t     dataIn          (struct STREAM *const S, 
                                     const uint8_t *const Data,
                                     const uint32_t Octets);
static struct STREAM_DataExchangeResult
                    DataExchange        (struct STREAM *const S,
                                     const uint8_t *const InData,
                                     const uint32_t InOctets,
                                     uint8_t *const OutBuffer,
                                     const uint32_t OutOctets);


static const struct STREAM_IFACE STREAM_I2C_CONTROLLER_IFACE =
{
    .Description    = "lpc43xx i2c controller",
    .HardwareInit   = hardwareInit,
    .Command        = command,
    .DataIn         = dataIn,
    .DataExchange       = DataExchange
};


void STREAM_I2C_CONTROLLER_Init (struct STREAM_I2C_CONTROLLER *const I,
                                 LPC_I2C_T *const I2c, const uint32_t Speed)
{
    // LPC43xx devices only have two i2c interfaces: I2C0 and I2C1
    BOARD_AssertParams (I && I2c && Speed &&
                        (I2c == LPC_I2C0 || I2c == LPC_I2C1));

    DEVICE_IMPLEMENTATION_Clear (I);

    I->i2c      = I2c;
    I->i2cId    = (I->i2c == LPC_I2C0)? I2C0 : I2C1;
    I->speed    = Speed;

    STREAM_Init ((struct STREAM *)I, &STREAM_I2C_CONTROLLER_IFACE);
}


static void hardwareInit (struct STREAM *const S)
{
    struct STREAM_I2C_CONTROLLER *const I = (struct STREAM_I2C_CONTROLLER *) S;

    Chip_I2C_Init (I->i2cId);

    // Set speed
    STREAM_Command (S, DEVICE_COMMAND_STREAM_SET_SPEED,
                    &VARIANT_SpawnUint(I->speed));
}


static enum DEVICE_CommandResult command (struct STREAM *const S,
                                          const char *const Name,
                                          struct VARIANT *const Value)
{
    struct STREAM_I2C_CONTROLLER *const I = (struct STREAM_I2C_CONTROLLER *) S;

    if (DEVICE_COMMAND_CHECK(STREAM_SET_SPEED))
    {
        const uint32_t Speed = VARIANT_ToUint (Value);

        // Only I2C0 support speeds above fast mode
        if (I->i2cId == I2C0)
        {
            Chip_SCU_I2C0PinConfig (Speed <= 400000? 
                                    I2C0_STANDARD_FAST_MODE :
                                    I2C0_FAST_MODE_PLUS);
        }
        else if (Speed > 400000)
        {
            return DEVICE_CommandResult_Failed;
        }

        Chip_I2C_SetClockRate (I->i2cId, Speed);
    }
    else
    {
        return DEVICE_CommandResult_NotHandled;
    }

    return DEVICE_CommandResult_Ok;
}


// It doesn't work right. sometimes it skips bytes, sometimes sends trash.
// works perfect on a debugging session though.
#if PACKET_I2C_CONTROLLER_NONBLOCKING
static void xferUpdate (struct PACKET_I2C_CONTROLLER *const I,
                        const uint8_t *const SendData,
                        const uint32_t SendOctets,
                        uint8_t *const RecvData,
                        const uint32_t RecvOctets)
{
    struct PACKET *const P = &I->device;

    // First iteration, initialize a new transfer.
    if (!P->iteration)
    {
        const uint64_t TargetAddress = VARIANT_ToUint (&P->sendTo);
        BOARD_AssertParams (TargetAddress < 128);

        I->xfer.slaveAddr   = TargetAddress;
        I->xfer.options     = 0;
        I->xfer.status      = 0;
        I->xfer.txBuff      = SendData;
        I->xfer.txSz        = SendOctets;
        I->xfer.rxBuff      = RecvData;
        I->xfer.rxSz        = RecvOctets;

        Chip_I2CM_Xfer (I->i2c, &I->xfer);
    }

    // Keep polling a previusly started transfer.
    if (Chip_I2CM_StateChanged (I->i2c))
    {
        Chip_I2CM_XferHandler (I->i2c, &I->xfer);

        switch (I->xfer.status)
        {
            case I2CM_STATUS_OK:
                BOARD_AssertState (!I->xfer.txSz && !I->xfer.rxSz);
                break;

            case  I2CM_STATUS_NAK:
                P->error = PACKET_Error_NoAck;
                break;

            case I2CM_STATUS_BUS_ERROR:
                P->error = PACKET_Error_Bus;
                break;

            case I2CM_STATUS_SLAVE_NAK:
                P->error = PACKET_Error_TargetNoAck;
                break;

            case I2CM_STATUS_ARBLOST:
                P->error = PACKET_Error_ArbitrationLost;
                break;

            case I2CM_STATUS_BUSY:
                break;

            case I2CM_STATUS_ERROR:
            default:
                P->error = PACKET_Error_Unknown;
                break;
        }
    }
}
#else
static void xferBlocking (struct STREAM_I2C_CONTROLLER *const I,
                          const uint8_t *const InData,
                          const uint32_t InOctets,
                          uint8_t *const OutBuffer,
                          const uint32_t OutOctets)
{
    struct STREAM *const S = &I->device;

    const uint64_t TargetAddress = VARIANT_ToUint (&S->address);
    BOARD_AssertParams (TargetAddress < 128);

    I->xfer.slaveAddr   = TargetAddress;
    I->xfer.options     = 0;
    I->xfer.status      = 0;
    I->xfer.txBuff      = InData;
    I->xfer.txSz        = InOctets;
    I->xfer.rxBuff      = OutBuffer;
    I->xfer.rxSz        = OutOctets;

    Chip_I2CM_XferBlocking (I->i2c, &I->xfer);

    switch (I->xfer.status)
    {
        case I2CM_STATUS_OK:
            BOARD_AssertState (!I->xfer.txSz && !I->xfer.rxSz);
            break;

        case I2CM_STATUS_NAK:
            S->status = STREAM_TransferStatus_NoAck;
            break;

        case I2CM_STATUS_BUS_ERROR:
            S->status = STREAM_TransferStatus_BusError;
            break;

        case I2CM_STATUS_SLAVE_NAK:
            S->status = STREAM_TransferStatus_TargetNoAck;
            break;

        case I2CM_STATUS_ARBLOST:
            S->status = STREAM_TransferStatus_ArbitrationLost;
            break;

        case I2CM_STATUS_BUSY:
            break;

        case I2CM_STATUS_ERROR:
        default:
            S->status = STREAM_TransferStatus_UnknownError;
            break;
    }
}
#endif


static uint32_t dataIn (struct STREAM *const S, const uint8_t *const Data,
                        const uint32_t Octets)
{
    struct STREAM_I2C_CONTROLLER *const I = (struct STREAM_I2C_CONTROLLER *) S;

    #ifdef PACKET_I2C_CONTROLLER_NONBLOCKING
    xferUpdate (I, Data, Octets, NULL, 0);
    #else 
    xferBlocking (I, Data, Octets, NULL, 0);
    #endif

    return Octets - I->xfer.txSz;
}


static struct STREAM_DataExchangeResult DataExchange (struct STREAM *const S,
                                          const uint8_t *const InData,
                                          const uint32_t InOctets,
                                          uint8_t *const OutBuffer,
                                          const uint32_t OutOctets)
{
    struct STREAM_I2C_CONTROLLER *const I = (struct STREAM_I2C_CONTROLLER *) S;

#ifdef PACKET_I2C_CONTROLLER_NONBLOCKING
    xferUpdate (I, InData, InOctets, OutBuffer, OutOctets);    
#else 
    xferBlocking (I, InData, InOctets, OutBuffer, OutOctets);
#endif

    return (struct STREAM_DataExchangeResult)
    {
        .inCount    = InOctets  - I->xfer.txSz,
        .outCount   = OutOctets - I->xfer.rxSz
    };
}

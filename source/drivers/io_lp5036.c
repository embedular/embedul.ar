/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] lp5036 rgb led driver device controller.

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

#include "embedul.ar/source/drivers/io_lp5036.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/device/packet/error_log.h"


static const char * s_OutputNamesRange[IO_LP5036_OUTR__COUNT] =
{
    "out0", "out1", "out2", "out3", "out4",
    "out5", "out6", "out7", "out8", "out9", 
    "out10", "out11", "out12", "out13", "out14",
    "out15", "out16", "out17", "out18", "out19", 
    "out20", "out21", "out22", "out23", "out24",
    "out25", "out26", "out27", "out28", "out29", 
    "out30", "out31", "out32", "out33", "out34",
    "out35"
};


static void         hardwareInit        (struct IO *const Io);
static void         update              (struct IO *const Io);
static IO_Count
                    availableOutputs    (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t OutputSource);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t OutputSource,
                                         const uint32_t Value);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);


static const struct IO_IFACE IO_LP5036_IFACE =
{
    .Description        = "lp5036 36-channel led driver",
    .HardwareInit       = hardwareInit,
    .Update             = update,
    .AvailableInputs    = UNSUPPORTED,
    .AvailableOutputs   = availableOutputs,
    .GetInput           = UNSUPPORTED,
    .SetOutput          = setOutput,
    .InputName          = UNSUPPORTED,
    .OutputName         = outputName
};


static bool deviceUpdate (struct IO_LP5036 *const L)
{
    // [0] = 0x14: set OUT0_Color (brightness of each R,G, and B component, 
    //             auto-incremented to OUT35_Color).
    // [1..36] = OUT0_Color..OUT35_Color values.
    L->outData[0] = 0x14;

    PACKET_SendTo       (L->packet, &VARIANT_SpawnUint(L->i2cAddr));
    PACKET_SendTimeout  (L->packet, 5);
    PACKET_Send         (L->packet, L->outData, 1 + IO_LP5036_OUTR__COUNT);

    return PACKET_ERROR_LOG_Generic (L->packet);
}


void IO_LP5036_Init (struct IO_LP5036 *const L, const enum COMM_Packet Com,
                     const uint8_t I2cAddr, const uint8_t MaxIntensity)
{
    BOARD_AssertParams (L);

    DEVICE_IMPLEMENTATION_Clear (L);

    L->packet       = COMM_GetPacket (Com);
    L->i2cAddr      = I2cAddr;
    L->maxIntensity = MaxIntensity;

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)L, &IO_LP5036_IFACE, 15);
}


static void hardwareInit (struct IO *const Io)
{
    struct IO_LP5036 *const L = (struct IO_LP5036 *) Io;

    // [0] = 0x00: set DEVICE_CONFIG0.
    // [1] = 0x40: bit 6, Chip_EN, going from standby to normal operation mode.
    L->outData[0] = 0x00;
    L->outData[1] = 0x40;

    PACKET_SendTo       (L->packet, &VARIANT_SpawnUint(L->i2cAddr));
    PACKET_SendTimeout  (L->packet, 5);
    PACKET_Send         (L->packet, L->outData, 2);

    if (PACKET_ERROR_LOG_nAck (L->packet))
    {
        BOARD_AssertInitialized (false);
    }

    // [0] = 0x08: set LED0_BRIGHTNESS (intensity of each RGB triplet, 
    //             auto-incremented to LED11_BRIGHTNESS).
    // [1..12] = LED0_BRIGHTNESS..LED11_BRIGHTNESS values. 
    L->outData[0] = 0x08;
    memset (&L->outData[1], L->maxIntensity, 36);

    PACKET_SendTo       (L->packet, &VARIANT_SpawnUint(L->i2cAddr));
    PACKET_SendTimeout  (L->packet, 5);
    PACKET_Send         (L->packet, L->outData, 1 + 36);

    if (PACKET_ERROR_LOG_Generic (L->packet))
    {
        BOARD_AssertInitialized (false);
    }

    // Initial status: all channels ON.
    memset (&L->outData[1], 0xFF, IO_LP5036_OUTR__COUNT);

    // Returns 'true' on error.
    if (deviceUpdate (L))
    {
        BOARD_AssertInitialized (false);
    }
}


static void update (struct IO *const Io)
{
    struct IO_LP5036 *const L = (struct IO_LP5036 *) Io;

    deviceUpdate (L);
}


static IO_Count availableOutputs (struct IO *const Io,
                                  const enum IO_Type IoType,
                                  const uint32_t OutputSource)
{
    (void) Io;
    (void) OutputSource;

    // This driver handles no digital outputs
    if (IoType == IO_Type_Bit)
    {
        return 0;
    }

    return IO_LP5036_OUTR__COUNT;
}


static void setOutput (struct IO *const Io, const enum IO_Type IoType,
                       const uint16_t Index, const uint32_t OutputSource,
                       const uint32_t Value)
{
    BOARD_AssertParams (IoType == IO_Type_Range &&
                         Index < IO_LP5036_OUTR__COUNT &&
                         Value <= 0xFF);

    (void) OutputSource;

    struct IO_LP5036 *const L = (struct IO_LP5036 *) Io;

    // outData[0] reserved as the i2c registry to write to.
    L->outData[Index + 1] = Value;
}


static const char * outputName (struct IO *const Io,
                                const enum IO_Type IoType,
                                const uint16_t Index)
{
    BOARD_AssertParams (IoType == IO_Type_Range &&
                         Index < IO_LP5036_OUTR__COUNT);

    (void) Io;

    return s_OutputNamesRange[Index];
}

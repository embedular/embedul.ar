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
    [IO_LP5036_OUTR_0]  = "out0",
    [IO_LP5036_OUTR_1]  = "out1",
    [IO_LP5036_OUTR_2]  = "out2",
    [IO_LP5036_OUTR_3]  = "out3",
    [IO_LP5036_OUTR_4]  = "out4",
    [IO_LP5036_OUTR_5]  = "out5",
    [IO_LP5036_OUTR_6]  = "out6",
    [IO_LP5036_OUTR_7]  = "out7",
    [IO_LP5036_OUTR_8]  = "out8",
    [IO_LP5036_OUTR_9]  = "out9", 
    [IO_LP5036_OUTR_10] = "out10",
    [IO_LP5036_OUTR_11] = "out11",
    [IO_LP5036_OUTR_12] = "out12",
    [IO_LP5036_OUTR_13] = "out13",
    [IO_LP5036_OUTR_14] = "out14",
    [IO_LP5036_OUTR_15] = "out15",
    [IO_LP5036_OUTR_16] = "out16",
    [IO_LP5036_OUTR_17] = "out17",
    [IO_LP5036_OUTR_18] = "out18",
    [IO_LP5036_OUTR_19] = "out19", 
    [IO_LP5036_OUTR_20] = "out20",
    [IO_LP5036_OUTR_21] = "out21",
    [IO_LP5036_OUTR_22] = "out22",
    [IO_LP5036_OUTR_23] = "out23",
    [IO_LP5036_OUTR_24] = "out24",
    [IO_LP5036_OUTR_25] = "out25",
    [IO_LP5036_OUTR_26] = "out26",
    [IO_LP5036_OUTR_27] = "out27",
    [IO_LP5036_OUTR_28] = "out28",
    [IO_LP5036_OUTR_29] = "out29", 
    [IO_LP5036_OUTR_30] = "out30",
    [IO_LP5036_OUTR_31] = "out31",
    [IO_LP5036_OUTR_32] = "out32",
    [IO_LP5036_OUTR_33] = "out33",
    [IO_LP5036_OUTR_34] = "out34",
    [IO_LP5036_OUTR_35] = "out35"
};


static void         hardwareInit        (struct IO *const Io);
static void         update              (struct IO *const Io);
static IO_Count
                    availableOutputs    (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Port OutPort);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Inx,
                                         const IO_Port OutPort,
                                         const uint32_t Value);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code Inx);


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
                                  const IO_Port OutPort)
{
    (void) Io;
    (void) OutPort;

    // This driver handles no digital outputs
    if (IoType == IO_Type_Bit)
    {
        return 0;
    }

    return IO_LP5036_OUTR__COUNT;
}


static void setOutput (struct IO *const Io, const enum IO_Type IoType,
                       const IO_Code Inx, const IO_Port OutPort,
                       const uint32_t Value)
{
    BOARD_AssertParams (IoType == IO_Type_Range &&
                        Inx < IO_LP5036_OUTR__COUNT &&
                        Value <= 0xFF);

    (void) OutPort;

    struct IO_LP5036 *const L = (struct IO_LP5036 *) Io;

    // outData[0] reserved as the i2c registry to write to.
    L->outData[Inx + 1] = Value;
}


static const char * outputName (struct IO *const Io,
                                const enum IO_Type IoType,
                                const IO_Code Inx)
{
    BOARD_AssertParams (IoType == IO_Type_Range &&
                        Inx < IO_LP5036_OUTR__COUNT);

    (void) Io;

    return s_OutputNamesRange[Inx];
}

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] pca9956b led lighting device driver.

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

#include "embedul.ar/source/drivers/io_pca9956b.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/device/packet/error_log.h"


#define PCA9956B_UPDATE_IREF                0x01
#define PCA9956B_UPDATE_PWM                 0x02
#define PCA9956B_UPDATE_ALL                 0xFF

#define PCA9956B_CH_STATUS_NORMAL           0x00
#define PCA9956B_CH_STATUS_SHORT            0x01
#define PCA9956B_CH_STATUS_OPEN             0x02

// Mode 2 register.
#define PCA9956B_MODE2                      0x01
// Auto increment starting from IREF0.
#define PCA9956B_AUTOINC_IREF0              (0x80 | 0x22)
// Auto increment starting from PWM0.
#define PCA9956B_AUTOINC_PWM0               (0x80 | 0x0A)
// Auto increment starting from EFLAG0.
#define PCA9956B_AUTOINC_EFLAG0             (0x80 | 0x41)


static const char *
s_InputNamesBit[IO_PCA9956B_INB__COUNT] =
{
    "overtemp", "channel error"
};


static const char *
s_InputNamesRange[IO_PCA9956B_INR__COUNT] =
{
    "channels shorted", "channels open"
};


static const char * s_OutputNamesRange[IO_PCA9956B_OUTR__COUNT] =
{
    "ch0 iref", "ch1 iref", "ch2 iref", "ch3 iref", "ch4 iref",
    "ch5 iref", "ch6 iref", "ch7 iref", "ch8 iref", "ch9 iref", 
    "ch10 iref", "ch11 iref", "ch12 iref", "ch13 iref", "ch14 iref",
    "ch15 iref", "ch16 iref", "ch17 iref", "ch18 iref", "ch19 iref", 
    "ch20 iref", "ch21 iref", "ch22 iref", "ch23 iref",
    "ch0 pwm", "ch1 pwm", "ch2 pwm", "ch3 pwm", "ch4 pwm",
    "ch5 pwm", "ch6 pwm", "ch7 pwm", "ch8 pwm", "ch9 pwm", 
    "ch10 pwm", "ch11 pwm", "ch12 pwm", "ch13 pwm", "ch14 pwm",
    "ch15 pwm", "ch16 pwm", "ch17 pwm", "ch18 pwm", "ch19 pwm", 
    "ch20 pwm", "ch21 pwm", "ch22 pwm", "ch23 pwm"
};


static void         hardwareInit        (struct IO *const Io);
static void         update              (struct IO *const Io);
static IO_Count
                    availableInputs     (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t InputSource);
static IO_Count
                    availableOutputs    (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t OutputSource);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t InputSource);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t OutputSource,
                                         const uint32_t Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);


static const struct IO_IFACE IO_PCA9956B_IFACE =
{
    .Description        = "pca9956B 24-channel led driver",
    .HardwareInit       = hardwareInit,
    .Update             = update,
    .AvailableInputs    = availableInputs,
    .AvailableOutputs   = availableOutputs,
    .GetInput           = getInput,
    .SetOutput          = setOutput,
    .InputName          = inputName,
    .OutputName         = outputName
};


void IO_PCA9956B_Init (struct IO_PCA9956B *const P,
                       const enum COMM_Packet Com, const uint8_t I2cAddr)
{
    BOARD_AssertParams (P);

    DEVICE_IMPLEMENTATION_Clear (P);

    P->packet   = COMM_GetPacket (Com);
    P->i2cAddr  = I2cAddr;

    // Update at 60 Hz.
    IO_Init ((struct IO *)P, &IO_PCA9956B_IFACE, 15);
}


void IO_PCA9956B_Attach (struct IO_PCA9956B *const P,
                         const uint32_t ChannelOffset,
                         const uint32_t DeviceOffset)
{
    BOARD_AssertParams (P);
    BOARD_AssertParams (ChannelOffset + IO_PCA9956B_CHANNEL_COUNT
                         <= LIB_EMBEDULAR_CONFIG_OUTPUT_MAX_LIGHT_CHANNELS);
    BOARD_AssertParams (DeviceOffset < 
                        LIB_EMBEDULAR_CONFIG_INPUT_MAX_LIGHTING_DEVICES);


    OUTPUT_SetDevice ((struct IO *)P, 0);

    const enum OUTPUT_Range IrefStart = OUTPUT_Range_LightCh0Iref +
                                        ChannelOffset;
    const enum OUTPUT_Range PwmStart = OUTPUT_Range_LightCh0Pwm +
                                       ChannelOffset;

    for (uint32_t i = 0; i < IO_PCA9956B_CHANNEL_COUNT; ++i)
    {
        OUTPUT_MapRange (IrefStart + i, IO_PCA9956B_OUTR_CH0_IREF + i);
        OUTPUT_MapRange (PwmStart + i, IO_PCA9956B_OUTR_CH0_PWM + i);
    }

    INPUT_SetDevice ((struct IO *)P, 0);

    INPUT_MapRange (INPUT_PROFILE_Type_LIGHTDEV,
                    INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted(DeviceOffset),
                    IO_PCA9956B_INR_CHANNELS_SHORTED_BITFIELD);

    INPUT_MapRange (INPUT_PROFILE_Type_LIGHTDEV,
                    INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen(DeviceOffset),
                    IO_PCA9956B_INR_CHANNELS_OPEN_BITFIELD);

    INPUT_MapBit (INPUT_PROFILE_Type_LIGHTDEV,
                  INPUT_PROFILE_LIGHTDEV_Bit_Overtemp(DeviceOffset),
                  IO_PCA9956B_INB_OVERTEMP);

    INPUT_MapBit (INPUT_PROFILE_Type_LIGHTDEV,
                  INPUT_PROFILE_LIGHTDEV_Bit_ChannelError(DeviceOffset),
                  IO_PCA9956B_INB_CHANNEL_ERROR);
}


static void channelStatus (struct IO_PCA9956B *const P, const uint8_t Ch,
                           const uint8_t Status)
{
    if (P->chIref[Ch] && P->chPwm[Ch])
    {
        P->channelsShorted  |= (Status & PCA9956B_CH_STATUS_SHORT)? 1 << Ch : 0;
        P->channelsOpen     |= (Status & PCA9956B_CH_STATUS_OPEN)?  1 << Ch : 0;
    }
    else 
    {
        P->channelsShorted  &= ~((uint32_t)(1 << Ch));
        P->channelsOpen     &= ~((uint32_t)(1 << Ch));
    }
}


static bool deviceMode2Status (struct IO_PCA9956B *const P)
{
    uint8_t tx;
    uint8_t rx;

    // Get MODE2 device status
    tx = PCA9956B_MODE2;

    PACKET_SendTo   (P->packet, &VARIANT_SpawnUint(P->i2cAddr));
    PACKET_Bidir    (P->packet, &tx, 1, &rx, 1);

    if (PACKET_ERROR_LOG_nAck (P->packet))
    {
        return false;
    }

    // Lower 3 bits should always read 101 (0x05)
    if ((rx & 0x07) != 0x05)
    {
        LOG_WarnDebug (&P->device, LANG_UNEXPECTED_VALUE);
        LOG_Items (2,
                    LANG_DEVICE_ADDRESS,    P->i2cAddr,
                    LANG_VALUE,             (uint8_t)(rx & 0x07),
                    LOG_ItemsBases(VARIANT_Base_Hex_UpperSuffix,
                                   VARIANT_Base_Hex_UpperSuffix));
        return false;
    }

    P->overtemp = (rx & 0x80)? 1 : 0;
    P->chError  = (rx & 0x40)? 1 : 0;

    return true;
}


static void deviceFullStatus (struct IO_PCA9956B *const P)
{
    uint8_t eflags[6];
    uint8_t reg;

    if (!deviceMode2Status(P) || !P->chError)
    {
        return;
    }

    // Retrieve EFLAGS[0-5]
    reg = PCA9956B_AUTOINC_EFLAG0;

    PACKET_SendTo   (P->packet, &VARIANT_SpawnUint(P->i2cAddr));
    PACKET_Bidir    (P->packet, &reg, 1, eflags, sizeof(eflags));

    if (!PACKET_ERROR_LOG_Generic (P->packet))
    {
        // EFLAGS[0-5] to channel status. According to PCA9956B datasheet, 
        // only active channel status (Current > 8, PWM > 8) should be
        // processed; reported status on inactive channels is invalid.
        for (uint8_t i = 0; i < 6; ++i)
        {
            channelStatus (P, i*4 + 0, (eflags[i] & 0x03) >> 0);
            channelStatus (P, i*4 + 1, (eflags[i] & 0x0C) >> 2);
            channelStatus (P, i*4 + 2, (eflags[i] & 0x30) >> 4);
            channelStatus (P, i*4 + 3, (eflags[i] & 0xC0) >> 6);
        }

        // MODE2 register: Clear channel error (CLRERR = 0x16)
        eflags[0] = PCA9956B_MODE2;
        eflags[1] = 0x16;

        PACKET_SendTo   (P->packet, &VARIANT_SpawnUint(P->i2cAddr));
        PACKET_Send     (P->packet, eflags, 2);
    }
}


static void hardwareInit (struct IO *const Io)
{
    struct IO_PCA9956B *const P = (struct IO_PCA9956B *) Io;

    // Check device presence
    if (!deviceMode2Status (P))
    {
        BOARD_AssertInitialized (false);
    }

    // Initial state
    memset (P->chIref, 0, sizeof(P->chIref));
    memset (P->chPwm, 0, sizeof(P->chPwm));

    P->update = PCA9956B_UPDATE_ALL;

    update (Io);
}


static void updateChannels (struct IO_PCA9956B *const P,
                            const uint8_t I2cReg,
                            uint8_t chData[IO_PCA9956B_CHANNEL_COUNT + 1])
{
    chData[0] = I2cReg;

    PACKET_SendTo   (P->packet, &VARIANT_SpawnUint(P->i2cAddr));
    PACKET_Send     (P->packet, chData, 1 + IO_PCA9956B_CHANNEL_COUNT);

    PACKET_ERROR_LOG_Generic (P->packet);
}


static void update (struct IO *const Io)
{
    struct IO_PCA9956B *const P = (struct IO_PCA9956B *) Io;

    if (P->update & PCA9956B_UPDATE_IREF)
    {
        updateChannels (P, PCA9956B_AUTOINC_IREF0, P->chIref);
    }

    if (P->update & PCA9956B_UPDATE_PWM)
    {
        updateChannels (P, PCA9956B_AUTOINC_PWM0, P->chPwm);
    }

    P->update = 0;

    deviceFullStatus (P);
}


static IO_Count availableInputs (struct IO *const Io,
                                 const enum IO_Type IoType,
                                 const uint32_t InputSource)
{
    (void) Io;
    (void) InputSource;

    // This driver handles no "analog" inputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    } 

    return IO_PCA9956B_INB__COUNT;
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

    return IO_PCA9956B_OUTR__COUNT;
}


static uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                          const uint16_t Index, const uint32_t InputSource)
{
    (void) InputSource;
    
    struct IO_PCA9956B *const P = (struct IO_PCA9956B *) Io;

    if (IoType == IO_Type_Bit)
    {
        BOARD_AssertParams (Index < IO_PCA9956B_INB__COUNT);

        switch (Index)
        {
            case IO_PCA9956B_INB_OVERTEMP:
                return P->overtemp;

            case IO_PCA9956B_INB_CHANNEL_ERROR:
                return P->chError;

            default:
                BOARD_AssertUnexpectedValue (Io, Index);
                break;
        }
    }
    else if (IoType == IO_Type_Range)
    {
        BOARD_AssertParams (Index < IO_PCA9956B_INR__COUNT);

        switch (Index)
        {
            case IO_PCA9956B_INR_CHANNELS_SHORTED_BITFIELD:
                return P->channelsShorted;

            case IO_PCA9956B_INR_CHANNELS_OPEN_BITFIELD:
                return P->channelsOpen;

            default:
                BOARD_AssertUnexpectedValue (Io, Index);
                break;
        }
    }

    BOARD_AssertState (false);
    return 0;
}


static void setOutput (struct IO *const Io,
                       const enum IO_Type IoType,
                       const uint16_t Index, const uint32_t OutputSource,
                       const uint32_t Value)
{
    BOARD_AssertParams (IoType == IO_Type_Range &&
                         Index < IO_PCA9956B_OUTR__COUNT &&
                         Value <= 0xFF);

    (void) OutputSource;

    struct IO_PCA9956B *const P = (struct IO_PCA9956B *) Io;

    if (Index <= IO_PCA9956B_OUTR__CH_IREF_END)
    {
        // chIref[0] is the i2c registry to write to.
        P->chIref[Index + 1] = Value;
        P->update |= PCA9956B_UPDATE_IREF;
    }
    else if (Index >= IO_PCA9956B_OUTR__CH_PWM_BEGIN &&
             Index <= IO_PCA9956B_OUTR__CH_PWM_END)
    {
        // chPwm[0] is the i2c registry to write to.
        P->chPwm[Index + 1 - IO_PCA9956B_OUTR__CH_PWM_BEGIN] = Value;
        P->update |= PCA9956B_UPDATE_PWM;
    }
    else
    {
        BOARD_AssertUnexpectedValue (Io, Index);
    }
}


static const char * inputName (struct IO *const Io,
                               const enum IO_Type IoType,
                               const uint16_t Index)
{
    (void) Io;

    if (IoType == IO_Type_Bit)
    {
        BOARD_AssertParams (Index < IO_PCA9956B_INB__COUNT);
        return s_InputNamesBit[Index];
    }

    BOARD_AssertParams (Index < IO_PCA9956B_INR__COUNT);
    return s_InputNamesRange[Index];
}


static const char * outputName (struct IO *const Io,
                                const enum IO_Type IoType,
                                const uint16_t Index)
{
    BOARD_AssertParams (IoType == IO_Type_Range &&
                        Index < IO_PCA9956B_OUTR__COUNT);

    (void) Io;

    return s_OutputNamesRange[Index];
}

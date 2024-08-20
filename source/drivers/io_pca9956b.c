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
#include "embedul.ar/source/core/device/stream/check.h"


#define PCA9956B_UPDATE_IREF                0x01
#define PCA9956B_UPDATE_PWM                 0x02
#define PCA9956B_UPDATE_ALL                 0xFF

#define PCA9956B_CH_STATUS_NORMAL           0x00
#define PCA9956B_CH_STATUS_SHORT            0x01
#define PCA9956B_CH_STATUS_OPEN             0x02

#define PCA9956B_I2C_TIMEOUT                8

// Mode 2 register.
#define PCA9956B_MODE2                      0x01
// Auto increment starting from IREF0.
#define PCA9956B_AUTOINC_IREF0              (0x80 | 0x22)
// Auto increment starting from PWM0.
#define PCA9956B_AUTOINC_PWM0               (0x80 | 0x0A)
// Auto increment starting from EFLAG0.
#define PCA9956B_AUTOINC_EFLAG0             (0x80 | 0x41)


static const char * s_InputBitNames[IO_PCA9956B_INB__COUNT] =
{
    [IO_PCA9956B_INB_OVERTEMP]      = "overtemp",
    [IO_PCA9956B_INB_CHANNEL_ERROR] = "channel error"
};


static const char * s_InputRangeNames[IO_PCA9956B_INR__COUNT] =
{
    [IO_PCA9956B_INR_CHANNELS_SHORTED_BITFIELD] = "channels shorted",
    [IO_PCA9956B_INR_CHANNELS_OPEN_BITFIELD]    = "channels open"
};


static const char * s_OutputRangeNames[IO_PCA9956B_OUTR__COUNT] =
{
    [IO_PCA9956B_OUTR_CH0_IREF]     = "Ch0 IRef",
    [IO_PCA9956B_OUTR_CH1_IREF]     = "Ch1 IRef",
    [IO_PCA9956B_OUTR_CH2_IREF]     = "Ch2 IRef",
    [IO_PCA9956B_OUTR_CH3_IREF]     = "Ch3 IRef",
    [IO_PCA9956B_OUTR_CH4_IREF]     = "Ch4 IRef",
    [IO_PCA9956B_OUTR_CH5_IREF]     = "Ch5 IRef",
    [IO_PCA9956B_OUTR_CH6_IREF]     = "Ch6 IRef",
    [IO_PCA9956B_OUTR_CH7_IREF]     = "Ch7 IRef",
    [IO_PCA9956B_OUTR_CH8_IREF]     = "Ch8 IRef",
    [IO_PCA9956B_OUTR_CH9_IREF]     = "Ch9 IRef", 
    [IO_PCA9956B_OUTR_CH10_IREF]    = "Ch10 IRef",
    [IO_PCA9956B_OUTR_CH11_IREF]    = "Ch11 IRef",
    [IO_PCA9956B_OUTR_CH12_IREF]    = "Ch12 IRef",
    [IO_PCA9956B_OUTR_CH13_IREF]    = "Ch13 IRef",
    [IO_PCA9956B_OUTR_CH14_IREF]    = "Ch14 IRef",
    [IO_PCA9956B_OUTR_CH15_IREF]    = "Ch15 IRef",
    [IO_PCA9956B_OUTR_CH16_IREF]    = "Ch16 IRef",
    [IO_PCA9956B_OUTR_CH17_IREF]    = "Ch17 IRef",
    [IO_PCA9956B_OUTR_CH18_IREF]    = "Ch18 IRef",
    [IO_PCA9956B_OUTR_CH19_IREF]    = "Ch19 IRef", 
    [IO_PCA9956B_OUTR_CH20_IREF]    = "Ch20 IRef",
    [IO_PCA9956B_OUTR_CH21_IREF]    = "Ch21 IRef",
    [IO_PCA9956B_OUTR_CH22_IREF]    = "Ch22 IRef",
    [IO_PCA9956B_OUTR_CH23_IREF]    = "Ch23 IRef",
    [IO_PCA9956B_OUTR_CH0_PWM]      = "Ch0 PWM",
    [IO_PCA9956B_OUTR_CH1_PWM]      = "Ch1 PWM",
    [IO_PCA9956B_OUTR_CH2_PWM]      = "Ch2 PWM",
    [IO_PCA9956B_OUTR_CH3_PWM]      = "Ch3 PWM",
    [IO_PCA9956B_OUTR_CH4_PWM]      = "Ch4 PWM",
    [IO_PCA9956B_OUTR_CH5_PWM]      = "Ch5 PWM",
    [IO_PCA9956B_OUTR_CH6_PWM]      = "Ch6 PWM",
    [IO_PCA9956B_OUTR_CH7_PWM]      = "Ch7 PWM",
    [IO_PCA9956B_OUTR_CH8_PWM]      = "Ch8 PWM",
    [IO_PCA9956B_OUTR_CH9_PWM]      = "Ch9 PWM", 
    [IO_PCA9956B_OUTR_CH10_PWM]     = "Ch10 PWM",
    [IO_PCA9956B_OUTR_CH11_PWM]     = "Ch11 PWM",
    [IO_PCA9956B_OUTR_CH12_PWM]     = "Ch12 PWM",
    [IO_PCA9956B_OUTR_CH13_PWM]     = "Ch13 PWM",
    [IO_PCA9956B_OUTR_CH14_PWM]     = "Ch14 PWM",
    [IO_PCA9956B_OUTR_CH15_PWM]     = "Ch15 PWM",
    [IO_PCA9956B_OUTR_CH16_PWM]     = "Ch16 PWM",
    [IO_PCA9956B_OUTR_CH17_PWM]     = "Ch17 PWM",
    [IO_PCA9956B_OUTR_CH18_PWM]     = "Ch18 PWM",
    [IO_PCA9956B_OUTR_CH19_PWM]     = "Ch19 PWM", 
    [IO_PCA9956B_OUTR_CH20_PWM]     = "Ch20 PWM",
    [IO_PCA9956B_OUTR_CH21_PWM]     = "Ch21 PWM",
    [IO_PCA9956B_OUTR_CH22_PWM]     = "Ch22 PWM",
    [IO_PCA9956B_OUTR_CH23_PWM]     = "Ch23 PWM"
};


static void         hardwareInit        (struct IO *const Io);
static void         update              (struct IO *const Io);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static void         setOutput           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port,
                                         const uint32_t Value);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);
static const char * outputName          (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_PCA9956B_IFACE =
{
    IO_IFACE_DECLARE("pca9956B 24-channel led driver", PCA9956B),
    .HardwareInit       = hardwareInit,
    .Update             = update,
    .GetInput           = getInput,
    .SetOutput          = setOutput,
    .InputName          = inputName,
    .OutputName         = outputName
};


void IO_PCA9956B_Init (struct IO_PCA9956B *const P,
                       const enum COMM_Device ComDevice, const uint8_t I2cAddr)
{
    BOARD_AssertParams (P);

    DEVICE_IMPLEMENTATION_Clear (P);

    IO_INIT_STATIC_PORT_INFO (P, PCA9956B);

    P->stream   = COMM_GetDevice (ComDevice);
    P->i2cAddr  = I2cAddr;

    // Update at 60 Hz.
    IO_Init ((struct IO *)P, &IO_PCA9956B_IFACE, P->portInfo, 15);
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

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)P, 0);

    MIO_Map (MIO_Dir_Input, INPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
             INPUT_PROFILE_LIGHTDEV_Range_ChannelsShorted(DeviceOffset),
             IO_PCA9956B_INR_CHANNELS_SHORTED_BITFIELD);

    MIO_Map (MIO_Dir_Input, INPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
             INPUT_PROFILE_LIGHTDEV_Range_ChannelsOpen(DeviceOffset),
             IO_PCA9956B_INR_CHANNELS_OPEN_BITFIELD);

    MIO_Map (MIO_Dir_Input, INPUT_PROFILE_Group_LIGHTDEV, IO_Type_Bit,
             INPUT_PROFILE_LIGHTDEV_Bit_Overtemp(DeviceOffset),
             IO_PCA9956B_INB_OVERTEMP);

    MIO_Map (MIO_Dir_Input, INPUT_PROFILE_Group_LIGHTDEV, IO_Type_Bit,
             INPUT_PROFILE_LIGHTDEV_Bit_ChannelError(DeviceOffset),
             IO_PCA9956B_INB_CHANNEL_ERROR);


    MIO_RegisterGateway (MIO_Dir_Output, (struct IO *)P, 0);

    const enum OUTPUT_PROFILE_LIGHTDEV_Range IrefStart = 
                    OUTPUT_PROFILE_LIGHTDEV_Range_Iref(ChannelOffset);

    const enum OUTPUT_PROFILE_LIGHTDEV_Range PwmStart =
                    OUTPUT_PROFILE_LIGHTDEV_Range_Pwm(ChannelOffset);

    for (uint32_t i = 0; i < IO_PCA9956B_CHANNEL_COUNT; ++i)
    {
        MIO_Map (MIO_Dir_Output, OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                 IrefStart + i, IO_PCA9956B_OUTR_CH0_IREF + i);
        MIO_Map (MIO_Dir_Output, OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                 PwmStart + i, IO_PCA9956B_OUTR_CH0_PWM + i);
    }
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

    STREAM_ADDRT_EXCHANGE_BUFFERS (P->stream, P->i2cAddr, PCA9956B_I2C_TIMEOUT,
                               &tx, 1, &rx, 1);

    if (!STREAM_CHECK_I2cControllerXferStatus (P->stream))
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

    STREAM_ADDRT_EXCHANGE_BUFFERS (P->stream, P->i2cAddr, PCA9956B_I2C_TIMEOUT,
                               &reg, 1, eflags, sizeof(eflags));

    if (STREAM_CHECK_I2cControllerXferStatus (P->stream))
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

        STREAM_ADDRT_IN_BUFFER (P->stream, P->i2cAddr, PCA9956B_I2C_TIMEOUT,
                                eflags, 2);
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

    STREAM_ADDRT_IN_BUFFER (P->stream, P->i2cAddr, PCA9956B_I2C_TIMEOUT,
                            chData, 1 + IO_PCA9956B_CHANNEL_COUNT);

    STREAM_CHECK_I2cControllerXferStatus (P->stream);
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


static uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                          const IO_Code DriverCode, const IO_Port Port)
{
    (void) Port;
    
    struct IO_PCA9956B *const P = (struct IO_PCA9956B *) Io;

    if (IoType == IO_Type_Bit)
    {
        BOARD_AssertParams (DriverCode < IO_PCA9956B_INB__COUNT);

        switch (DriverCode)
        {
            case IO_PCA9956B_INB_OVERTEMP:
                return P->overtemp;

            case IO_PCA9956B_INB_CHANNEL_ERROR:
                return P->chError;
        }
    }
    else
    {
        BOARD_AssertParams (DriverCode < IO_PCA9956B_INR__COUNT);

        switch (DriverCode)
        {
            case IO_PCA9956B_INR_CHANNELS_SHORTED_BITFIELD:
                return P->channelsShorted;

            case IO_PCA9956B_INR_CHANNELS_OPEN_BITFIELD:
                return P->channelsOpen;
        }
    }

    BOARD_AssertState (false);
    return 0;
}


static void setOutput (struct IO *const Io,
                       const enum IO_Type IoType,
                       const IO_Code DriverCode, const IO_Port Port,
                       const uint32_t Value)
{
    BOARD_AssertParams (Value <= 0xFF);

    (void) IoType;
    (void) Port;

    struct IO_PCA9956B *const P = (struct IO_PCA9956B *) Io;

    if (DriverCode <= IO_PCA9956B_OUTR__CH_IREF_END)
    {
        // chIref[0] is the i2c registry to write to.
        P->chIref[DriverCode + 1] = Value;
        P->update |= PCA9956B_UPDATE_IREF;
    }
    else if (DriverCode >= IO_PCA9956B_OUTR__CH_PWM_BEGIN &&
             DriverCode <= IO_PCA9956B_OUTR__CH_PWM_END)
    {
        // chPwm[0] is the i2c registry to write to.
        P->chPwm[DriverCode + 1 - IO_PCA9956B_OUTR__CH_PWM_BEGIN] = Value;
        P->update |= PCA9956B_UPDATE_PWM;
    }
    else
    {
        BOARD_AssertUnexpectedValue (Io, DriverCode);
    }
}


static const char * inputName (struct IO *const Io,
                               const enum IO_Type IoType,
                               const IO_Code DriverCode)
{
    (void) Io;

    if (IoType == IO_Type_Bit)
    {
        return s_InputBitNames[DriverCode];
    }

    return s_InputRangeNames[DriverCode];
}


static const char * outputName (struct IO *const Io,
                                const enum IO_Type IoType,
                                const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_OutputRangeNames[DriverCode];
}

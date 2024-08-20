/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [IO driver] dual genesys gamepads on pca9673.

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

#include "embedul.ar/source/drivers/io_dual_genesis_pca9673.h"
#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/device/stream/check.h"


#define IO_DUAL_GENESIS_I2C_TIMEOUT     8


// PCA9673 register position mask for each connected genesis signal + presence
// led. One register for each gamepad: gamepad 1 (P00/P07), gamepad 2 (P10/P17)
#define CHCFG_1TH   0x10
#define CHCFG_1TR   0x80
#define CHCFG_1TL   0x04
#define CHCFG_1D3   0x40
#define CHCFG_1D2   0x20
#define CHCFG_1D1   0x08
#define CHCFG_1D0   0x02
#define CHCFG_1OK   0x01

#define CHCFG_2TH   0x08
#define CHCFG_2TR   0x40
#define CHCFG_2TL   0x02
#define CHCFG_2D3   0x20
#define CHCFG_2D2   0x10
#define CHCFG_2D1   0x04
#define CHCFG_2D0   0x01
#define CHCFG_2OK   0x80


#define CH1(_ch,_btn)   ((G->rxData[0] & CHCFG_1 ## _ch)? \
                                0 : 1 << IO_DUAL_GENESIS_INB_ ## _btn)

#define CH2(_ch,_btn)   ((G->rxData[1] & CHCFG_2 ## _ch)? \
                                0 : 1 << IO_DUAL_GENESIS_INB_ ## _btn)


static const char *
s_InputNamesBit[IO_DUAL_GENESIS_INB__6Buttons_COUNT] =
{
    [IO_DUAL_GENESIS_INB_Right]     = "Right",
    [IO_DUAL_GENESIS_INB_Left]      = "Left",
    [IO_DUAL_GENESIS_INB_Down]      = "Down",
    [IO_DUAL_GENESIS_INB_Up]        = "Up",
    [IO_DUAL_GENESIS_INB_Start]     = "Start",
    [IO_DUAL_GENESIS_INB_A]         = "A",
    [IO_DUAL_GENESIS_INB_B]         = "B",
    [IO_DUAL_GENESIS_INB_C]         = "C",
    [IO_DUAL_GENESIS_INB_Mode]      = "Mode",
    [IO_DUAL_GENESIS_INB_X]         = "X",
    [IO_DUAL_GENESIS_INB_Y]         = "Y",
    [IO_DUAL_GENESIS_INB_Z]         = "Z"
};


static void         hardwareInit        (struct IO *const Io);
static void         update              (struct IO *const Io);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode,
                                         const IO_Port Port);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const IO_Code DriverCode);


static const struct IO_IFACE IO_DUAL_GENESIS_PCA9673_IFACE =
{
    IO_IFACE_DECLARE("dual genesis on pca9673", DUAL_GENESIS),
    .HardwareInit   = hardwareInit,
    .Update         = update,
    .GetInput       = getInput,
    .InputName      = inputName
};


void IO_DUAL_GENESIS_PCA9673_Init (struct IO_DUAL_GENESIS_PCA9673 *const G,
                                   const enum COMM_Device ComDevice,
                                   const uint8_t I2cAddr)
{
    BOARD_AssertParams (G);

    DEVICE_IMPLEMENTATION_Clear (G);

    // This drivers supports hot-plugged devices. Available input/outputs
    // is zero initially.

    G->stream   = COMM_GetDevice (ComDevice);
    G->i2cAddr  = I2cAddr;

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)G, &IO_DUAL_GENESIS_PCA9673_IFACE, G->portInfo, 15);
}


void IO_DUAL_GENESIS_PCA9673_Attach (struct IO_DUAL_GENESIS_PCA9673 *const G)
{
    BOARD_AssertParams (G);

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)G, 0);

    MIO_MAP_INPUT_BIT (GP1, Right, IO_DUAL_GENESIS_INB_Right);
    MIO_MAP_INPUT_BIT (GP1, Left, IO_DUAL_GENESIS_INB_Left);
    MIO_MAP_INPUT_BIT (GP1, Down, IO_DUAL_GENESIS_INB_Down);
    MIO_MAP_INPUT_BIT (GP1, Up, IO_DUAL_GENESIS_INB_Up);
    MIO_MAP_INPUT_BIT (GP1, Start, IO_DUAL_GENESIS_INB_Start);
    MIO_MAP_INPUT_BIT (GP1, Select, IO_DUAL_GENESIS_INB_Mode);
    MIO_MAP_INPUT_BIT (GP1, A, IO_DUAL_GENESIS_INB_A);
    MIO_MAP_INPUT_BIT (GP1, B, IO_DUAL_GENESIS_INB_B);
    MIO_MAP_INPUT_BIT (GP1, C, IO_DUAL_GENESIS_INB_C);
    MIO_MAP_INPUT_BIT (GP1, X, IO_DUAL_GENESIS_INB_X);
    MIO_MAP_INPUT_BIT (GP1, Y, IO_DUAL_GENESIS_INB_Y);
    MIO_MAP_INPUT_BIT (GP1, Z, IO_DUAL_GENESIS_INB_Z);

    MIO_RegisterGateway (MIO_Dir_Input, (struct IO *)G, 1);

    MIO_MAP_INPUT_BIT (GP2, Right, IO_DUAL_GENESIS_INB_Right);
    MIO_MAP_INPUT_BIT (GP2, Left, IO_DUAL_GENESIS_INB_Left);
    MIO_MAP_INPUT_BIT (GP2, Down, IO_DUAL_GENESIS_INB_Down);
    MIO_MAP_INPUT_BIT (GP2, Up, IO_DUAL_GENESIS_INB_Up);
    MIO_MAP_INPUT_BIT (GP2, Start, IO_DUAL_GENESIS_INB_Start);
    MIO_MAP_INPUT_BIT (GP2, Select, IO_DUAL_GENESIS_INB_Mode);
    MIO_MAP_INPUT_BIT (GP2, A, IO_DUAL_GENESIS_INB_A);
    MIO_MAP_INPUT_BIT (GP2, B, IO_DUAL_GENESIS_INB_B);
    MIO_MAP_INPUT_BIT (GP2, C, IO_DUAL_GENESIS_INB_C);
    MIO_MAP_INPUT_BIT (GP2, X, IO_DUAL_GENESIS_INB_X);
    MIO_MAP_INPUT_BIT (GP2, Y, IO_DUAL_GENESIS_INB_Y);
    MIO_MAP_INPUT_BIT (GP2, Z, IO_DUAL_GENESIS_INB_Z);
}


void hardwareInit (struct IO *const Io)
{
    struct IO_DUAL_GENESIS_PCA9673 *const G =
                            (struct IO_DUAL_GENESIS_PCA9673 *) Io;

    // PCA9673 initial configuration.
    //
    // Input channels must be HIGH to properly work as inputs when externally
    // driven low. TH is a clock output to each gamepad, and must be LOW at the
    // beginning of a new reading cycle (G->state == 1).
    G->txData[0] = 0xFF & ~CHCFG_1TH;
    G->txData[1] = 0xFF & ~CHCFG_2TH;

    STREAM_ADDRT_IN_BUFFER (G->stream, &VARIANT_SpawnUint(G->i2cAddr),
                            IO_DUAL_GENESIS_I2C_TIMEOUT, G->txData, 2);

    if (!STREAM_CHECK_I2cControllerXferStatus (G->stream))
    {
        BOARD_AssertInitialized (false);
    }
}


/*
    Buttons are active-LOW.

    Cycle #     1--------       2--------       3--------
    State #     0       1       2       3       4       5
    ---------------------------------------------------------
    TH  in      1       0       1       0       1       0
    ---------------------------------------------------------
    TR  out     C       Start   C       Start   C       Start
    TL  out     B       A       B       A       B       A
    D3  out     Right   0       Right   0       Mode    ---
    D2  out     Left    0       Left    0       X       ---
    D1  out     Down    Down    Down    0       Y       ---
    D0  out     Up      Up      Up      0       Z       ---
    ---------------------------------------------------------

    If state is even, output LOW to the TH pin. If it's odd, output HIGH
    A rising edge reports C,B,etc. A falling edge reports Start,A,etc.

    Pressing "Mode" while powering up a 6-button gamepad will make it behave as
    a 3-button one (state counting will reset after the 2nd cycle instead of
    the 8th).

    The gamepad controller will reset its current state counting if a rising
    edge is undetected within 1.5 ms.

    How to detect a 3 or 6-button gamepad:

    State 1, D3=D2=0 (Right & Left pressed): gamepad detected. 3 or 6-buttons.
    State 3, D3=D2=D1=D0=0 (4-way pressed at the same time): 6-button gamepad.
        If D1=D0 != 0, definitely a 3-button gamepad on its 2rd repeating cycle.
        Else
    State 4, D3,D2,D1,D0: new 6-button status (Mode,X,Y,Z).
    State 5 will set TH LOW, right to the end of a Cycle.
*/
static bool processHardwareState (struct IO_DUAL_GENESIS_PCA9673 *const G,
                                  const uint32_t State)
{
    if (State > 5)
    {
        return false;
    }

    G->txData[0] = 0xFF;
    G->txData[1] = 0xFF;

    // Even state = TH HIGH (keep as it is)
    // Odd state = TH LOW (remove clock high bit).
    if (State % 2)
    {
        G->txData[0] &= ~CHCFG_1TH;
        G->txData[1] &= ~CHCFG_2TH;
    }

    IO_Count *const Gp1Count = &G->portInfo[0].inAvailable[IO_Type_Bit];
    IO_Count *const Gp2Count = &G->portInfo[1].inAvailable[IO_Type_Bit];

    // Turn on/off gamepads detection led (OK LOW). Note that for the first
    // 2 states, gp#ButtonsSupported will keep the last state processed.
    G->txData[0] &= (* Gp1Count)? ~CHCFG_1OK : 0xFF;
    G->txData[1] &= (* Gp2Count)? ~CHCFG_2OK : 0xFF;

    // Keep inputs HIGH, flip TH, and get input status for both controllers.
    STREAM_ADDRT_EXCHANGE_BUFFERS (G->stream, &VARIANT_SpawnUint(G->i2cAddr),
                               IO_DUAL_GENESIS_I2C_TIMEOUT,
                               G->txData, 2, G->rxData, 2);

    if (!STREAM_CHECK_I2cControllerXferStatus (G->stream))
    {
        return false;
    }

    #ifdef IO_DUAL_GENESIS_PCA9673_I2C_LOG_GP1
    // Log Gamepad 1 signals for debugging purposes
    G->log[state] = G->rxData[0];
    #endif

    // Button status will be sampled at states number 2, 3, and 7 for a detected
    // 6-button gamepad.
    switch (State)
    {
        case 0:
            G->gp1Data = 0;
            G->gp2Data = 0;
            break;

        case 1:
            G->gp1Data |= CH1(TR, Start) | CH1(TL, A) | CH1(D1, Down) |
                          CH1(D0, Up);
            G->gp2Data |= CH2(TR, Start) | CH2(TL, A) | CH2(D1, Down) |
                          CH2(D0, Up);

            // State 2, D3=D2=0 (Right & Left pressed): gamepad detected. 3
            // or 6-buttons. Assume 3 by now.
            (* Gp1Count) = (CH1(D3, Right) && CH1(D2, Left))?
                                IO_DUAL_GENESIS_INB__3Buttons_COUNT : 0;
            (* Gp2Count) = (CH2(D3, Right) && CH2(D2, Left))?
                                IO_DUAL_GENESIS_INB__3Buttons_COUNT : 0;
            break;

        case 2:
            G->gp1Data |= CH1(TR, C)    | CH1(TL, B)    | CH1(D3, Right) |
                          CH1(D2, Left) | CH1(D1, Down) | CH1(D0, Up);
            G->gp2Data |= CH2(TR, C)    | CH2(TL, B)    | CH2(D3, Right) |
                          CH2(D2, Left) | CH2(D1, Down) | CH2(D0, Up);
            break;

        case 3:
            if ((* Gp1Count) && CH1(D1, Down) && CH1(D0, Up))
            {
                // 6-buttons gamepad
                (* Gp1Count) = IO_DUAL_GENESIS_INB__6Buttons_COUNT;
            }

            if ((* Gp2Count) && CH2(D1, Down) && CH2(D0, Up))
            {
                // 6-buttons gamepad
                (* Gp2Count) = IO_DUAL_GENESIS_INB__6Buttons_COUNT;
            }
            break;

        case 4:
            if ((* Gp1Count) == IO_DUAL_GENESIS_INB__6Buttons_COUNT)
            {
                G->gp1Data |= CH1(D3, Mode) | CH1(D2, X) | CH1(D1, Y) |
                              CH1(D0, Z);
            }

            if ((* Gp2Count) == IO_DUAL_GENESIS_INB__6Buttons_COUNT)
            {
                G->gp2Data |= CH2(D3, Mode) | CH2(D2, X) | CH2(D1, Y) |
                              CH2(D0, Z);
            }
            break;

        case 5:
            // Extra I2C transaction to set a LOW on TH.
            break;
    }

    return true;
}


void update (struct IO *const Io)
{
    struct IO_DUAL_GENESIS_PCA9673 *const G =
                            (struct IO_DUAL_GENESIS_PCA9673 *) Io;

    // Update both gamepads
    for (uint32_t state = 0; state < 6; ++state)
    {
        if (!processHardwareState (G, state))
        {
            break;
        }
    }
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const IO_Code DriverCode, const IO_Port Port)
{
    (void) IoType;

    struct IO_DUAL_GENESIS_PCA9673 *const G =
                            (struct IO_DUAL_GENESIS_PCA9673 *) Io;

    return (Port == 0)? G->gp1Data & (1 << DriverCode) :
                        G->gp2Data & (1 << DriverCode);
}


const char * inputName (struct IO *const Io, const enum IO_Type IoType,
                        const IO_Code DriverCode)
{
    (void) Io;
    (void) IoType;

    return s_InputNamesBit[DriverCode];
}

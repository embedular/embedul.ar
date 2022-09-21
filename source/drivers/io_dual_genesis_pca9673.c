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
#include "embedul.ar/source/core/device/packet/error_log.h"


#define I2C_XFER_STATUS_ITEM_STR        "i2c transfer status"
#define HW_STATE_ITEM_STR               "hardware state"


static const char *
s_InputNamesBit[IO_DUAL_GENESIS_INB__6Buttons_COUNT] =
{
    "→", "←", "↓", "↑", "start", "a", "b", "c",
    "mode", "x", "y", "z"
};


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


static void         hardwareInit        (struct IO *const Io);
static void         update              (struct IO *const Io);
static IO_Count
                    availableInputs     (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint32_t InputSource);
static uint32_t     getInput            (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index,
                                         const uint32_t InputSource);
static const char * inputName           (struct IO *const Io,
                                         const enum IO_Type IoType,
                                         const uint16_t Index);


static const struct IO_IFACE IO_DUAL_GENESIS_PCA9673_IFACE =
{
    .Description        = "dual genesis on pca9673",
    .HardwareInit       = hardwareInit,
    .Update             = update,
    .AvailableInputs    = availableInputs,
    .AvailableOutputs   = UNSUPPORTED,
    .GetInput           = getInput,
    .SetOutput          = UNSUPPORTED,
    .InputName          = inputName,
    .OutputName         = UNSUPPORTED
};


void IO_DUAL_GENESIS_PCA9673_Init (struct IO_DUAL_GENESIS_PCA9673 *const G,
                                   const enum COMM_Packet Com,
                                   const uint8_t I2cAddr)
{
    BOARD_AssertParams (G);

    DEVICE_IMPLEMENTATION_Clear (G);

    G->packet   = COMM_GetPacket (Com);
    G->i2cAddr  = I2cAddr;

    // Update once per frame (~60 Hz).
    IO_Init ((struct IO *)G, &IO_DUAL_GENESIS_PCA9673_IFACE, 15);
}


void IO_DUAL_GENESIS_PCA9673_Attach (
                            struct IO_DUAL_GENESIS_PCA9673 *const G)
{
    BOARD_AssertParams (G);

    INPUT_SetDevice ((struct IO *)G, 0);

    INPUT_MapBit (INPUT_Bit_P1Right, IO_DUAL_GENESIS_INB_Right);
    INPUT_MapBit (INPUT_Bit_P1Left, IO_DUAL_GENESIS_INB_Left);
    INPUT_MapBit (INPUT_Bit_P1Down, IO_DUAL_GENESIS_INB_Down);
    INPUT_MapBit (INPUT_Bit_P1Up, IO_DUAL_GENESIS_INB_Up);
    INPUT_MapBit (INPUT_Bit_P1Start, IO_DUAL_GENESIS_INB_Start);
    INPUT_MapBit (INPUT_Bit_P1Select, IO_DUAL_GENESIS_INB_Mode);
    INPUT_MapBit (INPUT_Bit_P1A, IO_DUAL_GENESIS_INB_A);
    INPUT_MapBit (INPUT_Bit_P1B, IO_DUAL_GENESIS_INB_B);
    INPUT_MapBit (INPUT_Bit_P1C, IO_DUAL_GENESIS_INB_C);
    INPUT_MapBit (INPUT_Bit_P1X, IO_DUAL_GENESIS_INB_X);
    INPUT_MapBit (INPUT_Bit_P1Y, IO_DUAL_GENESIS_INB_Y);
    INPUT_MapBit (INPUT_Bit_P1Z, IO_DUAL_GENESIS_INB_Z);


    INPUT_SetDevice ((struct IO *)G, 1);

    INPUT_MapBit (INPUT_Bit_P2Right, IO_DUAL_GENESIS_INB_Right);
    INPUT_MapBit (INPUT_Bit_P2Left, IO_DUAL_GENESIS_INB_Left);
    INPUT_MapBit (INPUT_Bit_P2Down, IO_DUAL_GENESIS_INB_Down);
    INPUT_MapBit (INPUT_Bit_P2Up, IO_DUAL_GENESIS_INB_Up);
    INPUT_MapBit (INPUT_Bit_P2Start, IO_DUAL_GENESIS_INB_Start);
    INPUT_MapBit (INPUT_Bit_P2Select, IO_DUAL_GENESIS_INB_Mode);
    INPUT_MapBit (INPUT_Bit_P2A, IO_DUAL_GENESIS_INB_A);
    INPUT_MapBit (INPUT_Bit_P2B, IO_DUAL_GENESIS_INB_B);
    INPUT_MapBit (INPUT_Bit_P2C, IO_DUAL_GENESIS_INB_C);
    INPUT_MapBit (INPUT_Bit_P2X, IO_DUAL_GENESIS_INB_X);
    INPUT_MapBit (INPUT_Bit_P2Y, IO_DUAL_GENESIS_INB_Y);
    INPUT_MapBit (INPUT_Bit_P2Z, IO_DUAL_GENESIS_INB_Z);
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

    PACKET_SendTo   (G->packet, &VARIANT_SpawnUint(G->i2cAddr));
    PACKET_Send     (G->packet, G->txData, 2);

    if (PACKET_ERROR_LOG_nAck (G->packet))
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

#define CH1(_ch,_btn) \
                ((G->rxData[0] & CHCFG_1 ## _ch)? \
                    0 : 1 << IO_DUAL_GENESIS_INB_ ## _btn)

#define CH2(_ch,_btn) \
                ((G->rxData[1] & CHCFG_2 ## _ch)? \
                    0 : 1 << IO_DUAL_GENESIS_INB_ ## _btn)


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

    // Turn on/off gamepads detection led (OK LOW). Note that for the first
    // 2 states, gp#ButtonsSupported will keep the last state processed.
    G->txData[0] &= G->gp1AvailIn? ~CHCFG_1OK : 0xFF;
    G->txData[1] &= G->gp2AvailIn? ~CHCFG_2OK : 0xFF;

    // Keep inputs HIGH, flip TH, and get input status for both controllers.
    PACKET_SendTo   (G->packet, &VARIANT_SpawnUint(G->i2cAddr));
    PACKET_Bidir    (G->packet, G->txData, 2, G->rxData, 2);

    if (PACKET_ERROR_LOG_Generic (G->packet))
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
            G->gp1AvailIn = (CH1(D3, Right) && CH1(D2, Left))?
                                IO_DUAL_GENESIS_INB__3Buttons_COUNT : 0;
            G->gp2AvailIn = (CH2(D3, Right) && CH2(D2, Left))?
                                IO_DUAL_GENESIS_INB__3Buttons_COUNT : 0;
            break;

        case 2:
            G->gp1Data |= CH1(TR, C)    | CH1(TL, B)    | CH1(D3, Right) |
                          CH1(D2, Left) | CH1(D1, Down) | CH1(D0, Up);
            G->gp2Data |= CH2(TR, C)    | CH2(TL, B)    | CH2(D3, Right) |
                          CH2(D2, Left) | CH2(D1, Down) | CH2(D0, Up);
            break;

        case 3:
            if (G->gp1AvailIn && CH1(D1, Down) && CH1(D0, Up))
            {
                // 6-buttons gamepad
                G->gp1AvailIn = IO_DUAL_GENESIS_INB__6Buttons_COUNT;
            }

            if (G->gp2AvailIn && CH2(D1, Down) && CH2(D0, Up))
            {
                // 6-buttons gamepad
                G->gp2AvailIn = IO_DUAL_GENESIS_INB__6Buttons_COUNT;
            }
            break;

        case 4:
            if (G->gp1AvailIn == IO_DUAL_GENESIS_INB__6Buttons_COUNT)
            {
                G->gp1Data |= CH1(D3, Mode) | CH1(D2, X) | CH1(D1, Y) |
                              CH1(D0, Z);
            }

            if (G->gp2AvailIn == IO_DUAL_GENESIS_INB__6Buttons_COUNT)
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


IO_Count availableInputs (struct IO *const Io, const enum IO_Type IoType,
                          const uint32_t InputSource)
{
    BOARD_AssertParams (InputSource < 2);

    struct IO_DUAL_GENESIS_PCA9673 *const G =
                            (struct IO_DUAL_GENESIS_PCA9673 *) Io;

    // Genesis/MegaDrive controllers have no analog inputs
    if (IoType == IO_Type_Range)
    {
        return 0;
    }

    return (InputSource == 0)? G->gp1AvailIn : G->gp2AvailIn;
}


uint32_t getInput (struct IO *const Io, const enum IO_Type IoType,
                   const uint16_t Index, const uint32_t InputSource)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                         Index < IO_DUAL_GENESIS_INB__6Buttons_COUNT &&
                         InputSource < 2);
    
    struct IO_DUAL_GENESIS_PCA9673 *const G =
                            (struct IO_DUAL_GENESIS_PCA9673 *) Io;

    return (InputSource == 0)? G->gp1Data & (1 << Index) :
                               G->gp2Data & (1 << Index);
}


const char * inputName (struct IO *const Io, const enum IO_Type IoType,
                        const uint16_t Index)
{
    BOARD_AssertParams (IoType == IO_Type_Bit &&
                         Index < IO_DUAL_GENESIS_INB__6Buttons_COUNT);

    (void) Io;

    return s_InputNamesBit[Index];
}

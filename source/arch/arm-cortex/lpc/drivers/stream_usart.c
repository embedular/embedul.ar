/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] lpcopen usart.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/drivers/stream_usart.h"
#include "embedul.ar/source/core/device/board.h"


#define STREAM_USART_LPC_IRQ_PRIORITY       1

// Interrupt access to STREAM_USART_LPC instances. One singleton for each UART.
static struct STREAM_USART * s_streamUsart0 = NULL;
static struct STREAM_USART * s_streamUart1  = NULL;
static struct STREAM_USART * s_streamUsart2 = NULL;
static struct STREAM_USART * s_streamUsart3 = NULL;


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


static const struct STREAM_IFACE STREAM_USART_IFACE =
{
    .Description    = "lpcopen usart",
    .HardwareInit   = hardwareInit,
    .Command        = command,
    .DataIn         = dataIn,
    .DataOut        = dataOut
};


static void usartInit (struct STREAM_USART *const U,
                       LPC_USART_T *const Usart, const uint32_t Irq,
                       uint8_t *const InBuffer, const uint32_t InOctets,
                       uint8_t *const OutBuffer, const uint32_t OutOctets,
                       const uint32_t Baud, const uint32_t LcrFlags)
{
    BOARD_AssertParams (U && Usart && 
                         InBuffer && InOctets && OutBuffer && OutOctets &&
                         Baud);

    DEVICE_IMPLEMENTATION_Clear (U);

    CYCLIC_Init (&U->sendCb, InBuffer, InOctets);
    CYCLIC_Init (&U->recvCb, OutBuffer, OutOctets);

    U->usart        = Usart;
    U->irq          = Irq;
    U->baud         = Baud;
    U->lcrFlags     = LcrFlags;
    U->hwFlowCtrl   = false;

    STREAM_Init ((struct STREAM *)U, &STREAM_USART_IFACE);
}


void STREAM_USART0_Init (struct STREAM_USART *const U,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets,
                         const uint32_t Baud, const uint32_t LcrFlags)
{
    BOARD_AssertState (!s_streamUsart0);

    s_streamUsart0 = U;
    usartInit (U, LPC_USART0, USART0_IRQn,
                InBuffer, InOctets, OutBuffer, OutOctets, Baud, LcrFlags);
}


void STREAM_UART1_Init (struct STREAM_USART *const U,
                        uint8_t *const InBuffer, const uint32_t InOctets,
                        uint8_t *const OutBuffer, const uint32_t OutOctets,
                        const uint32_t Baud, const uint32_t LcrFlags)
{
    BOARD_AssertState (!s_streamUart1);

    s_streamUart1 = U;
    usartInit (U, LPC_UART1, UART1_IRQn,
                InBuffer, InOctets, OutBuffer, OutOctets, Baud, LcrFlags);
}


void STREAM_USART2_Init (struct STREAM_USART *const U,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets,
                         const uint32_t Baud, const uint32_t LcrFlags)
{
    BOARD_AssertState (!s_streamUsart2);

    s_streamUsart2 = U;
    usartInit (U, LPC_USART2, USART2_IRQn,
                InBuffer, InOctets, OutBuffer, OutOctets, Baud, LcrFlags);
}


void STREAM_USART3_Init (struct STREAM_USART *const U,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets,
                         const uint32_t Baud, const uint32_t LcrFlags)
{
    BOARD_AssertState (!s_streamUsart3);

    s_streamUsart3 = U;
    usartInit (U, LPC_USART3, USART3_IRQn,
                InBuffer, InOctets, OutBuffer, OutOctets, Baud, LcrFlags);
}


void hardwareInit (struct STREAM *const S)
{
    struct STREAM_USART *const U = (struct STREAM_USART *) S;

    // Initial config.
    // Peripheral mux must have been already configured in LCPOpen Board code.
    Chip_UART_Init          (U->usart);
    Chip_UART_SetBaudFDR    (U->usart, U->baud);
    Chip_UART_ConfigData    (U->usart, U->lcrFlags);
	Chip_UART_SetupFIFOS    (U->usart, 
                                UART_FCR_FIFO_EN |
                                UART_FCR_RX_RS | UART_FCR_TX_RS |
                                UART_FCR_DMAMODE_SEL |
                                UART_FCR_TRG_LEV2);
    Chip_UART_TXEnable      (U->usart);

    // RBR: Receiver Buffer Register. Contains the next received character to be
    // read (DLAB = 0).
    // The UART1 RLS interrupt (IIR[3:1] = 011) is the highest priority
    // interrupt and is set whenever any one of four error conditions occur on
    // the UART1RX input: overrun error (OE), parity error (PE), framing error
    // (FE) and break interrupt (BI). The UART1 Rx error condition that set the
    // interrupt can be observed via LSR[4:1]. The interrupt is cleared upon
    // an LSR read.
    Chip_UART_IntDisable    (U->usart, UART_IER_THREINT);
    Chip_UART_IntEnable     (U->usart, UART_IER_RBRINT | UART_IER_RLSINT);

    NVIC_SetPriority        (U->irq, STREAM_USART_LPC_IRQ_PRIORITY);
    NVIC_EnableIRQ          (U->irq);

    // Default timeouts
    S->inTimeout    = 5000;
    S->outTimeout   = 50;
}


inline static void uartIrqSend (struct STREAM_USART *const U)
{
	/* Fill FIFO until full or until TX ring buffer is empty */
	while (Chip_UART_ReadLineStatus(U->usart) & UART_LSR_THRE &&
           CYCLIC_Elements(&U->sendCb)) 
    {
		Chip_UART_SendByte (U->usart, CYCLIC_OUT_ToOctet(&U->sendCb));
	}
}


inline static void uartIrqRecv (struct STREAM_USART *const U)
{
	/* New data will be ignored if data not popped in time */
	while (Chip_UART_ReadLineStatus(U->usart) & UART_LSR_RDR) 
    {
		const uint8_t Octet = Chip_UART_ReadByte (U->usart);
        CYCLIC_IN_FromOctet (&U->recvCb, Octet);
	}
}


inline static void uartIrq (struct STREAM_USART *const U)
{
    /* Handle transmit interrupt if enabled */
	if (U->usart->IER & UART_IER_THREINT)
    {
        uartIrqSend (U);

        /* Turn off interrupt if the ring buffer is empty */
        if (!CYCLIC_Elements (&U->sendCb))
        {
            /* Shut down transmit */
            Chip_UART_IntDisable (U->usart, UART_IER_THREINT);
        }
	}

	/* Handle receive interrupt */
    uartIrqRecv (U);
}


void UART0_IRQHandler (void)
{
    uartIrq (s_streamUsart0);
}


void UART1_IRQHandler (void)
{
    uartIrq (s_streamUart1);
}


void UART2_IRQHandler (void)
{
    uartIrq (s_streamUsart2);
}


void UART3_IRQHandler (void)
{
    uartIrq (s_streamUsart3);
}


inline static uint32_t checkAvailable (struct CYCLIC *const C,
                                       const uint32_t Octets)
{
    const uint32_t Available = CYCLIC_Available (C);
    return (Octets > Available)? Available : Octets;
}


inline static uint32_t checkContents (struct CYCLIC *const C,
                                      const uint32_t Octets)
{
    const uint32_t Elements = CYCLIC_Elements (C);
    return (Octets > Elements)? Elements : Octets;
}


uint32_t dataIn (struct STREAM *const S, const uint8_t *const Data,
                 const uint32_t Octets)
{
    struct STREAM_USART *const U = (struct STREAM_USART *) S;

    // Disable transmit interrupt
	Chip_UART_IntDisable (U->usart, UART_IER_THREINT);

    uint32_t count = 0;
    // Check if the cyclic buffer can hold 'Octets' or less.
    uint32_t available = checkAvailable (&U->sendCb, Octets);
    if (available)
    {
        CYCLIC_IN_FromBuffer (&U->sendCb, Data, available);
        count = CYCLIC_IN_Count (&U->sendCb);

        // Try to fill the transmit FIFO with contents from the cyclic buffer.
        // If empty, this action will cause to consume 16 bytes at most from the
        // cyclic buffer and the UART to start transmitting.
        uartIrqSend (U);

        // If the cyclic buffer available space was less than 'Octets', then try 
        // again to store more of 'Data'.
        if (count < Octets)
        {
            available = checkAvailable (&U->sendCb, Octets - count);
            if (available)
            {
                CYCLIC_IN_FromBuffer (&U->sendCb, &Data[count], available);
                count += CYCLIC_IN_Count (&U->sendCb);
            }
        }
    }

    // Enable transmit interrupt. This interrupt will be triggered when the 
    // THR FIFO is empty. The interrupt service will then send contents from
    // the cyclic buffer.
    Chip_UART_IntEnable (U->usart, UART_IER_THREINT);

    // Consumed 'Data' octets
    return count;
}


uint32_t dataOut (struct STREAM *const S, uint8_t *const Buffer,
                  const uint32_t Octets)
{
    struct STREAM_USART *const U = (struct STREAM_USART *) S;

    uint32_t        count       = 0;
    const uint32_t  Contents    = checkContents (&U->recvCb, Octets);

    if (Contents)
    {
        // More bytes might be inserted while copying data to Buffer.
        CYCLIC_OUT_ToBuffer (&U->recvCb, Buffer, Contents);
        count = CYCLIC_OUT_Count (&U->recvCb);
    }

    return count;
}


enum DEVICE_CommandResult command (struct STREAM *const S, 
                                   const char *const Name,
                                   struct VARIANT *const Value)
{
    struct STREAM_USART *const U = (struct STREAM_USART *const) S;

    if (DEVICE_COMMAND_CHECK(SET_UART_BAUD, SET_SPEED))
    {
        const uint32_t ReqBaud = (uint32_t) VARIANT_ToUint (Value);
        const uint32_t SetBaud = Chip_UART_SetBaudFDR (U->usart, ReqBaud);

        if (!SetBaud)
        {
            return DEVICE_CommandResult_Failed;
        }

        U->baud = SetBaud;
        VARIANT_SetUint (Value, SetBaud);
    }
    else if (DEVICE_COMMAND_CHECK(EXE_UART_HWFLOW))
    {
        Chip_UART_SetModemControl (U->usart,
                                UART_MCR_AUTO_RTS_EN |
                                UART_MCR_AUTO_CTS_EN);
        U->hwFlowCtrl = true;
    }
    else if (DEVICE_COMMAND_CHECK(EXE_UART_NOFLOW))
    {
        Chip_UART_ClearModemControl (U->usart,
                                    UART_MCR_AUTO_RTS_EN |
                                    UART_MCR_AUTO_CTS_EN);            
        U->hwFlowCtrl = false;
    }
    else
    {
        return DEVICE_CommandResult_NotHandled;
    }

    return DEVICE_CommandResult_Ok;
}

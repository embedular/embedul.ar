/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] stm32 uart.

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

#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/stream_uart.h"
#include "embedul.ar/source/core/device/board.h"


// Interrupt access to STREAM_UART instances. One singleton for each UART.
static struct STREAM_UART * s_streamUart1 = NULL;
static struct STREAM_UART * s_streamUart2 = NULL;
static struct STREAM_UART * s_streamUart3 = NULL;
static struct STREAM_UART * s_streamUart6 = NULL;


// Common IO interface
static void         hardwareInit    (struct STREAM *const S);
static enum DEVICE_CommandResult
                    command         (const void *const D,
                                     const char *const Name,
                                     struct VARIANT *const Value);
static uint32_t     dataIn          (struct STREAM *const S, 
                                     const uint8_t *const Data,
                                     const uint32_t Octets);
static uint32_t     dataOut         (struct STREAM *const S,
                                     uint8_t *const Buffer,
                                     const uint32_t Octets);


static const struct STREAM_IFACE STREAM_UART_IFACE =
{
    .Description    = "stm32 uart",
    .HardwareInit   = hardwareInit,
    .Command        = command,
    .DataIn         = dataIn,
    .DataOut        = dataOut
};


static void uartInit (struct STREAM_UART *const U,
                      UART_HandleTypeDef *const Huart,
                      uint8_t *const InBuffer, const uint32_t InOctets,
                      uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertParams (U && Huart && 
                        InBuffer && InOctets && OutBuffer && OutOctets);

    DEVICE_IMPLEMENTATION_Clear (U);

    U->huart = Huart;

    CYCLIC_Init (&U->sendCb, InBuffer, InOctets);
    CYCLIC_Init (&U->recvCb, OutBuffer, OutOctets);

    STREAM_Init ((struct STREAM *)U, &STREAM_UART_IFACE);
}


void STREAM_UART1_Init (struct STREAM_UART *const U,
                        UART_HandleTypeDef *const Huart,
                        uint8_t *const InBuffer, const uint32_t InOctets,
                        uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUart1);

    s_streamUart1 = U;
    uartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void STREAM_UART2_Init (struct STREAM_UART *const U,
                        UART_HandleTypeDef *const Huart,
                        uint8_t *const InBuffer, const uint32_t InOctets,
                        uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUart2);

    s_streamUart2 = U;
    uartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void STREAM_UART3_Init (struct STREAM_UART *const U,
                        UART_HandleTypeDef *const Huart,
                        uint8_t *const InBuffer, const uint32_t InOctets,
                        uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUart3);

    s_streamUart3 = U;
    uartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void STREAM_UART6_Init (struct STREAM_UART *const U,
                        UART_HandleTypeDef *const Huart,
                        uint8_t *const InBuffer, const uint32_t InOctets,
                        uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUart6);

    s_streamUart6 = U;
    uartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void hardwareInit (struct STREAM *const S)
{
    struct STREAM_UART *const U = (struct STREAM_UART *) S;

    // Disable transmit complete.
    __HAL_UART_DISABLE_IT (U->huart, UART_IT_TC);

    // Disable transmit register empty.
    __HAL_UART_DISABLE_IT (U->huart, UART_IT_TXE);

    // Disable parity error.
    __HAL_UART_DISABLE_IT (U->huart, UART_IT_PE);

    // Disable frame, noise and overrun errors.
    __HAL_UART_DISABLE_IT (U->huart, UART_IT_ERR);

    // Enable receive register not empty.
    __HAL_UART_ENABLE_IT (U->huart, UART_IT_RXNE);
}


inline static void uartIrqSend (struct STREAM_UART *const U, const uint32_t Sr,
                                const uint32_t Cr1)
{
    if ((Sr & USART_SR_TXE) && (Cr1 & USART_CR1_TXEIE) && 
        CYCLIC_Elements(&U->sendCb))
    {
        U->huart->Instance->DR = CYCLIC_OUT_ToOctet (&U->sendCb);

        if (!CYCLIC_Elements (&U->sendCb))
        {
            // Disable UART transmit register empty interrupt.
            __HAL_UART_DISABLE_IT(U->huart, UART_IT_TXE);
        }
    }
}


inline static void uartIrqRecv (struct STREAM_UART *const U, const uint32_t Sr,
                                const uint32_t Cr1)
{
    if ((Sr & USART_SR_RXNE) && (Cr1 & USART_CR1_RXNEIE))
    {
        const uint8_t Octet = U->huart->Instance->DR;
        CYCLIC_IN_FromOctet (&U->recvCb, Octet);
    }
}


inline static void uartIrq (struct STREAM_UART *const U)
{
    const uint32_t Sr   = U->huart->Instance->SR;
    const uint32_t Cr1  = U->huart->Instance->CR1;

    // Handle data transmission
    uartIrqSend (U, Sr, Cr1);

    // Handle data reception
    uartIrqRecv (U, Sr, Cr1);
}


void USART1_IRQHandler (void)
{
    uartIrq (s_streamUart1);
}


void USART2_IRQHandler (void)
{
    uartIrq (s_streamUart2);
}


void USART3_IRQHandler (void)
{
    uartIrq (s_streamUart3);
}


void USART6_IRQHandler (void)
{
    uartIrq (s_streamUart6);
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
    struct STREAM_UART *const U = (struct STREAM_UART *) S;

    // Disable UART transmit register empty interrupt.
    __HAL_UART_DISABLE_IT (U->huart, UART_IT_TXE);

    uint32_t count = 0;
    // Check if the cyclic buffer can hold 'Octets' or less.
    uint32_t available = checkAvailable (&U->sendCb, Octets);
    if (available)
    {
        CYCLIC_IN_FromBuffer (&U->sendCb, Data, available);
        count = CYCLIC_IN_Count (&U->sendCb);

        // Try to fill the transmit register with an octet from the cyclic
        // buffer. If the register was empty, this action will cause to consume
        // 1 octet at most from the cyclic buffer.
        uartIrqSend (U, U->huart->Instance->SR, U->huart->Instance->CR1);

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

    // Enable UART transmit register empty interrupt. The interrupt
    // service will then send the next octet from the cyclic buffer.
    __HAL_UART_ENABLE_IT (U->huart, UART_IT_TXE);

    // Number of consumed 'Data' octets
    return count;
}


uint32_t dataOut (struct STREAM *const S, uint8_t *const Buffer,
                  const uint32_t Octets)
{
    struct STREAM_UART *const U = (struct STREAM_UART *) S;

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


enum DEVICE_CommandResult command (const void *const D, 
                                   const char *const Name,
                                   struct VARIANT *const Value)
{
    struct STREAM_UART *const U = (struct STREAM_UART *const) D;

    if (DEVICE_COMMAND_CHECK(STREAM_SET_UART_BAUD, STREAM_SET_SPEED))
    {
        const uint32_t ReqBaud = (uint32_t) VARIANT_ToUint (Value);
        U->huart->Init.BaudRate = ReqBaud;

        if (HAL_UART_Init(U->huart) != HAL_OK)
        {
            return DEVICE_CommandResult_Failed;
        }

        // FIXME: How to retrieve the actual baud rate set?
        VARIANT_SetUint (Value, ReqBaud);
    }
    else if (DEVICE_COMMAND_CHECK(STREAM_SET_UART_HWFLOW))
    {
        const bool HwFlowEnabled = VARIANT_ToBoolean (Value);

        if (HwFlowEnabled)
        {
            U->huart->Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
        }
        else
        {
            U->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
        }

        if (HAL_UART_Init(U->huart) != HAL_OK)
        {
            return DEVICE_CommandResult_Failed;
        }

        U->hwFlowCtrl = HwFlowEnabled;
    }
    else
    {
        return DEVICE_CommandResult_NotHandled;
    }

    return DEVICE_CommandResult_Ok;
}

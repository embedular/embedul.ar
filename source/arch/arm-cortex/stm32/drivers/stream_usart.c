/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] stm32 usart.

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

#include "embedul.ar/source/arch/arm-cortex/stm32/drivers/stream_usart.h"
#include "embedul.ar/source/core/device/board.h"


#define STREAM_USART_LPC_IRQ_PRIORITY       1

// Interrupt access to STREAM_USART instances. One singleton for each UART.
static struct STREAM_USART * s_streamUsart1 = NULL;
static struct STREAM_USART * s_streamUsart2 = NULL;
static struct STREAM_USART * s_streamUsart3 = NULL;
static struct STREAM_USART * s_streamUsart6 = NULL;


// Common IO interface
static void         hardwareInit    (struct STREAM *const S);
static enum DEVICE_CommandResult
                    command     (struct STREAM *const S, const char *const Name,
                                 struct VARIANT *const Value);
static uint32_t     dataIn      (struct STREAM *const S, 
                                 const uint8_t *const Data,
                                 const uint32_t Octets);
static uint32_t     dataOut     (struct STREAM *const S, uint8_t *const Buffer,
                                 const uint32_t Octets);


static const struct STREAM_IFACE STREAM_USART_IFACE =
{
    .Description    = "stm32 usart",
    .HardwareInit   = hardwareInit,
    .Command        = command,
    .DataIn         = dataIn,
    .DataOut        = dataOut
};


static void usartInit (struct STREAM_USART *const U,
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

    STREAM_Init ((struct STREAM *)U, &STREAM_USART_IFACE);
}


void STREAM_USART1_Init (struct STREAM_USART *const U,
                         UART_HandleTypeDef *const Huart,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUsart1);

    s_streamUsart1 = U;
    usartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void STREAM_USART2_Init (struct STREAM_USART *const U,
                         UART_HandleTypeDef *const Huart,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUsart1);

    s_streamUsart2 = U;
    usartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void STREAM_USART3_Init (struct STREAM_USART *const U,
                         UART_HandleTypeDef *const Huart,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUsart1);

    s_streamUsart3 = U;
    usartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void STREAM_USART6_Init (struct STREAM_USART *const U,
                         UART_HandleTypeDef *const Huart,
                         uint8_t *const InBuffer, const uint32_t InOctets,
                         uint8_t *const OutBuffer, const uint32_t OutOctets)
{
    BOARD_AssertState (!s_streamUsart1);

    s_streamUsart6 = U;
    usartInit (U, Huart, InBuffer, InOctets, OutBuffer, OutOctets);
}


void hardwareInit (struct STREAM *const S)
{
    struct STREAM_USART *const U = (struct STREAM_USART *) S;

    // Default timeouts
    S->inTimeout    = 5000;
    S->outTimeout   = 50;

    // Disable transmit complete.
    __HAL_UART_DISABLE_IT (U->huart, UART_IT_TC);

    // Disable transmit register empty.
    __HAL_UART_DISABLE_IT(U->huart, UART_IT_TXE);

    // Disable parity error.
    __HAL_UART_DISABLE_IT(U->huart, UART_IT_PE);

    // Disable frame, noise and overrun errors.
    __HAL_UART_DISABLE_IT(U->huart, UART_IT_ERR);

    // Enable receive register not empty.
    __HAL_UART_ENABLE_IT (U->huart, UART_IT_RXNE);
}


inline static void uartIrqSend (struct STREAM_USART *const U, const uint32_t Sr,
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


inline static void uartIrqRecv (struct STREAM_USART *const U, const uint32_t Sr,
                                const uint32_t Cr1)
{
    if ((Sr & USART_SR_RXNE) && (Cr1 & USART_CR1_RXNEIE))
    {
        const uint8_t Octet = U->huart->Instance->DR;
        CYCLIC_IN_FromOctet (&U->recvCb, Octet);
    }
}


inline static void uartIrq (struct STREAM_USART *const U)
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
    uartIrq (s_streamUsart1);
}


void USART2_IRQHandler (void)
{
    uartIrq (s_streamUsart2);
}


void USART3_IRQHandler (void)
{
    uartIrq (s_streamUsart3);
}


void USART6_IRQHandler (void)
{
    uartIrq (s_streamUsart6);
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
        U->huart->Init.BaudRate = ReqBaud;

        if (HAL_UART_Init(U->huart) != HAL_OK)
        {
            return DEVICE_CommandResult_Failed;
        }

        // FIXME: How to retrieve the actual baud rate set?
        VARIANT_SetUint (Value, ReqBaud);
    }
    else if (DEVICE_COMMAND_CHECK(EXE_UART_HWFLOW))
    {
        U->huart->Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;

        if (HAL_UART_Init(U->huart) != HAL_OK)
        {
            return DEVICE_CommandResult_Failed;
        }
    }
    else if (DEVICE_COMMAND_CHECK(EXE_UART_NOFLOW))
    {
        U->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;

        if (HAL_UART_Init(U->huart) != HAL_OK)
        {
            return DEVICE_CommandResult_Failed;
        }
    }
    else
    {
        return DEVICE_CommandResult_NotHandled;
    }

    return DEVICE_CommandResult_Ok;
}

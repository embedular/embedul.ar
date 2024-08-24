/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM] input/output octet stream device driver interface.

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

#pragma once

#include "embedul.ar/source/core/cyclic.h"
#include "embedul.ar/source/core/variant.h"
#include "embedul.ar/source/core/timer.h"
#include "embedul.ar/source/core/device.h"


#define STREAM_IN_FromParsedString(_s,_c,_m,_str,...) \
    STREAM_IN_FromParsedStringArgs (_s, _c, _m, _str, \
                                    VARIANT_AutoParams(__VA_ARGS__))

#define STREAM_ADDRT_IN_BUFFER(_stream,_addr,_timeout,_data,_size) \
    STREAM_Address (_stream, &VARIANT_SpawnAuto(_addr)); \
    STREAM_Timeout (_stream, _timeout); \
    STREAM_IN_FromBuffer (_stream, _data, _size);

#define STREAM_ADDRT_EXCHANGE_BUFFERS(_stream,_addr,_timeout,_in_data,_in_size,\
                                        _out_data,_out_size) \
    STREAM_Address (_stream, &VARIANT_SpawnAuto(_addr)); \
    STREAM_Timeout (_stream, _timeout); \
    STREAM_EXCHANGE_Buffers (_stream, _in_data, _in_size, _out_data, _out_size);


struct STREAM;

struct STREAM_DataExchangeResult
{
    uint32_t inCount;
    uint32_t outCount;
};

typedef void        (* STREAM_HardwareInitFunc)(struct STREAM *const S);
typedef void        (* STREAM_ConnectFunc)(struct STREAM *const S);
typedef bool        (* STREAM_AssertConnectedFunc)(struct STREAM *const S);
typedef enum DEVICE_CommandResult
                    (* STREAM_CommandFunc)(struct STREAM *const S, 
                                            const char *const Name,
                                            struct VARIANT *const Value);
typedef uint32_t    (* STREAM_DataInFunc)(struct STREAM *const S, 
                                            const uint8_t *const Data,
                                            const uint32_t Octets);
typedef uint32_t    (* STREAM_DataOutFunc)(struct STREAM *const S,
                                            uint8_t *const Buffer,
                                            const uint32_t Octets);
typedef struct STREAM_DataExchangeResult
                    (* STREAM_DataExchangeFunc)(struct STREAM *const S,
                                            const uint8_t *const InData,
                                            const uint32_t InOctets,
                                            uint8_t *const OutBuffer,
                                            const uint32_t OutOctets);


enum STREAM_TransferType
{
    STREAM_TransferType_Undefined,
    STREAM_TransferType_In,
    STREAM_TransferType_Out,
    STREAM_TransferType_Composite
};


enum STREAM_TransferStatus
{
    STREAM_TransferStatus_Ok,
     // The driver uses "Stopped" to stop the API from requesting more data.
    STREAM_TransferStatus_Stopped,
    STREAM_TransferStatus_Disconnected,
    STREAM_TransferStatus_Timedout,
    STREAM_TransferStatus_NoAck,
    STREAM_TransferStatus_BusError,
    STREAM_TransferStatus_TargetNoAck,
    STREAM_TransferStatus_ArbitrationLost,
    STREAM_TransferStatus_UnknownError
};


struct STREAM_IFACE
{
    const char                      * const Description;
    const STREAM_HardwareInitFunc   HardwareInit;
    const STREAM_ConnectFunc        Connect;
    const STREAM_CommandFunc        Command;
    const STREAM_DataInFunc         DataIn;
    const STREAM_DataOutFunc        DataOut;
    const STREAM_DataExchangeFunc   DataExchange;
};


struct STREAM
{
    const struct STREAM_IFACE       * iface;
    bool                            connected;
    enum STREAM_TransferType        type;
    enum STREAM_TransferStatus      status;
    struct VARIANT                  address;
    uint32_t                        iteration;
    TIMER_Ticks                     timeout;
    uint32_t                        count;
    uint8_t                         s2sInRetry;
};


void            STREAM_Init                 (struct STREAM *const S,
                                             const struct STREAM_IFACE *const
                                             Iface);
bool            STREAM_IsValid              (struct STREAM *const S);
bool            STREAM_Connect              (struct STREAM *const S);
bool            STREAM_IsConnected          (struct STREAM *const S);
enum DEVICE_CommandResult
                STREAM_Command              (struct STREAM *const S,
                                             const char *const Name,
                                             struct VARIANT *const Value);
void            STREAM_Address              (struct STREAM *const S,
                                             struct VARIANT *const Address);
void            STREAM_Timeout              (struct STREAM *const S,
                                             const TIMER_Ticks Ticks);
enum STREAM_TransferType
                STREAM_TransferType         (struct STREAM *const S);
enum STREAM_TransferStatus
                STREAM_TransferStatus       (struct STREAM *const S);
uint32_t        STREAM_Count                (struct STREAM *const S);
void            STREAM_IN_FromBuffer        (struct STREAM *const S,
                                             const uint8_t *const Data,
                                             const uint32_t Octets);
void            STREAM_IN_FromString        (struct STREAM *const S,
                                             const char *const Str);
uint32_t        STREAM_IN_FromParsedStringArgs
                                            (struct STREAM *const S,
                                             const uint32_t OutColumn, 
                                             const size_t MaxOctets,
                                             const char *const Str,
                                             struct VARIANT *const ArgValues,
                                             const uint32_t ArgCount);
void            STREAM_IN_FromOctet         (struct STREAM *const S, 
                                             const uint8_t Octet);
void            STREAM_IN_FromStream        (struct STREAM *const S,
                                             struct STREAM *const Out);
void            STREAM_IN_S2SRetry          (struct STREAM *const S);
void            STREAM_OUT_ToBuffer         (struct STREAM *const S,
                                             uint8_t *const Buffer,
                                             const uint32_t Octets);
uint8_t         STREAM_OUT_ToOctet          (struct STREAM *const S);
void            STREAM_OUT_ToStream         (struct STREAM *const S,
                                             struct STREAM *const In);
void            STREAM_OUT_Discard          (struct STREAM *const S);
void            STREAM_EXCHANGE_Buffers     (struct STREAM *const S,
                                             const uint8_t *const InData,
                                             const uint32_t InOctets,
                                             uint8_t *const OutBuffer,
                                             const uint32_t OutOctets);
const char *    STREAM_Description          (struct STREAM *const S);

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
#include "embedul.ar/source/core/cc.h"


#define STREAM_IN_FromParsedString(_s,_c,_m,_str,...) \
    STREAM_IN_FromParsedStringArgs (_s, _c, _m, _str, \
                                    VARIANT_AutoParams(__VA_ARGS__))


struct STREAM;


typedef void        (* STREAM_HardwareInitFunc)(struct STREAM *const S);
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


struct STREAM_IFACE
{
    const char                      * const Description;
    const STREAM_HardwareInitFunc   HardwareInit;
    const STREAM_CommandFunc        Command;
    const STREAM_DataInFunc         DataIn;
    const STREAM_DataOutFunc        DataOut;
};


struct STREAM
{
    const struct STREAM_IFACE       * iface;
    TIMER_Ticks                     inTimeout;
    TIMER_Ticks                     outTimeout;
    uint32_t                        lastIn;
    uint32_t                        lastOut;
    uint8_t                         octetInRetry;
    bool                            stop;
};


void            STREAM_Init                 (struct STREAM *const S,
                                             const struct STREAM_IFACE *const
                                             Iface);
bool            STREAM_IsValid              (struct STREAM *const S);
enum DEVICE_CommandResult
                STREAM_Command              (struct STREAM *const S,
                                             const char *const Name,
                                             struct VARIANT *const Value);
void            STREAM_OUT_Timeout          (struct STREAM *const S,
                                             const TIMER_Ticks Ticks);
void            STREAM_OUT_ToBuffer         (struct STREAM *const S,
                                             uint8_t *const Buffer,
                                             const uint32_t Octets);
uint8_t         STREAM_OUT_ToOctet          (struct STREAM *const S);
void            STREAM_OUT_ToStream         (struct STREAM *const S,
                                             struct STREAM *const In);
void            STREAM_OUT_Discard          (struct STREAM *const S);
uint32_t        STREAM_OUT_Count            (struct STREAM *const S);
void            STREAM_IN_Timeout           (struct STREAM *const S,
                                             const TIMER_Ticks Ticks);
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
void            STREAM_IN_OctetRetry        (struct STREAM *const S);
uint32_t        STREAM_IN_Count             (struct STREAM *const S);
const char *    STREAM_Description          (struct STREAM *const S);

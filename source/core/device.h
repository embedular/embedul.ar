/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] device drivers common operations.

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

#include "embedul.ar/source/core/object.h"
#include "embedul.ar/source/core/variant.h"
#include <string.h>


#define DEVICE_COMMAND_STREAM_SET_HOSTNAME              "s|hostname"
#define DEVICE_COMMAND_STREAM_SET_WIFI_SSID             "s|wifi.ssid"
#define DEVICE_COMMAND_STREAM_SET_PASSWORD              "s|password"
#define DEVICE_COMMAND_STREAM_SET_UART_BAUD             "s|uart.baud"
#define DEVICE_COMMAND_STREAM_SET_UART_HWFLOW           "s|uart.hwflow"
#define DEVICE_COMMAND_STREAM_SET_SPEED                 "s|speed"
#define DEVICE_COMMAND_STREAM_SET_FRAME_BITS            "s|framebits"
#define DEVICE_COMMAND_STREAM_SET_IP_TCP_PORT           "s|ip.tcp.port"
#define DEVICE_COMMAND_STREAM_SET_IP_UDP_PORT           "s|ip.udp.port"

/*
#define DEVICE_COMMAND_STREAM_EXE_POWEROFF              "x|poweroff" 
#define DEVICE_COMMAND_STREAM_EXE_UART_HWFLOW           "x|uart.hwflow"
#define DEVICE_COMMAND_STREAM_EXE_UART_NOFLOW           "x|uart.noflow"
*/

#define DEVICE_COMMAND_CHECK_4(_cmd,...) \
                !strcmp(Name, DEVICE_COMMAND_##_cmd) || \
                DEVICE_COMMAND_CHECK_3(__VA_ARGS__)

#define DEVICE_COMMAND_CHECK_3(_cmd,...) \
                !strcmp(Name, DEVICE_COMMAND_##_cmd) || \
                DEVICE_COMMAND_CHECK_2(__VA_ARGS__)

#define DEVICE_COMMAND_CHECK_2(_cmd,...) \
                !strcmp(Name, DEVICE_COMMAND_##_cmd) || \
                DEVICE_COMMAND_CHECK_1(__VA_ARGS__)

#define DEVICE_COMMAND_CHECK_1(_cmd) \
                !strcmp(Name, DEVICE_COMMAND_##_cmd)

#define DEVICE_COMMAND_CHECK_0(_cmd) \
                _Static_assert(0, LANG_COMMAND_REQUIRED);

#define DEVICE_COMMAND_CHECK(...) \
                CC_ExpPaste(DEVICE_COMMAND_CHECK_,CC_ArgsCount(__VA_ARGS__)) \
                    (__VA_ARGS__)


enum DEVICE_CommandResult
{
    DEVICE_CommandResult_Ok = 0,
    DEVICE_CommandResult_NoMethod,
    DEVICE_CommandResult_NotHandled,
    DEVICE_CommandResult_Failed
};

typedef enum DEVICE_CommandResult (* DEVICE_CommandFunc)(const void *const V, 
                            const char *const Name,
                            struct VARIANT *const Value);


enum DEVICE_CommandResult DEVICE_Command (
                            const char *const ContextMsg,
                            struct OBJECT_INFO *const DevInfo,
                            const DEVICE_CommandFunc DevCommandFunc,
                            const char *const Name,
                            struct VARIANT *const Value);


#define DEVICE_IMPLEMENTATION_Clear(_p) \
    OBJECT_AssertValid (&_p->device); \
    _Static_assert ((sizeof(*_p) - sizeof(_p->device)) >= 0, \
                    LANG_INVALID_IMPLEMENTATION); \
    memset ((void *)((uintptr_t)_p + sizeof(_p->device)), 0, \
            sizeof(*_p) - sizeof(_p->device))

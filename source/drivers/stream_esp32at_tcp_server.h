/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] esp32 tcp server handling by 'at commands.

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

#include "embedul.ar/source/core/device/stream.h"


#define SOCKET_ESP32AT_TCP_SERVER_HOSTNAME_MAX      23
#define SOCKET_ESP32AT_TCP_SERVER_SSID_MAX          23
#define SOCKET_ESP32AT_TCP_SERVER_PASSWORD_MAX      23


struct STREAM_ESP32AT_TCP_SERVER
{
    struct STREAM       device;
    struct STREAM       * esp32at;  // AT Connection to ESP32
    char                hostname[SOCKET_ESP32AT_TCP_SERVER_HOSTNAME_MAX + 1];
    char                ssid[SOCKET_ESP32AT_TCP_SERVER_SSID_MAX + 1];
    char                password[SOCKET_ESP32AT_TCP_SERVER_PASSWORD_MAX + 1];
    uint32_t            cmdFullSpeed;
    uint16_t            tcpPort;
    uint16_t            udpPort;
    uint32_t            addrOrigin;
    uint32_t            addrDest;
    // Used for internal AT messages processing from ESP32 to MCU UART
    uint32_t            pendingData;
    char                atmBuf[24];
    uint32_t            atmUsed;
};


void STREAM_ESP32AT_TCP_SERVER_Init (struct STREAM_ESP32AT_TCP_SERVER *const E,
                                     struct STREAM *const Esp32at);

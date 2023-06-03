/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] tcp server by network sockets.

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

#include "embedul.ar/source/drivers/packet_socket_tcp_server.h"
#include "embedul.ar/source/core/device/board.h"
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>


#define SOCKET_DEFAULT_TCP_PORT             42424
#define SOCKET_DEFAULT_UDP_PORT             42424


static enum DEVICE_CommandResult
                    command         (struct STREAM *const S,
                                     const char *const Name,
                                     struct VARIANT *const Value);
static uint32_t     send_iface      (struct STREAM *const S, 
                                     const uint8_t *const Data,
                                     const uint32_t Octets);
static uint32_t     recv_iface      (struct STREAM *const S,
                                     uint8_t *const Buffer,
                                     const uint32_t Octets);


static const struct STREAM_IFACE STREAM_SOCKET_TCP_SERVER_IFACE =
{
    .Description    = "tcp/udp socket server",
    .Command        = command,
    .Send           = send_iface,
    .Recv           = recv_iface,
};


void STREAM_SOCKET_TCP_SERVER_Init (struct STREAM_SOCKET_TCP_SERVER *const T)
{
    BOARD_AssertParams (T);

    DEVICE_IMPLEMENTATION_Clear (T);

    T->listenTcpSocket  = -1;
    T->tcpPort          = SOCKET_DEFAULT_TCP_PORT;
    T->udpPort          = SOCKET_DEFAULT_UDP_PORT;

    STREAM_Init ((struct STREAM *)T, &STREAM_SOCKET_TCP_SERVER_IFACE);
}


static void haltOnError (struct STREAM *const S, const char *const Caption,
                         const int RetVal)
{
    if (RetVal == -1)
    {
        LOG_Warn (S, Caption);
        LOG_Items (2, 
                        "error",        strerrorname_np(errno),
                        "description",  strerrordesc_np(errno));
        BOARD_AssertState (false);
    }
}


static bool connect_impl (struct STREAM *const S)
{
    struct STREAM_SOCKET_TCP_SERVER *const T =
                            (struct STREAM_SOCKET_TCP_SERVER *) S;

    if (T->listenTcpSocket != -1)
    {
        close (T->listenTcpSocket);
    }

    haltOnError (S, "Socket creation",
                 T->listenTcpSocket = socket(AF_INET, SOCK_STREAM, 0));

    const int Flags = fcntl (T->listenTcpSocket, F_GETFL);

    haltOnError (S, "Get socket flags",
                 Flags);
                 
    haltOnError (S, "Set socket non-blocking mode",
                 fcntl (T->listenTcpSocket, F_SETFL, Flags | O_NONBLOCK));

    const struct sockaddr_in Addr =
    {
        .sin_family         = AF_INET,
        .sin_port           = htons (42424),
        .sin_addr.s_addr    = htonl (INADDR_ANY)
    };

    haltOnError (S, "Bind socket to port/address",
                 bind(T->listenTcpSocket, (struct sockaddr *)&Addr,
                      sizeof(Addr)));

    haltOnError (S, "Listening to socket",
                 listen(T->listenTcpSocket, 5));
}


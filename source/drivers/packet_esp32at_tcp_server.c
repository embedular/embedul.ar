/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [PACKET driver] esp32 tcp server handling by 'at commands.

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

#include "embedul.ar/source/drivers/packet_esp32at_tcp_server.h"
#include "embedul.ar/source/core/device/board.h"


#define ESP32AT_DEFAULT_CMD_INITIAL_SPEED       115200
#define ESP32AT_DEFAULT_CMD_FULL_SPEED          1000000
#define ESP32AT_DEFAULT_HOSTNAME                "retrociaa"
#define ESP32AT_DEFAULT_TCP_PORT                42424
#define ESP32AT_DEFAULT_UDP_PORT                42424
#define ESP32AT_POWERCYCLE_OFF_DELAY            1500
#define ESP32AT_POWERCYCLE_ON_DELAY             1500
#define ESP32AT_AT_CMD_DEFAULT_TIMEOUT          300
#define ESP32AT_HELLO_RETRY                     0
#define ESP32AT_HELLO_RETRY_DELAY               0


// Common IO interface
static void         hardwareInit    (struct PACKET *const P);
static enum DEVICE_CommandResult
                    command         (struct PACKET *const P,
                                     const char *const Name,
                                     struct VARIANT *const Value);
static uint32_t     send            (struct PACKET *const P, 
                                     const uint8_t *const Data,
                                     const uint32_t Octets);
static uint32_t     recvSize        (struct PACKET *const P,
                                     const uint32_t MaxOctets);
static uint32_t     recv            (struct PACKET *const P,
                                     uint8_t *const Buffer,
                                     const uint32_t Octets);


static const struct PACKET_IFACE PACKET_ESP32AT_TCP_SERVER_IFACE =
{
    .Description    = "esp32 tcp/udp packets",
    .HardwareInit   = hardwareInit,
    .Command        = command,
    .Send           = send,
    .RecvSize       = recvSize,
    .Recv           = recv,
};


void PACKET_ESP32AT_TCP_SERVER_Init (struct PACKET_ESP32AT_TCP_SERVER *const E,
                                     struct STREAM *const Esp32AT)
{
    BOARD_AssertParams (E && STREAM_IsValid(Esp32AT));

    DEVICE_IMPLEMENTATION_Clear (E);

    E->esp32at      = Esp32AT;
    E->cmdFullSpeed = ESP32AT_DEFAULT_CMD_FULL_SPEED;
    E->tcpPort      = ESP32AT_DEFAULT_TCP_PORT;
    E->udpPort      = ESP32AT_DEFAULT_UDP_PORT;

    PACKET_Init ((struct PACKET *)E, &PACKET_ESP32AT_TCP_SERVER_IFACE);
}


struct RnSplitEntry
{
    uint16_t begin;
    uint16_t end;
};


struct RnSplit
{
    char                    * data;
    uint32_t                dataCapacity;
    uint32_t                dataUsed;
    struct RnSplitEntry     * entries;
    uint32_t                entryCount;
    uint32_t                entriesFound;
};


struct RnCompareItem
{
    const int16_t           EntryIndex;
    const uint16_t          MaxChars;
    const char              *const Value;
};


struct RnCompareList
{
    const char              *const Command;
    const char              *const Description;
    const uint32_t          ExpectedOctets;
    const uint32_t          Timeout;
    const uint32_t          ItemCount;
    struct RnCompareItem    Items[];
};


const struct RnCompareList AT_RESTORE =
{
    .Command        = "AT+RESTORE",
    .Description    = LANG_RESTORE_FACTORY,
    // AT+RESTORE/r/n
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }    
};


const struct RnCompareList AT =
{
    .Command        = "AT",
    .Description    = LANG_HELLO,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_SYSSTORE =
{
    .Command        = "AT+SYSSTORE=0",
    .Description    = LANG_DISABLE_COMMAND_STORE,
    // AT+SYSSTORE=0/r/n
    // /r/n
    // OK/r/n
    .ExpectedOctets = 21,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};



const struct RnCompareList AT_SYSLOG =
{
    .Command        = "AT+SYSLOG=1",
    .Description    = LANG_ENABLE_ERROR_CODES,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList ATE0 =
{
    .Command        = "ATE0",
    .Description    = LANG_SWITCH_ECHO_OFF,
    // ATE0\r\n
    // \r\n
    // OK\r\n
    .ExpectedOctets = 12,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_UART_CUR =
{
    // `0: uart baudrate
    .Command        = "AT+UART_CUR=`0,8,1,0,3",
    .Description    = LANG_UART_CONFIG,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_GMR =
{
    .Command        = "AT+GMR",
    .Description    = LANG_VERSION_INFO,
    // (sample response below, note that .ResponseOctets is unknown)
    //
    // AT version:2.2.0.0(c6fa6bf - ESP32 - Jul  2 2021 06:44:05)\r\n
    // SDK version:v4.2.2-76-gefa6eca\r\n
    // compile time(3a696ba):Jul  2 2021 11:54:43\r\n
    // Bin version:2.2.0(WROOM-32)\r\n
    // /r/n
    // OK/r/n
    .ItemCount      = 2,
    .Items          = 
    {
        {
            .EntryIndex = 0,
            .MaxChars   = 11,
            .Value      = "AT version:"
        },
        {
            .EntryIndex = -1,
            .MaxChars   = 0,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_CWMODE =
{
    .Command        = "AT+CWMODE=1",
    .Description    = LANG_SET_WIFI_MODE,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_CWHOSTNAME =
{
    // `0: hostname
    .Command        = "AT+CWHOSTNAME=\"`0\"",
    .Description    = LANG_SET_HOSTNAME,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_CWJAP =
{
    // `0: SSID, `1: Password
    .Command        = "AT+CWJAP=\"`0\",\"`1\",,,,,0",
    .Description    = LANG_CONNECT_TO_ACESS_POINT,
    // WIFI DISCONNECT/r/n
    // WIFI CONNECTED/r/n
    // WIFI GOT IP/r/n
    // /r/n
    // OK/r/n"
    .ExpectedOctets = 52,
    .Timeout        = 5000,
    .ItemCount      = 3,
    .Items          =
    {
        {
            .EntryIndex = -4,
            .Value      = "WIFI CONNECTED"
        },
        {
            .EntryIndex = -3,
            .Value      = "WIFI GOT IP"
        },
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_CWRECONNCFG =
{
    // The ESP32 station will not reconnect to AP when disconnected.
    .Command        = "AT+CWRECONNCFG=0,0",
    .Description    = LANG_NO_AUTO_RECONNECT,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          = 
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_CIPMUX =
{
    // Enable multiple connections.
    .Command        = "AT+CIPMUX=1",
    .Description    = LANG_SET_MULTI_CONNECTIONS,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          =
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"    
        }
    }
};


const struct RnCompareList AT_CIPSERVER =
{
    // `0: TCP server port
    .Command        = "AT+CIPSERVER=1,`0",
    .Description    = LANG_CREATE_TCP_SERVER,
    // /r/n
    // OK/r/n
    .ExpectedOctets = 6,
    .ItemCount      = 1,
    .Items          =
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


const struct RnCompareList AT_CIPSTART =
{
    // `0: UDP transmission port
    // IP stated does not care in receiving mode 2.
    .Command        = "AT+CIPSTARTEX=\"UDP\",\"192.168.1.1\",`0,`0,2",
    .Description    = LANG_CREATE_UDP_TRANSMISSION,
    // #,CONNECT\r\n
    // /r/n
    // OK/r/n
    .ExpectedOctets = 17,
    .ItemCount      = 1,
    .Items          =
    {
        {
            .EntryIndex = -1,
            .Value      = "OK"
        }
    }
};


static void rnInit (struct RnSplit *const R, 
                     struct RnSplitEntry *const Entries, 
                     const uint32_t EntryCount, char *const Data,
                     const uint32_t DataCapacity)
{
    BOARD_AssertParams (R && Entries && EntryCount && Data && DataCapacity);

    R->data         = Data;
    R->dataCapacity = DataCapacity;
    R->dataUsed     = 0;
    R->entries      = Entries;
    R->entryCount   = EntryCount;
    R->entriesFound = 0;
}


static void rnSplit (struct RnSplit *const R, const uint32_t DataUsed)
{
    BOARD_AssertParams (R && R->data && R->entries && R->entryCount);

    R->dataUsed     = DataUsed;
    R->entriesFound = 0;

    // At least 2 octets required
    if (DataUsed < 2)
    {
        return;
    }

    uint32_t i = 0;
    uint32_t e = 0;

    R->entries[e].begin = i;

    do
    {
        if (i > R->dataUsed - 2)
        {
            R->entries[e].end = R->dataUsed;
            break;
        }

        if (R->data[i] == '\r' && R->data[i + 1] == '\n')
        {
            R->entries[e].end = i;
            ++ e;

            if (e >= R->entryCount)
            {
                break;
            }

            R->entries[e].begin = i + 2;

            ++ R->entriesFound;
            i += 2;
        }
        else
        {
            ++ i;
        }
    }
    while (1);
}


uint32_t rnEntryToRealIndex (const struct RnSplit *const R,
                             const int32_t EntryIndex)
{
    BOARD_AssertParams (R);

    BOARD_AssertParams ((EntryIndex < 0)?
                            EntryIndex + (int32_t)R->entriesFound >= 0 :
                            (uint32_t)EntryIndex < R->entriesFound);

    const uint32_t RealIndex = (EntryIndex < 0)?
                                    EntryIndex + R->entriesFound :
                                    (uint32_t)EntryIndex;

    return RealIndex;    
}


static bool rnCompare (const struct RnSplit *const R, const int32_t EntryIndex,
                       const char *const Value, const uint32_t MaxChars)
{
    BOARD_AssertParams (R && Value);

    if (!R->entriesFound)
    {
        return false;
    }

    const uint32_t RealIndex        = rnEntryToRealIndex (R, EntryIndex);
    const struct RnSplitEntry * SE  = &R->entries[RealIndex];
    const uint32_t EntryLen         = SE->end - SE->begin;
    const uint32_t Len              = (MaxChars > EntryLen || !MaxChars)? 
                                        EntryLen : MaxChars;

    return (!strncmp (&R->data[SE->begin], Value, Len))?
            true : false;
}


static bool rnCompareList (const struct RnSplit *const R,
                           const struct RnCompareList *const List)
{
    BOARD_AssertParams (R && List);

    if (!R->entriesFound || R->entriesFound < List->ItemCount)
    {
        return false;
    }

    for (uint32_t i = 0; i < List->ItemCount; ++i)
    {
        const struct RnCompareItem *const Item = &List->Items[i];

        if (!rnCompare (R, Item->EntryIndex, Item->Value, Item->MaxChars))
        {
            return false;
        }
    }

    return true;
}


// This function modifies R->data
static const char * rnGetString (struct RnSplit *const R,
                                 const int32_t EntryIndex)
{
    BOARD_AssertParams (R);

    const uint32_t RealIndex = rnEntryToRealIndex (R, EntryIndex);

    // Every entry must terminate with "\r\n", so end should index \r even
    // on the last element. If not, at least capacity should be bigger enough
    // to allow for a null termination after R->dataUsed.
    BOARD_AssertState (R->entries[RealIndex].end < R->dataCapacity);

    R->data[R->entries[RealIndex].end] = '\0';

    return &R->data[R->entries[RealIndex].begin];
}


#define sendAtCommand(_r,_l,_s,_e,...) \
    sendAtCommandArgs (_r,_l,_s,_e, \
                       VARIANT_AutoParams(__VA_ARGS__))


static bool sendAtCommandArgs (struct RnSplit *const R,
                               const struct RnCompareList *const List,
                               struct PACKET *const Server,
                               struct STREAM *const Esp32at,
                               struct VARIANT *const ArgValues,
                               const uint32_t ArgCount)
{
    BOARD_AssertParams (List->ExpectedOctets <= R->dataCapacity);

    STREAM_OUT_Discard (Esp32at);

    LOG_PendingBegin (Server, List->Description);

    // Send AT command to ESP32
    STREAM_IN_FromParsedStringArgs (Esp32at, 0, 256, List->Command,
                              ArgValues, ArgCount);
    STREAM_IN_FromString (Esp32at, "\r\n");

    const TIMER_Ticks   Timeout         = List->Timeout? List->Timeout : 
                                                ESP32AT_AT_CMD_DEFAULT_TIMEOUT;
    const uint32_t      ExpectedOctets  = List->ExpectedOctets?
                                                List->ExpectedOctets : 
                                                R->dataCapacity;

    memset (R->data, 0xFF, R->dataCapacity);

    STREAM_OUT_Timeout (Esp32at, Timeout);

    // Receive response from ESP32
    STREAM_OUT_ToBuffer (Esp32at, (uint8_t *)R->data, ExpectedOctets);

    const uint32_t ResponseOctets = STREAM_OUT_Count (Esp32at);

    rnSplit (R, ResponseOctets);

    if (rnCompareList (R, List))
    {
        LOG_PendingEndOk ();
        return true;
    }

    STREAM_OUT_Discard (Esp32at);

    LOG_PendingEndFail ();
    LOG_BinaryDump (Server, LANG_RESPONSE, (uint8_t *)R->data, ResponseOctets);

    return false;
}


static bool waitHelloResponse (struct RnSplit *const R,
                         struct PACKET *const Server,
                         struct STREAM *const Esp32at,
                         const uint32_t Retries)
{
    uint32_t    retries = Retries;
    bool        result;

    while (!(result = sendAtCommand(R, &AT, Server, Esp32at)) && retries--)
    {
        #if ESP32AT_HELLO_RETRY_DELAY
            BOARD_Delay (ESP32AT_HELLO_RETRY_DELAY);
        #endif
    }

    return result;
}


enum ConnectStepFlags
{
    ConnectStepFlags_PowerOn            = 0x01,
    ConnectStepFlags_Wifi               = 0x02,
    ConnectStepFlags_TcpServer          = 0x04,
    ConnectStepFlags_UdpTransmission    = 0x08,
    ConnectStepFlags_FullSpeed          = 0x10,
    ConnectStepFlags__ALL               = 0xFF
};


static void hidePassword (const char *const Original, char *const Hidden,
                          const size_t Length)
{
    Hidden[Length] = '\0';
    memset (Hidden, '*', Length);

    if (Length > 3)
    {
        for (size_t i = 0; i < 2; ++i)
        {
            Hidden[i] = Original[i];
        }
    }

    if (Length > 10)
    {
        for (size_t i = Length - 1; i > Length - 3; --i)
        {
            Hidden[i] = Original[i];
        }
    }    
}


static void esp32atCmdInitialSpeed (struct STREAM *const Esp32AT)
{
    STREAM_Command (Esp32AT, DEVICE_COMMAND_SET_UART_BAUD, 
                        &VARIANT_SpawnUint(ESP32AT_DEFAULT_CMD_INITIAL_SPEED));
    STREAM_Command (Esp32AT, DEVICE_COMMAND_EXE_UART_NOFLOW,
                        &VARIANT_SpawnBoolean(false));
}


static void esp32atCmdFullSpeed (struct STREAM *const Esp32AT, 
                              const uint32_t FullSpeed)
{
    STREAM_Command (Esp32AT, DEVICE_COMMAND_SET_UART_BAUD, 
                        &VARIANT_SpawnUint(FullSpeed));
    STREAM_Command (Esp32AT, DEVICE_COMMAND_EXE_UART_HWFLOW,
                        &VARIANT_SpawnBoolean(false));
}


static bool connect (struct PACKET *const P, struct STREAM *const Esp32AT,
                     const enum ConnectStepFlags Steps)
{
    char                    buf[256];
    struct RnSplit          rn;
    struct RnSplitEntry     entries[8];

    struct PACKET_ESP32AT_TCP_SERVER *const E =
                                (struct PACKET_ESP32AT_TCP_SERVER *) P;

    rnInit (&rn, entries, 8, buf, sizeof(buf));

    if (Steps & ConnectStepFlags_PowerOn)
    {
        LOG_ContextBegin (P, LANG_MODULE_RESTART);
        {
            OUTPUT_BitNow  (OUTPUT_Bit_WirelessEnable, 0);
            BOARD_Delay    (ESP32AT_POWERCYCLE_OFF_DELAY);

            OUTPUT_BitNow  (OUTPUT_Bit_WirelessEnable, 1);
            BOARD_Delay    (ESP32AT_POWERCYCLE_ON_DELAY);

            LOG (P, LANG_DEFAULT_SPEED_NO_FLOW_CTRL);

            // MCU uart to module initial speed
            esp32atCmdInitialSpeed (Esp32AT);

            // check for module response
            if (!waitHelloResponse (&rn, P, Esp32AT, ESP32AT_HELLO_RETRY))
            {
                LOG_ContextEnd ();
                return false;
            }
        }
        LOG_ContextEnd ();

        LOG_ContextBegin (P, LANG_MODULE_SETUP);
        {
            // Disable command store
            if (!sendAtCommand (&rn, &AT_SYSSTORE, P, Esp32AT))
            {
                LOG_ContextEnd ();
                return false;
            }

            // AT command echo off
            if (!sendAtCommand (&rn, &ATE0, P, Esp32AT))
            {
                LOG_ContextEnd ();
                return false;
            }

            // Extended error log
            if (!sendAtCommand (&rn, &AT_SYSLOG, P, Esp32AT))
            {
                LOG_ContextEnd ();
                return false;
            }

            // Multiple connections mode, max 5 on stock firmware
            if (!sendAtCommand (&rn, &AT_CIPMUX, P, Esp32AT))
            {
                LOG_ContextEnd ();
                return false;
            }
        }
        LOG_ContextEnd ();

        // Version info
        if (!sendAtCommand (&rn, &AT_GMR, P, Esp32AT))
        {
            return false;
        }

        for (uint32_t i = 0; i < rn.entriesFound - 2; ++i)
        {
            LOG_Items (1, i + 1, rnGetString(&rn, i));
        }
    }

    if (Steps & ConnectStepFlags_Wifi)
    {
        LOG_ContextBegin (P, LANG_CONFIGURING_WIFI);
        {
            // WI-FI station mode.
            if (!sendAtCommand (&rn, &AT_CWMODE, P, Esp32AT))
            {
                LOG_ContextEnd ();
                return false;
            }

            // WI-FI hostname.
            LOG_Items (1, LANG_HOSTNAME, E->hostname);
            if (!sendAtCommand (&rn, &AT_CWHOSTNAME, P, Esp32AT,
                                E->hostname))
            {
                LOG_ContextEnd ();
                return false;
            }

            const size_t PassLength =
                strnlen (E->password, PACKET_ESP32AT_TCP_SERVER_PASSWORD_MAX);

            char hiddenPass[PassLength + 1];
            hidePassword (E->password, hiddenPass, PassLength);

            // WI-FI connect to access point.
            LOG_Items (2,   
                            LANG_SSID,         E->ssid,
                            LANG_PASSWORD,     hiddenPass);

            if (!sendAtCommand (&rn, &AT_CWJAP, P, Esp32AT, 
                                E->ssid, E->password))
            {
                LOG_ContextEnd ();
                return false;
            }

            // WI-FI disable automatic reconnection to AP.
            // For some unknown reason this setting is ignored when passed
            // through the CWJAP command.
            if (!sendAtCommand (&rn, &AT_CWRECONNCFG, P, Esp32AT))
            {
                LOG_ContextEnd ();
                return false;
            }
        }
        LOG_ContextEnd ();
    }

    if (Steps & ConnectStepFlags_TcpServer)
    {
        LOG_ContextBegin (P, LANG_TCP_SERVER_START);
        {
            LOG_Items (1, LANG_TCP_PORT, E->tcpPort);

            if (!sendAtCommand (&rn, &AT_CIPSERVER, P, Esp32AT,
                                E->tcpPort))
            {
                LOG_ContextEnd ();
                return false;
            }
        }
        LOG_ContextEnd ();
    }

    if (Steps & ConnectStepFlags_UdpTransmission)
    {
        LOG_ContextBegin (P, LANG_UDP_TRANSMISSION_START);
        {
            LOG_Items (1, LANG_UDP_PORT, E->udpPort);

            if (!sendAtCommand (&rn, &AT_CIPSTART, P, Esp32AT,
                                E->udpPort))
            {
                LOG_ContextEnd ();
                return false;
            }
        }
        LOG_ContextEnd ();
    }

    if (Steps & ConnectStepFlags_FullSpeed)
    {
        LOG_ContextBegin (P, LANG_FULL_SPEED);
        {
            if (!sendAtCommand (&rn, &AT_UART_CUR, P, Esp32AT,
                                E->cmdFullSpeed))
            {
                LOG_ContextEnd ();
                return false;
            }

            // Change AT UART to full speed and hardware flow control
            esp32atCmdFullSpeed (Esp32AT, E->cmdFullSpeed);
        }
        LOG_ContextEnd ();
    }

    // Clear timeouts used on connection AT commands. Further data retrieved
    // will have no timeout.
    STREAM_OUT_Timeout (Esp32AT, 0);

    // "connected" means everything needed to initialize a fully-functional
    // connection (more than a successful connection to a wifi AP)
    E->connected = true;

    return true;
}


static void hardwareInit (struct PACKET *const P)
{
    struct PACKET_ESP32AT_TCP_SERVER *const E =
                                (struct PACKET_ESP32AT_TCP_SERVER *) P;

    LOG_Items (1, "esp32 at-stream", STREAM_Description(E->esp32at));

    // Set default hostname
    PACKET_Command (P, DEVICE_COMMAND_SET_HOSTNAME, 
                            &VARIANT_SpawnString(ESP32AT_DEFAULT_HOSTNAME));

    // Set default full-speed (used after initialization)
    PACKET_Command (P, DEVICE_COMMAND_SET_SPEED,
                            &VARIANT_SpawnUint(E->cmdFullSpeed));

    // Assume ESP32 UART is currently at default speed and no flow control
    esp32atCmdInitialSpeed (E->esp32at);
}


static inline bool setConfString (char *const String, 
                                  const uint32_t StringChars,
                                  struct VARIANT *const Value)
{
    if (VARIANT_StringOctetCount(Value) > StringChars)
    {
        return false;
    }

    String[StringChars] = '\0';
    strncpy (String, VARIANT_ToString(Value), StringChars);

    return true;
}


static enum DEVICE_CommandResult command (struct PACKET *const P,
                                          const char *const Name,
                                          struct VARIANT *const Value)
{
    struct PACKET_ESP32AT_TCP_SERVER *const E =
                                (struct PACKET_ESP32AT_TCP_SERVER *const) P;

    if (DEVICE_COMMAND_CHECK(GET_CONNECTED))
    {
        VARIANT_SetBoolean (Value, E->connected);

        if (!E->connected && E->autoRecDelay)
        {
            if (E->autoRecTarget < BOARD_TicksNow())
            {
                LOG (P, LANG_TRY_RECONNECT_NOW);

                if (!connect (P, E->esp32at, ConnectStepFlags__ALL))
                {
                    E->autoRecTarget = BOARD_TicksNow() + E->autoRecDelay;
                    LOG (&E->device, LANG_RETRYING_IN_MS_FMT, E->autoRecDelay);
                }
                else
                {
                    VARIANT_SetBoolean (Value, true);
                }
            }
        }
    }
    else if (DEVICE_COMMAND_CHECK(SET_HOSTNAME))
    {
        if (!setConfString (E->hostname,
                            PACKET_ESP32AT_TCP_SERVER_HOSTNAME_MAX, Value))
        {
            return DEVICE_CommandResult_Failed;
        }
    }
    else if (DEVICE_COMMAND_CHECK(SET_WIFI_SSID))
    {
        if (!setConfString (E->ssid,
                       PACKET_ESP32AT_TCP_SERVER_SSID_MAX, Value))
        {
            return DEVICE_CommandResult_Failed;
        }
    }
    else if (DEVICE_COMMAND_CHECK(SET_PASSWORD))
    {
        if (!setConfString (E->password,
                       PACKET_ESP32AT_TCP_SERVER_PASSWORD_MAX, Value))
        {
            return DEVICE_CommandResult_Failed;
        }
    }
    else if (DEVICE_COMMAND_CHECK(SET_IP_TCP_PORT))
    {
        E->tcpPort = VARIANT_ToUint (Value);
    }
    else if (DEVICE_COMMAND_CHECK(SET_IP_UDP_PORT))
    {
        E->udpPort = VARIANT_ToUint (Value);  
    }
    else if (DEVICE_COMMAND_CHECK(SET_SPEED))
    {
        E->cmdFullSpeed = VARIANT_ToUint (Value);
    }
    else if (DEVICE_COMMAND_CHECK(EXE_CONNECT))
    {
        E->autoRecDelay = VARIANT_ToUint (Value);
        E->autoRecTarget = 0;

        if (!connect (P, E->esp32at, ConnectStepFlags__ALL))
        {
            return DEVICE_CommandResult_Failed;
        }
    }
    else if (DEVICE_COMMAND_CHECK(EXE_POWEROFF))
    {
        OUTPUT_BitNow (OUTPUT_Bit_WirelessEnable, 0);
    }
    else
    {
        return DEVICE_CommandResult_NotHandled;
    }

    return DEVICE_CommandResult_Ok;
}


static uint32_t send (struct PACKET *const P, const uint8_t *const Data,
                      const uint32_t Octets)
{
//    struct PACKET_ESP32AT_TCP_SERVER *e =
//                                (struct PACKET_ESP32AT_TCP_SERVER *) s;
    (void) P;
    (void) Data;
    (void) Octets;

    // TODO: Send
    return 0;
}


static void atmReset (struct PACKET_ESP32AT_TCP_SERVER *const E)
{
    memset (E->atmBuf, 0, sizeof(E->atmBuf));
    E->atmUsed = 0;
}


static void atmCheckMessage (struct PACKET_ESP32AT_TCP_SERVER *const E)
{
    // expected responses, where # = connection id (one ASCII char), and 
    // $ = received data size:
    // #,CONNECT\r\n (11 octets)
    // #,CLOSED\r\n (10 octets)
    // WIFI DISCONNECT\r\n (17 octets)
    // Have # Connections\r\n (20 octets)
    // \r\n (2 octets)
    switch (E->atmUsed)
    {
        case 11:
            if (!strncmp(&E->atmBuf[1], ",CONNECT\r\n", 10))
            {

            }
            break;

        case 10:
            if (!strncmp(&E->atmBuf[1], ",CLOSED\r\n", 9))
            {

            }
            break;

        case 17:
            if (!strncmp(E->atmBuf, "WIFI DISCONNECT\r\n", 17))
            {
                E->connected = false;

                LOG (&E->device, "wifi disconnected");

                if (E->autoRecDelay)
                {
                    E->autoRecTarget = BOARD_TicksNow() + E->autoRecDelay;
                    LOG (&E->device, LANG_RETRYING_IN_MS_FMT, E->autoRecDelay);
                }
            }
            break;

        case 20:
            if (!strncmp(E->atmBuf, "Have ", 5) &&
                !strncmp(&E->atmBuf[7], " Connections\r\n", 14))
            {

            }
            break;

        case 2:
            if (!strncmp(E->atmBuf, "\r\n", 2))
            {

            }
            break;

        default:
            LOG (&E->device, LANG_UNKNOWN_RESPONSE);
            LOG_BinaryDump (&E->device, LANG_RESPONSE,
                            (uint8_t *)E->atmBuf, E->atmUsed);
            break;
    }
}


static uint32_t recvSize (struct PACKET *const P, const uint32_t MaxOctets)
{
    (void) MaxOctets;

    struct PACKET_ESP32AT_TCP_SERVER *const E =
                                (struct PACKET_ESP32AT_TCP_SERVER *) P;

    char data;

    if (!E->atmUsed)
    {
        // Get a first octet
        STREAM_OUT_ToBuffer (E->esp32at, (uint8_t *)E->atmBuf, 1);

        // No data available
        if (STREAM_OUT_Count(E->esp32at) < 1)
        {
            return 0;
        }

        E->atmUsed = 1;
    }

    // Check the first octet for the start of a "+IPD,#,$" sequence
    if (E->atmBuf[0] == '+')
    {
        // Retrieve additional 6 octets. Should read "IPD,#," but check
        // for an unexpected '\r' to resync.
        if (E->atmUsed < 7)
        {
            while (E->atmUsed < 7)
            {
                STREAM_OUT_ToBuffer (E->esp32at, (uint8_t *)&data, 1);
                if (!STREAM_OUT_Count (E->esp32at))
                {
                    return 0;
                }

                E->atmBuf[E->atmUsed] = data;
                ++ E->atmUsed;

                if (data == '\r')
                {
                    LOG_Warn (P, LANG_UNEXPECTED_IPD_BODY);
                    LOG (P, LANG_RESINCYNG);
                    LOG_BinaryDump (P, "emBuf", (uint8_t *)E->atmBuf,
                                    E->atmUsed);

                    // Resync to look for "\r\n"
                    //E->atmBuf[0] = '-';
                    atmReset (E);
                    return 0;
                }
            }

            // Check for "+IPD,#,"
            if (E->atmBuf[1] == 'I' && E->atmBuf[2] == 'P' &&
                E->atmBuf[3] == 'D' && E->atmBuf[4] == ',' &&
                E->atmBuf[5] >= '0' && E->atmBuf[5] <= '9' &&
                E->atmBuf[6] == ',')
            {
                VARIANT_SetUint (&P->recvFrom, E->atmBuf[5] - 48);
            }
            else
            {
                LOG_Warn (P, LANG_INVALID_IPD_BODY);
                LOG (P, LANG_RESINCYNG);
                LOG_BinaryDump (P, "emBuf", (uint8_t *)E->atmBuf,
                                E->atmUsed);

                // Resync to look for "\r\n"
                //E->atmBuf[0] = '-';
                atmReset (E);
                return 0;                    
            }
        }
        
        if (E->atmUsed >= 7)
        {
            while (E->atmUsed < sizeof(E->atmBuf))
            {
                STREAM_OUT_ToBuffer (E->esp32at, (uint8_t *)&data, 1);
                if (!STREAM_OUT_Count (E->esp32at))
                {
                    return 0;
                }

                E->atmBuf[E->atmUsed] = data;
                ++ E->atmUsed;

                // No digit
                if (data < '0' || data > '9')
                {
                    // End of data size field
                    if (data == ':')
                    {
                        E->atmBuf[E->atmUsed - 1] = '\0';
                        const uint32_t RecvSize =
                            VARIANT_ToUint (
                                &VARIANT_SpawnString(&E->atmBuf[7]));

                        atmReset (E);
                        return RecvSize;
                        // Assembles data packed directly to user-owned 
                        // buffer.
                        // return atmDataRead (P, Buffer, Octets);
                    }
                    // Unexpected CR.
                    else if (data == '\r')
                    {
                        LOG_Warn (P, LANG_INVALID_IPD_SIZE);
                        LOG (P, LANG_RESINCYNG);
                        LOG_BinaryDump (P, "emBuf", (uint8_t *)E->atmBuf,
                                        E->atmUsed);

                        // Resync to look for "\r\n".
                        //E->atmBuf[0] = '-';
                        atmReset (E);
                        return 0;
                    }

                    // Any other character
                    BOARD_AssertUnexpectedValue (P, (uint8_t)data);
                }
            }

            if (E->atmUsed >= sizeof(E->atmBuf))
            {
                // unexpected message length
                LOG_Warn (P, LANG_AT_MSG_BIGGER_THAN_EXPECT);
                LOG_Items (2,
                            LANG_OCTETS,        E->atmUsed,
                            LANG_BUFFER_SIZE,   (uint32_t)sizeof(E->atmBuf));
                BOARD_AssertState (false);
            }
        }
    }
    // Not "+IPD"; no data to report back to the user. Check for other 
    // ESP32 AT message.
    else
    {
        // Keep iterating until a '\n' or '+' is found
        while (E->atmUsed < sizeof(E->atmBuf))
        {
            STREAM_OUT_ToBuffer (E->esp32at, (uint8_t *)&data, 1);
            if (!STREAM_OUT_Count (E->esp32at))
            {
                return 0;
            }
            /*
            else if (data == '+')
            {
                // Resync assuming the start of an "+IPD" message
                E->emBuf[0] = data;
                E->emUsed = 1;

                return 0;
            }
            */

            E->atmBuf[E->atmUsed] = data;
            ++ E->atmUsed;

            if (data == '\n')
            {
                atmCheckMessage (E);
                atmReset (E);

                return 0;
            }
        }

        if (E->atmUsed >= sizeof(E->atmBuf))
        {
            // unexpected esp32 status message length
            // Log this to account for unhandled messages that may appear
            LOG_Warn (P, LANG_UNEXPECTEDLY_LONG_MSG);
            LOG_BinaryDump (P, "emBuf", (uint8_t *)E->atmBuf, E->atmUsed);

            atmReset (E);
        }
    }

    return 0;
}


static uint32_t recv (struct PACKET *const P, uint8_t *const Buffer,
                      const uint32_t Octets)
{
    struct PACKET_ESP32AT_TCP_SERVER *const E =
                                (struct PACKET_ESP32AT_TCP_SERVER *) P;

    const uint32_t OctetsLeft = P->recvSize - P->recvOctets;

    if (Octets > OctetsLeft)
    {
        LOG_Warn (P, LANG_REQ_READ_MORE_THAN_AVAIL);
        LOG_Items (4,   LANG_READ,          Octets,
                        LANG_PACKET_SIZE,   P->recvSize,
                        LANG_RECEIVED,      P->recvOctets,
                        LANG_LEFT,          OctetsLeft);
        BOARD_AssertState (false);
    }

    STREAM_OUT_ToBuffer (E->esp32at, Buffer, OctetsLeft);

    const uint32_t ReadResult = STREAM_OUT_Count (E->esp32at);
    const uint32_t TotalRead  = P->recvOctets + ReadResult;

    if (TotalRead == P->recvSize)
    {
        atmReset (E);
    }
    else if (TotalRead > P->recvSize)
    {
        LOG_Warn (P, LANG_TOTAL_READ_HIGH_THAN_RECV);
        LOG_Items (2,   LANG_READ,          TotalRead,
                        LANG_RECEIVED,      P->recvSize);
        BOARD_AssertState (false);
    }

    return ReadResult;
}

/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [RAWSTOR] raw storage device driver interface, fatfs compatible.

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

#include "embedul.ar/source/core/device/rawstor.h"
#include "embedul.ar/source/core/device/board.h"


void RAWSTOR_Init (struct RAWSTOR *const R, const struct RAWSTOR_IFACE *iface)
{
    BOARD_AssertParams (R && iface);

    BOARD_AssertInterface (iface->Description &&
                            iface->MediaInit &&
                            iface->MediaRead &&
                            iface->MediaWrite &&
                            iface->MediaIoctl);

    OBJECT_Clear (R);

    R->iface            = iface;

    R->status.ticks     = BOARD_TicksNow ();
    R->status.disk      = RAWSTOR_Status_Disk_START_VALUE;
    R->status.result    = RAWSTOR_Status_Result_Ok;
    R->status.media     = RAWSTOR_Status_Media_Undefined;
    R->status.mediaP1   = 0;
    R->status.mediaP2   = 0;

    LOG_ContextBegin (R, LANG_INIT);
    {
        CYCLIC_Init (&R->statusLog, R->statusLogBuffer, 
                                        sizeof(R->statusLogBuffer));

        RAWSTOR_HardwareInit (R);

        LOG_ContextBegin (R, LANG_STORAGE_MEDIA_INIT);
        {
            RAWSTOR_MediaInit (R);

            LOG_Items (1, LANG_STORAGE_MEDIA_STATUS,
                            RAWSTOR_MediaStatusString(RAWSTOR_Status(R).media));
        }
        LOG_ContextEnd ();
    }
    LOG_ContextEnd ();
}


struct RAWSTOR_Status RAWSTOR_Status (struct RAWSTOR *const R)
{
    BOARD_AssertParams (R);
    return R->status;
}


bool RAWSTOR_PeekStatusLog (struct RAWSTOR *const R, const uint32_t Entry,
                            struct RAWSTOR_Status *status)
{
    BOARD_AssertParams (R && status);

    if (!CYCLIC_Elements (&R->statusLog))
    {
        return false;
    }

    const uint32_t EntryOffset = Entry * sizeof(struct RAWSTOR_Status);

    CYCLIC_PeekToBuffer (&R->statusLog, EntryOffset, (uint8_t *) status,
                         sizeof(struct RAWSTOR_Status));
    return true;
}


void RAWSTOR_UpdateStatus (struct RAWSTOR *const R,
                           const RAWSTOR_Status_Disk Disk,
                           const RAWSTOR_Status_Result Result,
                           const RAWSTOR_Status_Media Media,
                           const uint8_t MediaP1,
                           const uint32_t MediaP2)
{
    BOARD_AssertParams (R);

    // Store last status
    CYCLIC_IN_FromBuffer (&R->statusLog, (const uint8_t *)&R->status,
                          sizeof(struct RAWSTOR_Status));

    R->status.ticks = BOARD_TicksNow ();

    if (Disk != RAWSTOR_Status_KEEP_VALUE)
    {
        R->status.disk      = Disk;
    }

    if (Result != RAWSTOR_Status_KEEP_VALUE)
    {
        R->status.result    = Result;
    }

    if (Media != RAWSTOR_Status_KEEP_VALUE)
    {
        R->status.media     = Media;
        R->status.mediaP1   = MediaP1;
        R->status.mediaP2   = MediaP2;
    }
}


const char * RAWSTOR_MediaStatusString (const RAWSTOR_Status_Media Sm)
{
    switch (Sm)
    {
        case RAWSTOR_Status_Media_Undefined:
            return LANG_UNDEFINED;

        case RAWSTOR_Status_Media_Error:
            return LANG_ERROR;

        case RAWSTOR_Status_Media_Removed:
            return LANG_REMOVED;

        case RAWSTOR_Status_Media_Inserted:
            return LANG_INSERTED;

        case RAWSTOR_Status_Media_Initializing:
            return LANG_INITIALIZING;

        case RAWSTOR_Status_Media_Ready:
            return LANG_READY;

        default:
            break;
    };

    return LANG_UNKNOWN;
}


bool RAWSTOR_IsMediaReady (struct RAWSTOR *const R)
{
    BOARD_AssertParams (R);

    if (R->status.media != RAWSTOR_Status_Media_Ready)
    {
        return false;
    }

    return true;
}


void RAWSTOR_HardwareInit (struct RAWSTOR *const R)
{
    BOARD_AssertParams (R && R->iface);

    if (R->iface->HardwareInit)
    {
        R->iface->HardwareInit (R);
    }
}


static RAWSTOR_Status_Result checkMediaReady (struct RAWSTOR *const R)
{
    if (R->status.media == RAWSTOR_Status_Media_Ready)
    {
        return RAWSTOR_Status_Result_Ok;
    }

    return R->iface->MediaInit (R);
}


RAWSTOR_Status_Result RAWSTOR_MediaInit (struct RAWSTOR *const R)
{
    BOARD_AssertParams (R && R->iface && R->iface->MediaInit);
    return checkMediaReady (R);
}


RAWSTOR_Status_Result RAWSTOR_MediaRead (struct RAWSTOR *const R,
                                         uint8_t *const Data, 
                                         const uint32_t SectorBegin,
                                         const uint32_t SectorCount)
{
    BOARD_AssertParams (R && R->iface && R->iface->MediaRead &&
                         Data && SectorCount);

    const RAWSTOR_Status_Result Res = checkMediaReady (R);
    if (Res != RAWSTOR_Status_Result_Ok)
    {
        return Res;
    }

    return R->iface->MediaRead (R, Data, SectorBegin, SectorCount);
}


RAWSTOR_Status_Result RAWSTOR_MediaWrite (struct RAWSTOR *const R,
                                          const uint8_t *const Data,
                                          const uint32_t SectorBegin,
                                          const uint32_t SectorCount)
{
    BOARD_AssertParams (R && R->iface && R->iface->MediaWrite &&
                         Data && SectorCount);

    const RAWSTOR_Status_Result Res = checkMediaReady (R);
    if (Res != RAWSTOR_Status_Result_Ok)
    {
        return Res;
    }

    return R->iface->MediaWrite (R, Data, SectorBegin, SectorCount);
}


RAWSTOR_Status_Result RAWSTOR_MediaIoctl (struct RAWSTOR *const R,
                                          const uint8_t Cmd, void *const Data)
{
    BOARD_AssertParams (R && R->iface && R->iface->MediaIoctl);

    const RAWSTOR_Status_Result Res = checkMediaReady (R);
    if (Res != RAWSTOR_Status_Result_Ok)
    {
        return Res;
    }

    return R->iface->MediaIoctl (R, Cmd, Data);
}


const char * RAWSTOR_Description (struct RAWSTOR *const R)
{
    BOARD_AssertParams (R && R->iface && R->iface->Description);
    return R->iface->Description;
}

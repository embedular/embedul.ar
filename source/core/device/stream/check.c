/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [PACKET] common error logging.

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

#include "embedul.ar/source/core/device/stream/check.h"
#include "embedul.ar/source/core/device/board.h"


static void i2cLogStatus (struct STREAM *const S, const char *const Title)
{
    LOG_WarnDebug (S, Title);
    LOG_Items (5,
                LANG_TYPE,          (uint32_t)S->type,
                LANG_ADDRESS,       &S->address,
                LANG_TIMEOUT,       S->timeout,
                LANG_OCTETS,        S->count,
                LANG_ERROR_CODE,    (uint32_t)S->status,
                LOG_ItemsBases(0, VARIANT_Base_Hex_UpperSuffix, 0, 0, 0));
}


bool STREAM_CHECK_I2cControllerXferStatus (struct STREAM *const S)
{
    BOARD_AssertParams (STREAM_IsValid(S));

    const enum STREAM_TransferStatus Status = STREAM_TransferStatus (S);

    switch (Status)
    {
        case STREAM_TransferStatus_Ok:
        case STREAM_TransferStatus_Stopped:
            return true;

        case STREAM_TransferStatus_NoAck:
            i2cLogStatus (S, LANG_DEVICE_NOT_FOUND);
            return false;

        default:
            i2cLogStatus (S, LANG_COMMUNICATION_ERROR);
    }

    return false;
}

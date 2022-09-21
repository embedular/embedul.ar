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

#include "embedul.ar/source/core/device/packet/error_log.h"
#include "embedul.ar/source/core/device/board.h"


static void logGenericError (struct PACKET *const P)
{
    LOG_WarnDebug (P, LANG_COMMUNICATION_ERROR);
    LOG_Items (2, 
                LANG_DEVICE_ADDRESS,    &P->sendTo, 
                LANG_ERROR_CODE,        PACKET_Error(P),
                LOG_ItemsBases(VARIANT_Base_Hex_UpperSuffix,
                                0));
}


bool PACKET_ERROR_LOG_nAck (struct PACKET *const P)
{
    BOARD_AssertParams (PACKET_IsValid(P));

    // No error
    if (!PACKET_Error (P))
    {
        return false;
    }

    // nAck
    if (PACKET_Error(P) == PACKET_Errors_NoAck)
    {
        LOG_WarnDebug (P, LANG_DEVICE_NOT_FOUND);
        LOG_Items (1, LANG_DEVICE_ADDRESS, &P->sendTo, 
                    LOG_ItemsBases(VARIANT_Base_Hex_UpperSuffix));
    }
    // Any other error
    else
    {
        logGenericError (P);
    }

    return true;
}


bool PACKET_ERROR_LOG_Generic (struct PACKET *const P)
{
    BOARD_AssertParams (PACKET_IsValid(P));    

    // No error
    if (!PACKET_Error (P))
    {
        return false;        
    }

    // Any error
    logGenericError (P);

    return true;
}

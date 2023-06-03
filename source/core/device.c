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

#include "embedul.ar/source/core/device.h"
#include "embedul.ar/source/core/device/board.h"


enum DEVICE_CommandResult DEVICE_Command (
                            const char *const ContextMsg,
                            struct OBJECT_INFO *const DevInfo,
                            const DEVICE_CommandFunc DevCommandFunc,
                            const char *const Name,
                            struct VARIANT *const Value)
{
    enum DEVICE_CommandResult sr = DEVICE_CommandResult_Ok;

    LOG_ContextBegin (DevInfo, ContextMsg);
    {
        if (!DevCommandFunc)
        {
            // By design, it is allowed for a device implementation not to
            // handle commands.
            LOG_Warn (DevInfo, LANG_METHOD_NOT_IMPLEMENTED);
            LOG_ContextEnd ();
            return DEVICE_CommandResult_NoMethod;
        }

        // Any device command must follow the regular expression
        // template "(s|g|x)|[a-zA-Z0-9]+" where s = Set, g = Get, x = Execute.
        if (strlen(Name) < 2
            || (Name[0] != 's' && Name[0] != 'g' && Name[0] != 'x')
            || Name[1] != '|')
        {
            LOG_Warn (DevInfo, LANG_INVALID_COMMAND_FORMAT_FMT, Name);
            BOARD_AssertParams (false);
        }

        struct VARIANT OrigValue = *Value;

        switch (Name[0])
        {
            case 's':
                LOG_Items (2,
                            LANG_SET,   Name,
                            LANG_VALUE, VARIANT_ToString(Value));
                break;

            case 'g':
                LOG_Items (1, LANG_GET, Name);
                break;

            case 'x':
                LOG_Items (1, LANG_EXECUTE, Name);
                break;
        }

        sr = DevCommandFunc (DevInfo->Ptr, Name, Value);

        switch (sr)
        {
            case DEVICE_CommandResult_NotHandled:
            {
                LOG_Warn (DevInfo, LANG_UNHANDLED_COMMAND);
                // By design, it is allowed for a device implementation not to
                // handle every possible command.
                break;
            }

            case DEVICE_CommandResult_Failed:
            {
                LOG_Warn (DevInfo, LANG_COMMAND_FAILED);
                // The application should decide if this represents a critical
                // error, how many times to retry, etc.
                break;
            }

            case DEVICE_CommandResult_Ok:
            {
                switch (Name[0])
                {
                    case 's':
                        // The command handler at the device implementation may
                        // adjust `Value` to the actual value that was set.
                        if (VARIANT_IsEqual(&OrigValue, Value))
                        {
                            LOG (DevInfo, LANG_COMMAND_SAME_VALUE_SET);
                        }
                        else 
                        {
                            LOG_Warn (DevInfo, LANG_COMMAND_MOD_VALUE_SET_FMT,
                                      Value);
                        }
                        break;

                    case 'g':
                        // The command handler at the device implementation
                        // returns the requested data by modifying `Value`.
                        LOG (DevInfo, LANG_COMMAND_GOT_VALUE_FMT, Value);
                        break;

                    case 'x':
                        // The command handler at the device implementation
                        // returns the executed task result by modifying
                        // `Value`.
                        LOG (DevInfo, LANG_COMMAND_EXECUTED_RET_FMT, Value);
                        break;
                }
                break;
            }

            default:
                BOARD_AssertUnexpectedValue (DevInfo, (uint32_t)sr);
                break;
        }
    }
    LOG_ContextEnd ();

    return sr;
}

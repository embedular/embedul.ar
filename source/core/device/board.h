/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [BOARD] platform device interface (singleton).

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

#include "embedul.ar/source/core/timer.h"
#include "embedul.ar/source/core/variant.h"
#include "embedul.ar/source/core/ansi.h"
#include "embedul.ar/source/core/object.h"
#include "embedul.ar/source/core/device.h"
#include "embedul.ar/source/core/device/random.h"
#include "embedul.ar/source/core/device/video.h"
#include "embedul.ar/source/core/device/sound.h"
#include "embedul.ar/source/core/manager/comm.h"
#include "embedul.ar/source/core/manager/log.h"
#include "embedul.ar/source/core/manager/input.h"
#include "embedul.ar/source/core/manager/output.h"
#include "embedul.ar/source/core/manager/storage.h"
#include "embedul.ar/source/core/lang/en.h"
#include <string.h>


#define UNSUPPORTED         NULL


//#define DESCRIPTION(s)  ((s)? s : DescriptionNA)

#define BOARD_Assert(_c,_s) \
    BOARD_AssertContext (__func__, __FILE__, __LINE__, _c, _s)

#define BOARD_AssertParams(_c) \
    BOARD_Assert (_c, LANG_ASSERT_INVALID_PARAMS)

#define BOARD_AssertInterface(_c) \
    BOARD_Assert (_c, LANG_ASSERT_INVALID_INTERFACE)

#define BOARD_AssertAccess(_c) \
    BOARD_Assert (_c, LANG_ASSERT_INVALID_ACCESS)

#define BOARD_AssertState(_c) \
    BOARD_Assert (_c, LANG_ASSERT_INVALID_STATE)

#define BOARD_AssertInitialized(_c) \
    BOARD_Assert (_c, LANG_ASSERT_NOT_INITIALIZED)

#define BOARD_AssertSupported(_c) \
    BOARD_Assert (_c, LANG_ASSERT_UNSUPPORTED)

#define BOARD_AssertUnexpectedValue(_d,_v) \
    LOG_Warn (_d, LANG_UNEXPECTED_VALUE); \
    LOG_Items (1, LANG_VALUE, _v, LOG_ItemsBases(VARIANT_Base_Hex)); \
    BOARD_AssertState (false);


enum BOARD_Stage
{
    BOARD_Stage_InitHardware,
    BOARD_Stage_InitDebugStreamDriver,      // STREAM
    BOARD_Stage_Greetings,
    BOARD_Stage_InitIOProfiles,
    BOARD_Stage_InitRandomDriver,           // RANDOM
    BOARD_Stage_InitIOLevel1Drivers,        // IO(s) with no dependencies
    BOARD_Stage_InitCommDrivers,            // STREAM(s)
    BOARD_Stage_InitStorageDrivers,         // RAWSTOR(s)
    BOARD_Stage_InitIOLevel2Drivers,        // IO(s) that depends on Comm or 
                                            // Storage to function properly
    BOARD_Stage_InitVideoDriver,            // VIDEO
    BOARD_Stage_InitSoundDriver,            // SOUND
    BOARD_Stage_Ready,
    BOARD_Stage_Shutdown
};


struct BOARD_RealTimeClock
{
    uint16_t    dayOfYear;
    uint16_t    year;
    uint8_t     dayOfMonth;
    uint8_t     month;
    uint8_t     dayOfWeek;
    uint8_t     hour;
    uint8_t     minute;
    uint8_t     second;
};


enum BOARD_TickHookFuncSlot
{
    BOARD_TickHookFuncSlot_BSP = 0,
    BOARD_TickHookFuncSlot_OS,
    BOARD_TickHookFuncSlot_App,
    BOARD_TickHookFuncSlot__COUNT
};


struct BOARD;

typedef void *      (* BOARD_StageChangeFunc)(struct BOARD *const B,
                                    const enum BOARD_Stage Stage);
typedef void        (* BOARD_AssertFunc)(struct BOARD *const B,
                                    const bool Condition);
typedef void        (* BOARD_SetTickFreqFunc)(struct BOARD *const B,
                                    const uint32_t Hz);
typedef TIMER_TickHookFunc
                    (* BOARD_SetTickHookFunc)(struct BOARD *const B,
                                    TIMER_TickHookFunc const Func,
                                    const enum BOARD_TickHookFuncSlot Slot);
typedef TIMER_Ticks (* BOARD_TicksNowFunc)(struct BOARD *const B);
typedef struct BOARD_RealTimeClock
                    (* BOARD_RealTimeClockFunc)(struct BOARD *const B);
typedef void        (* BOARD_DelayFunc)(struct BOARD *const B,
                                    const TIMER_Ticks Ticks);
typedef void        (* BOARD_UpdateFunc)(struct BOARD *const B);


struct BOARD_IFACE
{
    const char                      * const Description;
    const BOARD_StageChangeFunc     StageChange;
    const BOARD_AssertFunc          Assert;
    const BOARD_SetTickFreqFunc     SetTickFreq;
    const BOARD_SetTickHookFunc     SetTickHook;
    const BOARD_TicksNowFunc        TicksNow;
    const BOARD_RealTimeClockFunc   Rtc;
    const BOARD_DelayFunc           Delay;
    const BOARD_UpdateFunc          Update;
};


struct BOARD
{
    const struct BOARD_IFACE        * iface;
    struct BOARD_RIG                * rig;
    TIMER_TickHookFunc              * tickHookFunc;
    uint32_t                        tickFrequency;
    struct STREAM                   * debugStream;
    struct RANDOM                   * random;
    struct VIDEO                    * video;
    struct SOUND                    * sound;
    // Already allocated managers
    struct COMM                     comm;
    struct LOG                      log;
    struct INPUT                    input;
    struct OUTPUT                   output;
    struct STORAGE                  storage;
    enum BOARD_Stage                currentStage;
};


struct BOARD_RIG;

typedef void *      (* BOARD_RIG_StageChangeFunc)(struct BOARD_RIG *const R,
                                    const enum BOARD_Stage Stage);


struct BOARD_RIG_IFACE
{
    const char                      * const Description;
    const BOARD_RIG_StageChangeFunc StageChange;
};


struct BOARD_RIG
{
    const struct BOARD_RIG_IFACE    * iface;
};


const char *        BOARD_Init              (struct BOARD *const B,
                                             const struct BOARD_IFACE *const
                                             Iface, struct BOARD_RIG *const R);
enum BOARD_Stage    BOARD_CurrentStage      (void);
void                BOARD_AssertContext     (const char *const Func,
                                             const char *const File,
                                             const int Line,
                                             const bool Condition,
                                             const char *const Str);
void                BOARD_SetTickFreq       (const uint32_t Hz);
uint32_t            BOARD_TickFreq          (void);
TIMER_TickHookFunc  BOARD_SetTickHook       (TIMER_TickHookFunc const Func,
                                             const enum BOARD_TickHookFuncSlot
                                             Slot);
TIMER_Ticks         BOARD_TicksNow          (void);
struct BOARD_RealTimeClock
                    BOARD_RealTimeClock     (void);
void                BOARD_Delay             (const TIMER_Ticks Ticks);
bool                BOARD_VideoDeviceOk     (void);
bool                BOARD_SoundDeviceOk     (void);
const char *        BOARD_Description       (void);
void                BOARD_Update            (void);

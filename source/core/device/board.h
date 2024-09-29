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

#include "embedul.ar/source/core/device/oswrap.h"
#include "embedul.ar/source/core/device/ticks.h"
#include "embedul.ar/source/core/device/random.h"
#include "embedul.ar/source/core/device/sound.h"
#include "embedul.ar/source/core/manager/comm.h"
#include "embedul.ar/source/core/manager/log.h"
#include "embedul.ar/source/core/manager/mio.h"
#include "embedul.ar/source/core/manager/storage.h"
#include "embedul.ar/source/core/manager/screen.h"
#include "embedul.ar/source/core/lang/en.h"
#include <string.h>


#define BOARD_Assert(_c,_s) \
    BOARD_AssertContext (__func__, __FILE__, __LINE__, (_c)? true : false, _s)

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
    BOARD_AssertState (false)


enum BOARD_Stage
{
    BOARD_Stage_InitPreTicksHardware,
    BOARD_Stage_InitPostTicksHardware,
    BOARD_Stage_InitDebugStreamDriver,      // STREAM
    BOARD_Stage_Greetings,
    BOARD_Stage_InitIOProfiles,
    BOARD_Stage_InitRandomDriver,           // RANDOM
    BOARD_Stage_InitIOLevel1Drivers,        // IO(s) with no dependencies
    BOARD_Stage_InitCommDrivers,            // STREAM(s)
    BOARD_Stage_InitStorageDrivers,         // RAWSTOR(s)
    BOARD_Stage_InitIOLevel2Drivers,        // IO(s) that depends on Comm or 
                                            // Storage to function properly
    BOARD_Stage_InitScreenDrivers,          // VIDEO(s)
    BOARD_Stage_InitSoundDriver,            // SOUND
    BOARD_Stage_InitIOLevel3Drivers,        // IO(s) that depends on a given
                                            // Screen to function properly
    BOARD_Stage_Ready,
    BOARD_Stage_ShutdownHardware
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


struct BOARD;

typedef void *      (* BOARD_StageChangeFunc)(struct BOARD *const B,
                                              const enum BOARD_Stage Stage);
typedef void        (* BOARD_AssertFunc)(struct BOARD *const B,
                                         const bool Condition);
typedef struct BOARD_RealTimeClock
                    (* BOARD_RealTimeClockFunc)(struct BOARD *const B);
typedef void        (* BOARD_UpdateFunc)(struct BOARD *const B);


struct BOARD_IFACE
{
    const char                      *const Description;
    const BOARD_StageChangeFunc     StageChange;
    const BOARD_AssertFunc          Assert;
    const BOARD_RealTimeClockFunc   Rtc;
    const BOARD_UpdateFunc          Update;
};


struct BOARD
{
    const struct BOARD_IFACE        * iface;
    int                             argc;
    const char                      ** argv;
    int                             returnValue;
    enum BOARD_Stage                currentStage;
    uint32_t                        inhibitAssert;
    bool                            exit;
    // Singleton device drivers
    struct TICKS                    * ticks;
    struct BOARD_RIG                * rig;
    struct STREAM                   * debugStream;
    struct RANDOM                   * random;
    struct SOUND                    * sound;
    // Already allocated managers for device drivers
    struct COMM                     comm;
    struct LOG                      log;
    struct MIO                      mio;
    struct STORAGE                  storage;
    struct SCREEN                   screen;
};


struct BOARD_RIG;

typedef void *      (* BOARD_RIG_StageChangeFunc)(struct BOARD_RIG *const R,
                                    const enum BOARD_Stage Stage);


struct BOARD_RIG_IFACE
{
    const char                      *const Description;
    const BOARD_RIG_StageChangeFunc StageChange;
};


struct BOARD_RIG
{
    const struct BOARD_RIG_IFACE    * iface;
};


const char *        BOARD_Init              (struct BOARD *const B,
                                             const struct BOARD_IFACE *const
                                             Iface, const int Argc,
                                             const char **const Argv,
                                             struct BOARD_RIG *const R);
int                 BOARD_Argc              (void);
const char **       BOARD_Argv              (void);
int                 BOARD_ReturnValue       (void);
enum BOARD_Stage    BOARD_CurrentStage      (void);
void                BOARD_AssertContext     (const char *const Func,
                                             const char *const File,
                                             const int Line,
                                             const bool Condition,
                                             const char *const Str);
struct BOARD_RealTimeClock
                    BOARD_RealTimeClock     (void);
bool                BOARD_SoundDeviceOk     (void);
void                BOARD_Exit              (const int ReturnValue);
const char *        BOARD_Description       (void);
void                BOARD_Sync              (void);

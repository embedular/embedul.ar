/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [MANAGER] system logging functions.

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

#include "embedul.ar/source/core/device.h"
#include "embedul.ar/source/core/device/stream.h"
#include "embedul.ar/source/core/ansi.h"


#define LOG_BASE_COLOR              ANSI_SGR_SET_NFG(WHITE)
#define LOG_BASE_BOLD_COLOR         ANSI_SGR_SET_BFG(WHITE)
#define LOG_ASSERT_COLOR            ANSI_SGR_SET_NFG(RED)
#define LOG_WARNING_COLOR           ANSI_SGR_SET_NFG(RED)
#define LOG_PENDING_COLOR           ANSI_SGR_SET_NFG(BRIGHT_YELLOW)
#define LOG_OK_COLOR                ANSI_SGR_SET_NFG(GREEN)
#define LOG_FAILED_COLOR            ANSI_SGR_SET_NFG(RED)
#define LOG_BASE_BOLD(x)            LOG_BASE_BOLD_COLOR x LOG_BASE_COLOR
#define LOG_ASSERT_STR(x)           LOG_ASSERT_COLOR x LOG_BASE_COLOR
#define LOG_WARNING_STR(x)          LOG_WARNING_COLOR x LOG_BASE_COLOR
#define LOG_PENDING_STR(x)          LOG_PENDING_COLOR x LOG_BASE_COLOR
#define LOG_OK_STR(x)               LOG_OK_COLOR x LOG_BASE_COLOR
#define LOG_FAILED_STR(x)           LOG_FAILED_COLOR x LOG_BASE_COLOR
#define LOG_PREFIX_ASSERT_STR       LOG_ASSERT_STR("[ASSERT FAILED]") " "
#define LOG_PREFIX_WARNING_STR      LOG_WARNING_STR("⚠") " "
#define LOG_PREFIX_PENDING_STR      LOG_PENDING_STR("⧖") " "
#define LOG_PREFIX_ITEM_STR         "⚙ " LOG_BASE_COLOR
#define LOG_SUFFIX_NEWLINE_STR      "\r\n"
#define LOG_SUFFIX_DOT_NEWLINE_STR  "." LOG_SUFFIX_NEWLINE_STR
#define LOG_SUFFIX_PENDING_STR      "… "
#define LOG_PENDING_OK_STR          LOG_OK_STR("✔")
#define LOG_PENDING_FAILED_STR      LOG_FAILED_STR("✖")
//#define LOG_GROUP_FMT               "`S1─`F15`S1━`M40`U0`F65`S1─`F80`L"
#define LOG_BINARY_DUMP_FMT         "binary dump: `0, `1 octets"
#define LOG_LINE_TIMING_ONLY        0
#define LOG_LINE_NO_TIMING          -1
#define LOG_ITEMS_MAX               6U
#define LOG_CONTEXT_TICKS_DEPTH     12U


#define LOG(_dp,_msg,...) \
    OBJECT_AssertValid(_dp); \
    LOG_Args (NULL, NULL, NULL, LOG_LINE_TIMING_ONLY, \
              &OBJECT_INFO_Spawn(_dp), \
              NULL, LOG_SUFFIX_DOT_NEWLINE_STR, \
              _msg, VARIANT_AutoParams(__VA_ARGS__))

#define LOG_ContextBegin(_dp,_msg,...) \
    OBJECT_AssertValid(_dp); \
    LOG__contextBegin(); \
    LOG_Args ("┌", NULL, NULL, LOG_LINE_TIMING_ONLY, \
              &OBJECT_INFO_Spawn(_dp), \
              NULL, LOG_SUFFIX_DOT_NEWLINE_STR, \
              _msg, VARIANT_AutoParams(__VA_ARGS__)) \

#define LOG_AutoContext(_dp,_msg,...) \
    LOG_ContextBegin(_dp,_msg,__VA_ARGS__); \
    __attribute__((cleanup(LOG__autoContextEnd))) uint32_t log_ac__ = 0; \
    while (log_ac__) {}

#define LOG_Debug(_dp,_msg,...) \
    OBJECT_AssertValid(_dp); \
    LOG_Args (NULL, __func__, __FILE__, __LINE__, \
              &OBJECT_INFO_Spawn(_dp), \
              NULL, LOG_SUFFIX_DOT_NEWLINE_STR, \
              _msg, VARIANT_AutoParams(__VA_ARGS__))

#define LOG_Warn(_dp,_msg,...) \
    OBJECT_AssertValid(_dp); \
    LOG_Args (NULL, NULL, NULL, LOG_LINE_TIMING_ONLY, \
              &OBJECT_INFO_Spawn(_dp), \
              LOG_PREFIX_WARNING_STR, LOG_SUFFIX_DOT_NEWLINE_STR, \
              _msg, VARIANT_AutoParams(__VA_ARGS__))

#define LOG_WarnDebug(_dp,_msg,...) \
    OBJECT_AssertValid(_dp); \
    LOG_Args (NULL, __func__, __FILE__, __LINE__, \
              &OBJECT_INFO_Spawn(_dp), \
              LOG_PREFIX_WARNING_STR, LOG_SUFFIX_DOT_NEWLINE_STR, \
              _msg, VARIANT_AutoParams(__VA_ARGS__))

#define LOG_Plain(_msg,...) \
    OBJECT_AssertValid(_dp); \
    LOG_Args (NULL, NULL, NULL, LOG_LINE_NO_TIMING, \
              NULL, \
              NULL, LOG_SUFFIX_NEWLINE_STR, \
              _msg, VARIANT_AutoParams(__VA_ARGS__))


#define LOG_PendingBegin(_dp,_msg,...) \
    OBJECT_AssertValid(_dp); \
    LOG_Args (NULL, NULL, NULL, LOG_LINE_TIMING_ONLY, \
              &OBJECT_INFO_Spawn(_dp), \
              LOG_PREFIX_PENDING_STR, LOG_SUFFIX_PENDING_STR, \
              _msg, VARIANT_AutoParams(__VA_ARGS__))

#define LOG_BinaryDump(_dp,_title,_data,_octets) \
    LOG_ContextBegin (_dp, LOG_BINARY_DUMP_FMT, _title, _octets); \
    LOG__binaryDump (_data, _octets); \
    LOG_ContextEnd ();

#define LOG_TableEntry(_t,...) \
    LOG_TableEntryArgs (_t, VARIANT_AutoParams(__VA_ARGS__))

#define LOG_Items(_count,...) \
    LOG_ItemsArg (true,_count, VARIANT_AutoParams(__VA_ARGS__))

#define LOG_Newline() \
    LOG_Args (NULL, NULL, NULL, LOG_LINE_TIMING_ONLY, \
              NULL, \
              NULL, LOG_SUFFIX_NEWLINE_STR, \
              "", NULL, 0)


#define LOG_IB_8(a,...) ((uint32_t)(a)),LOG_IB_7(__VA_ARGS__)
#define LOG_IB_7(a,...) ((uint32_t)(a)),LOG_IB_6(__VA_ARGS__)
#define LOG_IB_6(a,...) ((uint32_t)(a)),LOG_IB_5(__VA_ARGS__)
#define LOG_IB_5(a,...) ((uint32_t)(a)),LOG_IB_4(__VA_ARGS__)
#define LOG_IB_4(a,...) ((uint32_t)(a)),LOG_IB_3(__VA_ARGS__)
#define LOG_IB_3(a,...) ((uint32_t)(a)),LOG_IB_2(__VA_ARGS__)
#define LOG_IB_2(a,...) ((uint32_t)(a)),LOG_IB_1(__VA_ARGS__)
#define LOG_IB_1(a)     ((uint32_t)(a))
#define LOG_IB_0        _Static_assert(0, LANG_ASSERT_INVALID_PARAMS);

#define LOG_ItemsBases(...) \
        CC_ExpPaste(LOG_IB_,CC_ArgsCount(__VA_ARGS__))(__VA_ARGS__)


struct LOG_TableItem
{
    const char                  * const Name;
    const uint8_t               RowEnd;
    const enum VARIANT_Base     UintBase;
};


struct LOG_Table
{
    const char                  * const Title;
    const uint32_t              FieldCount;
    const struct LOG_TableItem  Fields[];
};


struct LOG_TableStyle
{
    const char      * const TableBegin;
    const char      * const TableTitle;
    const char      * const HBorders[3][5];
    const char      * const VBorder;
    const char      * const BorderField;
    const char      * const BorderEnd;
    const char      * const EntrySpacing;
    const char      * const EntryField;
    const char      * const EntryEnd;
};


struct LOG_ItemsStyle
{
    const char      * const Items[LOG_ITEMS_MAX];
};


enum LOG_ProgressType
{
    LOG_ProgressType_None = 0,
    LOG_ProgressType_Work,
    LOG_ProgressType_Timeout,
    LOG_ProgressType__COUNT
};


struct LOG_ProgressStyle
{
    uint32_t        Max; 
    const char      * const Type[LOG_ProgressType__COUNT];
    const char      * const Begin;
    const char      * const Update;
    const char      * const End;
};


struct LOG
{
    struct STREAM                   * debugStream;
    const struct LOG_TableStyle     * logTableStyle;
    const struct LOG_ItemsStyle     * logItemsStyle;
    const struct LOG_ProgressStyle  * logProgressStyle;
    uint32_t                        progressStartColumn;
    uint32_t                        contextIndent;
    TIMER_Ticks                     contextStartTicks[LOG_CONTEXT_TICKS_DEPTH];
};


struct BOARD;

void        LOG_Init            (struct LOG *const L, struct BOARD *const Board,
                                 struct STREAM *const DebugStream);
void        LOG_Args            (const char *const InnerFlow,
                                 const char *const Func,
                                 const char *const File, const int Line,
                                 const struct OBJECT_INFO *const DevInfo,
                                 const char *const Prefix,
                                 const char *const Suffix,
                                 const char *const Msg,
                                 struct VARIANT *const ArgValues,
                                 const uint32_t ArgCount);
void        LOG_ItemsStyle      (const struct LOG_ItemsStyle *const Style);
void        LOG_ItemsArg        (const bool Timestamp, const uint32_t Items,
                                 struct VARIANT *const ArgValues,
                                 const uint32_t ArgCount);
void        LOG_TableStyle      (const struct LOG_TableStyle *const Style);
void        LOG_TableBegin      (const struct LOG_Table *const Table);
void        LOG_TableEntryArgs  (const struct LOG_Table *const Table,
                                 struct VARIANT *const ArgValues,
                                 const uint32_t ArgCount);
void        LOG_TableEnd        (const struct LOG_Table *const Table);
void        LOG_ProgressStyle   (const struct LOG_ProgressStyle *const Style);
uint32_t    LOG_ProgressMax     (void);
uint32_t    LOG_ProgressBegin   (const enum LOG_ProgressType Type);
uint32_t    LOG_ProgressUpdate  (const uint32_t OutColumn,
                                 const uint32_t Progress);
void        LOG_ProgressEnd     (void);
void        LOG_PendingEndOk    (void);
void        LOG_PendingEndFail  (void);
void        LOG__contextBegin   (void);
void        LOG_ContextEnd      (void);
void        LOG__autoContextEnd (uint32_t *const Lac);
void        LOG__binaryDump     (const uint8_t *const Data,
                                 const uint32_t Octets);

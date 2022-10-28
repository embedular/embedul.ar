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

#include "embedul.ar/source/core/device/board.h"


#define LOG_ARG_FMT_MAX_SIZE        512
#define LOG_DEVICE_FMT              "`0" OBJECT_TYPE_SEPARATOR "`1: "

static struct LOG * s_l = NULL;


#define outStrAutoArgs(_stream,_outcol,_str,...) \
    outStrArgs (_stream,_outcol,_str, \
                VARIANT_AutoParams(__VA_ARGS__))

const struct LOG_ItemsStyle s_DefaultLogItemsStyle =
{
    .Items = 
    {
        "`l0",
        "`l0: `^1`&1`1",
        "`l0: `^1`&1`1 • `^0`&0`l2: `^1`&1`3",
        "`l0: `^1`&1`1 • `^0`&0`l2: `^1`&1`3 • `^0`&0`l4: `^1`&1`5",
        "`l0: `^1`&1`1 • `^0`&0`l2: `^1`&1`3 • `^0`&0`l4: `^1`&1`5"
                                           " • `^0`&0`l6: `^1`&1`7",
        "`l0: `^1`&1`1 • `^0`&0`l2: `^1`&1`3 • `^0`&0`l4: `^1`&1`5"
                     " • `^0`&0`l6: `^1`&1`7 • `^0`&0`l8: `^1`&1`9"
    }
};


const struct LOG_TableStyle s_DefaultLogTableStyle =
{
    .TableBegin     = " ╭`S1─`F77╮`L",
    // `0: Title
    .TableTitle     = " │`M40`0`F77│`L",
    .HBorders       = { //[0]  [1]  [2]  [3]  [4]
                        { " ", "╞", "╤", "╡", "═" },   // [T]op
                        { " ", "├", "┼", "┤", "╌" },   // [M]iddle
                        { " ", "└", "┴", "┘", "─" },   // [B]ottom
                      },
    .VBorder        = "│",
    // `0: Borders[T|M|B] [1|2]
    // `1: Borders[T|M|B] [4]
    // `2: LOG_TableItem.RowEnd
    .BorderField    = "`0`$`S1`F2",
    // `0: Borders[T|M|B] [3]
    // `1: Borders[T|M|B] [4]
    .BorderEnd      = "`$`S1`#`F77`0`L",
    .EntrySpacing   = " ",
    // `0: BorderVert
    // `1: ArgValues[i]
    // `2: LOG_TableItem.RowEnd
    .EntryField     = "`0`1`$`F2",
    // `0: BorderVert
    .EntryEnd       = "`F77`0`L"
};


const struct LOG_ProgressStyle s_DefaultLogProgressStyle =
{
    .Max        = 74,
    .Type       = {
                    [LOG_ProgressType_None] = " ",
                    [LOG_ProgressType_Work] = "⛏",
                    [LOG_ProgressType_Timeout] = "⏲"
                  },
    // `0: Type [None|Work|Timeout]
    .Begin      = "`0 ", //"  `F77│`0\r |",
    // `0: Progress
    .Update     = "`S1▒`$`F0",
    .End        = "\r`F80\r"
};


void LOG_Init (struct LOG *const L, struct BOARD *const Board,
               struct STREAM *const DebugStream)
{
    BOARD_AssertState  (!s_l);
    BOARD_AssertParams (L && STREAM_IsValid(DebugStream));

    OBJECT_Clear (L);

    L->debugStream      = DebugStream;
    L->logTableStyle    = &s_DefaultLogTableStyle;
    L->logItemsStyle    = &s_DefaultLogItemsStyle;
    L->logProgressStyle = &s_DefaultLogProgressStyle;

    s_l = L;

    LOG_ContextBegin (Board, LANG_BOARD_INIT_SEQUENCE);

    {
        LOG_AutoContext (L, LANG_INIT);

        LOG_Items (2,
                    LANG_DEBUG_STREAM,      STREAM_Description(DebugStream),
                    LANG_MAX_LOG_ITEMS,     (uint32_t)LOG_ITEMS_MAX);
    }
}


inline static void outStr (struct STREAM *const S, const char *const Str)
{
    STREAM_IN_FromString (S, Str);
}


inline static uint32_t outStrArgs (struct STREAM *const S,
                                   const uint32_t OutColumn,
                                   const char *const Str,
                                   struct VARIANT *const ArgValues,
                                   const uint32_t ArgCount)
{
    const uint32_t LastOutColumn =
        STREAM_IN_FromParsedStringArgs (S, OutColumn, LOG_ARG_FMT_MAX_SIZE, Str,
                                  ArgValues, ArgCount);
    return LastOutColumn;
}


inline static void outContextColorByLevel (struct STREAM *const S,
                                           const uint32_t Level)
{
    // Here zero is the first level
    // 🌈
    switch (Level % 6)
    {
        case 0: outStr (S, ANSI_SGR_SET_NFG(MAGENTA));
                break;
        case 1: outStr (S, ANSI_SGR_SET_NFG(BLUE));
                break;
        case 2: outStr (S, ANSI_SGR_SET_NFG(CYAN));
                break;
        case 3: outStr (S, ANSI_SGR_SET_NFG(GREEN));
                break;
        case 4: outStr (S, ANSI_SGR_SET_NFG(BRIGHT_YELLOW));
                break;
        case 5: outStr (S, ANSI_SGR_SET_NFG(RED));
                break;
    }
}


inline static void outContextColor (struct STREAM *const S)
{
    if (s_l->contextIndent)
    {
        outContextColorByLevel (S, s_l->contextIndent - 1);
    }
    else 
    {
        outStr (S, LOG_BASE_COLOR);
    }    
}


inline static void outContextLevel (struct STREAM *const S,
                                    const char *const OuterFlow,
                                    const char *const InnerFlow,
                                    const bool KeepLevelColor)
{
    const char *const ActualOuterFlow = OuterFlow? OuterFlow : "╎";
    const char *const ActualInnerFlow = InnerFlow? InnerFlow : "│";

    // No contextual indentation if there is still no log manager
    if (!s_l)
    {
        return;
    }

    if (s_l->contextIndent)
    {
        for (uint32_t i = 0; i < s_l->contextIndent - 1; ++i)
        {
            outContextColorByLevel (S, i);
            outStr (S, ActualOuterFlow);
        }

        outContextColorByLevel (S, s_l->contextIndent - 1);
        outStr (S, ActualInnerFlow);
    }

    if (!s_l->contextIndent || !KeepLevelColor)
    {
        outStr (S, LOG_BASE_COLOR);
    }
}


static void outContextTicks (struct STREAM *const S,
                             const TIMER_Ticks Ticks)
{
    struct VARIANT ticks    = VARIANT_SpawnInt (Ticks);
    const uint32_t TicksLen = VARIANT_StringOctetCount(&ticks) - 1;

    // Add blank spaces to accomodate the first 16 minutes (6 digits = 999999
    // max milliseconds) so that the timestamp field remains fixed in length.
    if (TicksLen < 6)
    {
        for (uint32_t i = 0; i < 6 - TicksLen; ++i)
        {
            STREAM_IN_FromOctet (S, ' ');
        }
    }

    // Logs after the first 16 minutes will adjust its timestamp length field
    // according to the actual timestamp since startup. Note that 7 
    // digits represents almost 3 hours, and 8 digits, more than a day.
    outStr (S, VARIANT_ToString(&ticks));
}


// This function displays assertion failures, even before MESSAGE 
// initialization. It is critical to assess the likelihood of failure on
// anything executed in this function; any assertion failure while preparing
// a previous assertion output will lead to an infinite loop.
static void outContextInfo (struct STREAM *const S, 
                            const char *const TimingBegin,
                            const char *const TimingEnd,
                            const TIMER_Ticks Ticks, const char *const Prefix,
                            const char *const Func, const char *const File,
                            const int Line)
{
    if (Line < 0)
    {
        return;
    }

    if (TimingBegin)
    {
        outStr (S, TimingBegin);
    }

    outContextTicks (S, Ticks);

    if (TimingEnd)
    {
        outStr (S, TimingEnd);
    }

    outStr (S, " ");

    if (Prefix)
    {
        outStr (S, Prefix);
        outContextColor (S);
        outStr (S, " ");
    }

    if (File)
    {
        outStr (S, "[");
        outStr (S, File);
        outStr (S, ":");
        outStr (S, VARIANT_ToString(&VARIANT_SpawnInt(Line)));

        if (Func)
        {
            outStr (S, " ");
            outStr (S, Func);
            outStr (S, "()]");
        }

        outStr (S, " ");
    }
}


// BOARD_Assert may call this function even before the log subsystem
// initializes. Also, log initialization itself may fail an assertion.
// That is why the caller passes the stream used instead of assuming
// s_l->debugStream or even s_l are good. out* functions not used here may
// still take a stream for consistency.
void LOG__assertFailed (struct STREAM *const S, const char *const Func,
                        const char *const File, const int Line,
                        const char *const Msg)
{
    // Visually closing current context levels, if any
    outContextLevel (S, "x", "x", true);
    outContextInfo  (S, s_l->contextIndent? " " : "[",
                        s_l->contextIndent? "╵" : "]",
                        BOARD_TicksNow(),
                        LOG_PREFIX_ASSERT_STR, Func, File, Line);

    outStr (S, LOG_BASE_COLOR);

    if (Msg)
    {
        outStr (S, Msg);
    }
    else
    {
        outStr (S, LANG_ASSERT_NO_DETAILS);
    }

    outStr  (S, ".\r\n");
}


static void logBegin (struct STREAM *const S, const char *const InnerFlow, 
                      const char *const Func, const char *const File,
                      const int Line, 
                      const struct OBJECT_INFO *const DevInfo,
                      const char *const Prefix)
{
    // ╎╎╎│   (in this example: OuterFlow x 3, InnerFlow x 1)
    outContextLevel (S, NULL, InnerFlow, true);

    if (Line == LOG_LINE_NO_TIMING)
    {
        outStr (S, " ");
    }

    // A custom InnerFlow creates a new context header, like this:
    // ╎╎┌┤  5450│ dev:video:sdl: init.
    if (InnerFlow)
    {
        outContextInfo (S, "┤", "│", BOARD_TicksNow(), 
                        Prefix, Func, File, Line);
    }
    // Current context indentation partial timings, like this:
    // ╎╎│     14┊ ⚙ framebuffer width:256₁₀ • height:144₁₀
    else if (s_l->contextIndent)
    {
        const uint32_t I = (s_l->contextIndent - 1) % LOG_CONTEXT_TICKS_DEPTH;
        const TIMER_Ticks Elapsed = BOARD_TicksNow() -
                                    s_l->contextStartTicks[I];
        outContextInfo (S, " ", "┊", Elapsed,
                        Prefix, Func, File, Line);
    }
    // No context indentation,
    // [  5871] dev:board:SDL hosted: ⚠ shutting down.
    else
    {
        outContextInfo (S, "[", "]", BOARD_TicksNow(),
                        Prefix, Func, File, Line);
    }

    if (DevInfo && DevInfo->Ptr)
    {
        BOARD_AssertState (DevInfo->Type && DevInfo->Description);

        outStrArgs (S, 0, LOG_DEVICE_FMT,
                    VARIANT_AutoParams(DevInfo->Type, DevInfo->Description));
    }
}


inline static void logEnd (struct STREAM *const S, const char *const Suffix)
{
    if (Suffix)
    {
        outStr (S, Suffix);
    }
}


void LOG_Args (const char *const InnerFlow, const char *const Func,
               const char *const File, const int Line, 
               const struct OBJECT_INFO *const DevInfo, 
               const char *const Prefix, const char *const Suffix, 
               const char *const Msg, struct VARIANT *const ArgValues, 
               const uint32_t ArgCount)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (Msg);

    struct STREAM *const S = s_l->debugStream;

    logBegin (S, InnerFlow, Func, File, Line, DevInfo, Prefix);

    // A custom InnerFlow keeps the same color as the context level
    if (!InnerFlow)
    {
        outStr (S, LOG_BASE_COLOR);
    }

    outStrArgs (S, 0, Msg, ArgValues, ArgCount);

    logEnd (S, Suffix);

    if (InnerFlow)
    {
        outStr (S, LOG_BASE_COLOR);
    }
}


static void outTableHBorder (struct STREAM *const S,
                             const struct LOG_Table *const Table,
                             const uint8_t Border)
{
    const struct LOG_TableStyle *const Fmt = s_l->logTableStyle;

    outContextLevel (S, NULL, NULL, false);

    uint32_t outColumn = outStrAutoArgs (S, 0, Fmt->HBorders[Border][0]);

    outColumn = outStrAutoArgs (S, outColumn, Fmt->BorderField, 
                                Fmt->HBorders[Border][1],
                                Fmt->HBorders[Border][4],
                                Table->Fields[0].RowEnd); 

    for (uint32_t i = 1; i < Table->FieldCount; ++i)
    {
        outColumn = outStrAutoArgs (S, outColumn, Fmt->BorderField, 
                                    Fmt->HBorders[Border][2],
                                    Fmt->HBorders[Border][4],
                                    Table->Fields[i].RowEnd); 
    }

    outStrAutoArgs (S, outColumn, Fmt->BorderEnd,
                    Fmt->HBorders[Border][3],
                    Fmt->HBorders[Border][4]);
}


static void outTableEntry (struct STREAM *const S,
                           const struct LOG_Table *const Table,
                           struct VARIANT *const ArgValues)
{
    const struct LOG_TableStyle *const Fmt = s_l->logTableStyle;

    outContextLevel (S, NULL, NULL, false);
    uint32_t outColumn = outStrAutoArgs (S, 0, Fmt->EntrySpacing);

    for (uint32_t i = 0; i < Table->FieldCount; ++i)
    {
        const enum VARIANT_Base LastBase = 
            VARIANT_ChangeBaseUint (&ArgValues[i], Table->Fields[i].UintBase);
        
        outColumn = outStrAutoArgs (S, outColumn, Fmt->EntryField,
                                    Fmt->VBorder,
                                    VARIANT_ToString(&ArgValues[i]),
                                    Table->Fields[i].RowEnd);

        VARIANT_ChangeBaseUint (&ArgValues[i], LastBase);
    }

    outStrAutoArgs (S, outColumn, Fmt->EntryEnd,
                    Fmt->VBorder);
}


void LOG_ItemsStyle (const struct LOG_ItemsStyle *const Style)
{
    BOARD_AssertInitialized (s_l);
    s_l->logItemsStyle = ((Style)? Style : &s_DefaultLogItemsStyle);
}


void LOG_ItemsArg (const bool Timestamp, const uint32_t ItemCount,
                   struct VARIANT *const ArgValues,
                   const uint32_t ArgCount)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (ItemCount && ItemCount < LOG_ITEMS_MAX);

    // A single caption or one or more pairs of caption/value
    const uint32_t ItemIndex    = (ArgCount == 1)? 0 : ItemCount;
    const uint32_t MinItemArgs  = ItemCount * 2;

    if (ArgCount == 1)
    {
        BOARD_AssertParams (ItemCount == 1);
    }
    else
    {
        BOARD_AssertParams (ArgCount >= MinItemArgs);
    }

    const int Line = Timestamp? 0 : LOG_LINE_NO_TIMING;

    if (ArgCount > MinItemArgs)
    {
        const uint32_t BaseCount = ArgCount - MinItemArgs;
        enum VARIANT_Base origBase[BaseCount];

        for (uint32_t i = 0; i < BaseCount; ++i)
        {
            origBase[i] = VARIANT_ChangeBaseUint (&ArgValues[(i * 2) + 1],
                                VARIANT_ToUint(&ArgValues[i + MinItemArgs]));
        }

        LOG_Args (NULL, NULL, NULL, Line, NULL,
                  NULL, LOG_SUFFIX_NEWLINE_STR,
                  s_l->logItemsStyle->Items[ItemIndex], 
                  ArgValues, ArgCount);

        for (uint32_t i = 0; i < BaseCount; ++i)
        {
            VARIANT_ChangeBaseUint (&ArgValues[(i * 2) + 1], origBase[i]);
        }
    }
    else
    {
        LOG_Args (NULL, NULL, NULL, Line, NULL,
                  NULL, LOG_SUFFIX_NEWLINE_STR,
                  s_l->logItemsStyle->Items[ItemIndex], 
                  ArgValues, ArgCount);
    }
}


void LOG_TableStyle (const struct LOG_TableStyle *const Style)
{
    BOARD_AssertInitialized (s_l);
    s_l->logTableStyle = ((Style)? Style : &s_DefaultLogTableStyle);
}


void LOG_TableBegin (const struct LOG_Table *const Table)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (Table);

    struct STREAM *const S = s_l->debugStream;
    const struct LOG_TableStyle *const Style = s_l->logTableStyle;

    outContextLevel (S, NULL, NULL, false);
    outStrAutoArgs  (S, 0, Style->TableBegin);
    outContextLevel (S, NULL, NULL, false);
    outStrAutoArgs  (S, 0, Style->TableTitle, Table->Title);
    outTableHBorder (S, Table, 0);

    struct VARIANT argValues[Table->FieldCount];

    for (uint32_t i = 0; i < Table->FieldCount; ++i)
    {
        VARIANT_SetString (&argValues[i], Table->Fields[i].Name);
    }

    outTableEntry   (S, Table, argValues);
    outTableHBorder (S, Table, 1);
}


void LOG_TableEntryArgs (const struct LOG_Table *const Table,
                         struct VARIANT *const ArgValues,
                         const uint32_t ArgCount)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (Table && ArgCount >= Table->FieldCount);

    outTableEntry (s_l->debugStream, Table, ArgValues);
}


void LOG_TableEnd (const struct LOG_Table *const Table)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (Table);

    outTableHBorder (s_l->debugStream, Table, 2);
}


void LOG_ProgressStyle (const struct LOG_ProgressStyle *const Style)
{
    BOARD_AssertInitialized (s_l);
    s_l->logProgressStyle = ((Style)? Style : &s_DefaultLogProgressStyle);
}


uint32_t LOG_ProgressMax (void)
{
    BOARD_AssertInitialized (s_l);
    return s_l->logProgressStyle->Max;
}


uint32_t LOG_ProgressBegin (const enum LOG_ProgressType Type)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (Type < LOG_ProgressType__COUNT);

    const struct LOG_ProgressStyle *const Style = s_l->logProgressStyle;

    outContextLevel (s_l->debugStream, NULL, NULL, true);

    s_l->progressStartColumn = 
                outStrAutoArgs (s_l->debugStream, 0, 
                                Style->Begin, Style->Type[Type]);

    s_l->progressStartColumn += s_l->contextIndent;

    return s_l->progressStartColumn;
}


uint32_t LOG_ProgressUpdate (const uint32_t OutColumn,
                             const uint32_t Progress)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (Progress <= LOG_ProgressMax());

    const struct LOG_ProgressStyle *const Style = s_l->logProgressStyle;

    return outStrAutoArgs (s_l->debugStream, OutColumn, Style->Update,
                           s_l->progressStartColumn + Progress);
}


void LOG_ProgressEnd (void)
{
    BOARD_AssertInitialized (s_l);
    const struct LOG_ProgressStyle *const Style = s_l->logProgressStyle;

    outContextLevel (s_l->debugStream, NULL, NULL, true);

    outStrAutoArgs (s_l->debugStream, 0, Style->End);
    
    s_l->progressStartColumn = 0;
}


void LOG_PendingEndOk (void)
{
    BOARD_AssertInitialized (s_l);
    STREAM_IN_FromString (s_l->debugStream, LOG_PENDING_OK_STR "\r\n");
}


void LOG_PendingEndFail (void)
{
    BOARD_AssertInitialized (s_l);
    STREAM_IN_FromString (s_l->debugStream, LOG_PENDING_FAILED_STR "\r\n");
}


void LOG__contextBegin (void)
{
    BOARD_AssertInitialized (s_l);

    const uint32_t I = s_l->contextIndent % LOG_CONTEXT_TICKS_DEPTH;
    s_l->contextStartTicks[I] = BOARD_TicksNow();
    ++ s_l->contextIndent;
}


void LOG_ContextEnd (void)
{
    BOARD_AssertInitialized (s_l);

    if (s_l->contextIndent)
    {
        const TIMER_Ticks Elapsed =
            BOARD_TicksNow() - s_l->contextStartTicks[s_l->contextIndent - 1];

        outContextLevel (s_l->debugStream, NULL, "└", true);
        outContextInfo  (s_l->debugStream, "x", "╵", Elapsed,
                         NULL, NULL, NULL, LOG_LINE_TIMING_ONLY);
        outStr          (s_l->debugStream, "\r\n");

        -- s_l->contextIndent;
    }
    else
    {
        LOG_Warn (s_l, LANG_INVALID_CONTEXT_END_FMT, s_l->contextIndent);
        BOARD_AssertState (false);
    }
}


void LOG__autoContextEnd  (uint32_t *const Lac)
{
    (void) Lac;
    LOG_ContextEnd ();
}


void LOG__binaryDump (const uint8_t *const Data, const uint32_t Octets)
{
    BOARD_AssertInitialized (s_l);
    BOARD_AssertParams (Data);

    struct STREAM *const S = s_l->debugStream;

    for (uint32_t offs = 0; offs < Octets; offs += 16)
    {
        logBegin    (S, NULL, NULL, NULL, LOG_LINE_NO_TIMING, NULL, NULL);
        outStr      (S, LOG_BASE_COLOR);
        outStrArgs  (S, 0, "`R7`T5`X0:`T10`X1",
                     VARIANT_AutoParams((offs & 0xFFFF0000) >> 16, 
                                         offs & 0x0000FFFF));

        const uint32_t OffsMax = (offs + 16 < Octets)? offs + 16 : Octets;
        uint32_t col = 0;

        for (uint32_t i = offs; i < OffsMax; ++i)
        {
            col += outStrArgs (S, 0, ((i & 0x00000003) == 0)? "  " : " ",
                                                        VARIANT_AutoParams());
            col += outStrArgs (S, 0, "`R7`T2`X0", VARIANT_AutoParams(Data[i]));
        }

        outStrArgs (S, col, "`F54", VARIANT_AutoParams());

        for (uint32_t i = offs; i < OffsMax; ++i)
        {
            const char Ascii = (Data[i] > 31 && Data[i] < 127)? Data[i] : '.';
            STREAM_IN_FromOctet (S, Ascii);
        }

        outStr (S, "\r\n");
    }
}

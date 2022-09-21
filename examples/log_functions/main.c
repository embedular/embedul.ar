#include "embedul.ar/source/core/device/board.h"


const char *const Text = "The quick brown fox jumps over the lazy dog";


int EMBEDULAR_Main (const int Argc, const char *const Argv[])
{
    (void) Argc;
    (void) Argv;

    LOG_Newline ();
    LOG (NOBJ, "LOG()");
    LOG (NOBJ, Text);

    LOG_Newline ();
    LOG (NOBJ, "LOG_Debug()");
    LOG_Debug (NOBJ, Text);

    LOG_Newline ();
    LOG (NOBJ, "LOG_Warn()");
    LOG_Warn (NOBJ, Text);

    LOG_Newline ();
    LOG (NOBJ, "LOG_WarnDebug()");
    LOG_WarnDebug (NOBJ, Text);

    LOG_Newline ();
    LOG (NOBJ, "LOG_Plain()");
    LOG_Plain (Text);

    LOG_Newline ();
    LOG (NOBJ, "LOG_PendingBegin() & LOG_PendingEndOk()");
    LOG_PendingBegin (NOBJ, Text);
    LOG_PendingEndOk ();

    LOG_Newline ();
    LOG (NOBJ, "LOG_PendingBegin() & LOG_PendingEndFail()");
    LOG_PendingBegin (NOBJ, Text);
    LOG_PendingEndFail ();

    LOG_Newline ();
    LOG (NOBJ, "LOG_BinaryDump()");
    LOG_BinaryDump (NOBJ, "Text", (const uint8_t *)Text,
                    (uint32_t)strlen(Text) + 1);

    return 0;
}

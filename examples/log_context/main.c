#include "embedul.ar/source/core/device/board.h"


const char *const Text1 = "The quick brown fox...";
const char *const Text2 = "...jumps over the lazy dog";
const char *const Text3 = "Executing a delay of 200 milliseconds...";


int EMBEDULAR_Main (const int Argc, const char *const Argv[])
{
    (void) Argc;
    (void) Argv;

    // Context opened and closed by calling the corresponding functions.
    LOG_ContextBegin (NOBJ, "Context level 1");
    
        LOG (NOBJ, Text1);

        {
            // Context automatically closed when the call goes out of scope.
            LOG_AutoContext (NOBJ, "Context level 2");

            LOG (NOBJ, Text1);
            LOG (NOBJ, Text2);

            {
                LOG_AutoContext (NOBJ, "Context level 3");

                LOG (NOBJ, Text1);
                LOG (NOBJ, Text2);
                LOG (NOBJ, Text1);

                {
                    LOG_AutoContext (NOBJ, "Context level 4");

                    LOG (NOBJ, Text1);
                    LOG (NOBJ, Text2);
                    LOG (NOBJ, Text1);
                    LOG (NOBJ, Text2);

                    {
                        LOG_AutoContext (NOBJ, "Context level 5");

                        LOG (NOBJ, Text1);
                        LOG (NOBJ, Text2);
                        LOG (NOBJ, Text1);
                        LOG (NOBJ, Text2);
                        LOG (NOBJ, Text1);
                    }
                }
            }

            {
                LOG_AutoContext (NOBJ, "Context level 3.2");

                BOARD_Delay (200);
                LOG (NOBJ, Text3);

                LOG (NOBJ, Text1);
                LOG (NOBJ, Text2);
                LOG (NOBJ, Text1);
            }

            BOARD_Delay (200);
            LOG (NOBJ, Text3);
        }

    LOG_ContextEnd ();

    return 0;
}

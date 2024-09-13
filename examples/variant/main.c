#include "embedul.ar/source/core/main.h"


static void logUint (const char *const Name, struct VARIANT *const V)
{
        struct VARIANT n = VARIANT_SpawnString(Name);

        LOG (NOBJ, "[`0] base 8, no suffix: '`o1'", &n, V);
        LOG (NOBJ, "[`0] base 8, suffix: '`O1'", &n, V);
        LOG (NOBJ, "[`0] base 16, lowercase, no suffix: '`x1'", &n, V);
        LOG (NOBJ, "[`0] base 16, uppercase, no suffix: '`X1'", &n, V);
        LOG (NOBJ, "[`0] base 16, lowercase, suffix: '`h1'", &n, V);
        LOG (NOBJ, "[`0] base 16, uppercase, suffix: '`H1'", &n, V);
}


void EMBEDULAR_Main (void *param)
{
    (void) param;

    {
        // Context automatically closed when the call goes out of scope.
        LOG_AutoContext (NOBJ, "Floating-points");

        struct VARIANT a = VARIANT_SpawnFp (123.1234588623046875);
        struct VARIANT b = VARIANT_SpawnFp (2.125);
        struct VARIANT c = VARIANT_SpawnFp (10.1122112274169921875);

        LOG (NOBJ, "[a]: '`0'", &a);
        LOG (NOBJ, "[b]: '`0'", &b);
        LOG (NOBJ, "[c]: '`0'", &c);
        LOG_Newline ();

        VARIANT_ChangeDigits (&a, 4);
        LOG (NOBJ, "[a] four decimal digits: '`0'", &a);

        VARIANT_ChangeDigits (&a, 15);
        LOG (NOBJ, "[a] fifteen digits: '`0'", &a);

        VARIANT_ChangeDigits (&b, 5);
        LOG (NOBJ, "[b] five digits, filled with zeros: '`0'", &b);

        VARIANT_ChangeDigits (&c, 1);
        LOG (NOBJ, "[c] one digit: '`0'", &c);
        LOG_Newline ();

        LOG (NOBJ, "[a,b,c] own settings: '`0', '`1', '`2'", &a, &b, &c);
        LOG (NOBJ, "[a,b,c] global settings, three digits: `.3'`0', '`1', '`2'", &a, &b, &c);
    }
    {
        LOG_AutoContext (NOBJ, "Integers");

        struct VARIANT a = VARIANT_SpawnInt (10);
        struct VARIANT b = VARIANT_SpawnInt (INT64_MIN);
        struct VARIANT c = VARIANT_SpawnInt (INT64_MAX);

        LOG (NOBJ, "[a]: '`0'", &a);
        LOG (NOBJ, "[b]: '`0'", &b);
        LOG (NOBJ, "[c]: '`0'", &c);
    }
    {
        LOG_AutoContext (NOBJ, "Unsigned integers");

        struct VARIANT a = VARIANT_SpawnUint (0xABCDEF);
        struct VARIANT b = VARIANT_SpawnUint (0777);
        struct VARIANT c = VARIANT_SpawnUint (UINT64_MAX);

        LOG (NOBJ, "[a]: '`0'", &a);
        LOG (NOBJ, "[b]: '`0'", &b);
        LOG (NOBJ, "[c]: '`0'", &c);
        LOG_Newline ();

        logUint ("a", &a);
        LOG_Newline ();

        logUint ("b", &b);
        LOG_Newline ();

        logUint ("c", &c);
    }
}

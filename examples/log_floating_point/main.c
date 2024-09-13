#include "embedul.ar/source/core/main.h"


void EMBEDULAR_Main (void *param)
{
    (void) param;

    struct VARIANT a = VARIANT_SpawnFp (123.1234588623046875);
    struct VARIANT b = VARIANT_SpawnFp (2.125);
    struct VARIANT c = VARIANT_SpawnFp (10.1122112274169921875);

    LOG (NOBJ, "[a]: '`0'", &a);
    LOG (NOBJ, "[b]: '`0'", &b);
    LOG (NOBJ, "[c]: '`0'", &c);

    VARIANT_ChangeDigits (&a, 4);

    LOG (NOBJ, "[a] four decimal digits: '`0'", &a);

    VARIANT_ChangeDigits (&a, 15);

    LOG (NOBJ, "[a] fifteen digits: '`0'", &a);

    VARIANT_ChangeDigits (&b, 5);

    LOG (NOBJ, "[b] five digits, filled with zeros: '`0'", &b);

    VARIANT_ChangeDigits (&c, 1);

    LOG (NOBJ, "[c] one digit: '`0'", &c);

    LOG (NOBJ, "[a,b,c] own settings: '`0', '`1', '`2'", &a, &b, &c);

    LOG (NOBJ, "[a,b,c] global settings, three digits: `.3'`0', '`1', '`2'", &a, &b, &c);

}

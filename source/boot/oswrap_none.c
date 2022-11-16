#include "embedul.ar/source/core/device/oswrap.h"


struct OSWRAP_NONE
{
    struct OSWRAP device;
};


static struct OSWRAP_NONE s_oswrap_none;


static const struct OSWRAP_IFACE OSWRAP_NONE_IFACE =
{
    .Description    = "none"
};


void OSWRAP__boot (void)
{
    OSWRAP_Init ((struct OSWRAP *)&s_oswrap_none, &OSWRAP_NONE_IFACE);
}

#pragma once

#include "embedul.ar/source/core/device/io.h"


struct IO_PROFILE_Map
{
    // corresponding gateway defined in input/output managers
    IO_GatewayId    gatewayId;
    // Inb, Inr, Outb, Outr
    IO_Code         driverCode;
};

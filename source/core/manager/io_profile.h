#pragma once

#include "embedul.ar/source/core/device/io.h"


struct IO_PROFILE_Map
{
    // corresponding gateway defined in input/output managers
    IO_GatewayId    gatewayId;
    // driver Inb, Inr, Outb, or Outr
    IO_Code         driverCode;
    // maximum expected value, zero for none
    IO_Value        maxValue;
};

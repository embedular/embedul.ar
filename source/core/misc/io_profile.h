#pragma once

#include "embedul.ar/source/core/device/io.h"


typedef uint16_t IO_PROFILE_Group;


struct IO_PROFILE_Map
{
    // corresponding gateway defined in input/output managers
    IO_GatewayId    gatewayId;
    // driver Inb, Inr, Outb, or Outr
    IO_Code         driverCode;
    // maximum expected value, zero for none
    IO_Value        maxValue;
};


struct IO_PROFILE
{
    struct IO_PROFILE_Map   * map[IO_Type__COUNT];
    uint16_t                count[IO_Type__COUNT];
};

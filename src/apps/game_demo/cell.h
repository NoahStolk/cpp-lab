#pragma once

#include <cstdint>

#include "direction.h"

struct Cell final
{
    uint8_t health : 3;
    Direction direction : 2;

    void hit();
};

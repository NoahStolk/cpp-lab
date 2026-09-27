#pragma once
#include <array>
#include <random>

#include "cell.h"

class Ui final
{
public:
    static constexpr int size = 16;

    std::array<Cell, size * size> cells;

    std::mt19937 mt;

    Ui()
    {
        for (auto & cell : cells)
        {
            cell = Cell{.health = get_random_health(), .direction = get_random_direction()};
        }
    }

    void print() const;

    void update();

    static int get_index(const int x, const int y)
    {
        return (y * size) + x;
    }

private:
    uint8_t get_random_health()
    {
        const unsigned long r = mt();
        return r; // Health in Cell struct is 3 bits, so values are 0-7.
    }

    Direction get_random_direction()
    {
        const unsigned long r = mt();
        return static_cast<Direction>(r); // Direction in Cell struct is 2 bits.
    }
};

#pragma once

#include <cmath>

struct Vec3
{
    float x;
    float y;
    float z;

    Vec3(const float x, const float y, const float z)
        : x(x), y(y), z(z)
    {
    }

    [[nodiscard]] float len_squared() const
    {
        return (x * x) + (y * y) + (z * z);
    }

    [[nodiscard]] float len() const
    {
        return std::sqrt(len_squared());
    }
};

#pragma once

#include <cmath>
#include <iostream>

struct Vec3
{
    float x;
    float y;
    float z;

    Vec3(const float x, const float y, const float z)
        : x(x), y(y), z(z)
    {
    }

    Vec3 operator+(const Vec3 &vec) const
    {
        return {
            x + vec.x,
            y + vec.y,
            z + vec.z
        };
    }

    Vec3 operator-(const Vec3 &vec) const
    {
        return {
            x - vec.x,
            y - vec.y,
            z - vec.z
        };
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec3& vec)
    {
        return os << vec.x << ", " << vec.y << ", " << vec.z;
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

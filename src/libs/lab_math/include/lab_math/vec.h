#pragma once

#include <cmath>
#include <iostream>

struct Vec3 final
{
    float x;
    float y;
    float z;

    constexpr Vec3(const float x, const float y, const float z) noexcept
        : x(x), y(y), z(z)
    {
    }

    constexpr Vec3& operator+=(const Vec3& vec) noexcept
    {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        return *this;
    }

    [[nodiscard]] friend constexpr Vec3 operator+(Vec3 lhs, const Vec3& rhs) noexcept
    {
        return lhs += rhs;
    }

    constexpr Vec3& operator-=(const Vec3& vec) noexcept
    {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        return *this;
    }

    [[nodiscard]] friend constexpr Vec3 operator-(Vec3 lhs, const Vec3& rhs) noexcept
    {
        return lhs -= rhs;
    }

    constexpr Vec3& operator*=(const float s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    [[nodiscard]] friend constexpr Vec3 operator*(Vec3 vec, const float s) noexcept
    {
        return vec *= s;
    }

    [[nodiscard]] friend constexpr Vec3 operator*(const float s, Vec3 vec) noexcept
    {
        return vec *= s;
    }

    constexpr Vec3& operator/=(const float s) noexcept
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    [[nodiscard]] friend constexpr Vec3 operator/(Vec3 vec, const float s) noexcept
    {
        return vec /= s;
    }

    [[nodiscard]] friend constexpr Vec3 operator-(const Vec3 vec) noexcept
    {
        return {
            -vec.x,
            -vec.y,
            -vec.z
        };
    }

    [[nodiscard]] constexpr bool operator==(const Vec3& vec) const = default;

    friend std::ostream& operator<<(std::ostream& os, const Vec3& vec)
    {
        return os << vec.x << ", " << vec.y << ", " << vec.z;
    }

    [[nodiscard]] constexpr float len_squared() const noexcept
    {
        return (x * x) + (y * y) + (z * z);
    }

    [[nodiscard]] float len() const noexcept
    {
        return std::sqrt(len_squared());
    }
};

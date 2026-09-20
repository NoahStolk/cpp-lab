#pragma once

template <typename T>
concept Dereferenceable = requires(T value)
{
    *value;
};

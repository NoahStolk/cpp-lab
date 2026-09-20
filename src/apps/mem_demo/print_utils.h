#pragma once

#include <iostream>
#include <type_traits>

#include "dereferenceable.h"

static void print(Dereferenceable auto t)
{
    using Pointee = std::remove_pointer_t<decltype(t)>;

    if constexpr (std::is_function_v<Pointee>)
    {
        std::cout << "function:  "
                  << reinterpret_cast<void*>(t) << '\n';
    }
    else
    {
        std::cout << "points to: "
                  << static_cast<void*>(t) << '\n';
        std::cout << "value:     "
                  << *t << '\n';
    }
}

static void header(const char* str)
{
    std::cout << '\n' << "==========" << '\n';
    std::cout << str << '\n';
    std::cout << "==========" << '\n';
}

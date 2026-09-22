#pragma once

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <utility>

class IntOrFloat final
{
public:
    enum class Type : std::uint8_t
    {
        Int,
        Float,
    };

    explicit IntOrFloat(const int int_val)
        : typeTag(Type::Int),
          intVal(int_val)
    {
    }

    explicit IntOrFloat(const float float_val)
        : typeTag(Type::Float),
          floatVal(float_val)
    {
    }

    [[nodiscard]] Type get_type() const
    {
        return typeTag;
    }

    void set_int_val(const int val)
    {
        typeTag = Type::Int;
        intVal = val;
    }

    [[nodiscard]] int get_int_val() const
    {
        if (typeTag != Type::Int)
        {
            throw std::runtime_error{"Cannot call get_int_val when union is of a different type."};
        }
        return intVal;
    }

    void set_float_val(const float val)
    {
        typeTag = Type::Float;
        floatVal = val;
    }

    [[nodiscard]] float get_float_val() const
    {
        if (typeTag != Type::Float)
        {
            throw std::runtime_error{"Cannot call get_float_val when union is of a different type."};
        }
        return floatVal;
    }

    friend std::ostream& operator<<(std::ostream& os, const IntOrFloat& val)
    {
        switch (val.typeTag)
        {
        case Type::Int:
            return os << "int(" << val.intVal << ")";
        case Type::Float:
            return os << "float(" << val.floatVal << ")";
        }

        std::unreachable();
    }

private:
    Type typeTag;

    union
    {
        int intVal;
        float floatVal;
    };
};

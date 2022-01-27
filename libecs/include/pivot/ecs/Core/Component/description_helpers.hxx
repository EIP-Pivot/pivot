#pragma once

#include <pivot/ecs/Core/Data/value.hxx>

namespace pivot::ecs::component::helpers
{
template <typename T>
struct Helpers {
    static data::Type getType();
    static data::Value createValueFromType(const T &v);
    static void updateTypeWithValue(T &data, const data::Value &value);
};
}    // namespace pivot::ecs::component::helpers

#pragma once

#include <pivot/ecs/Core/Data/value.hxx>

namespace pivot::ecs::component::helpers
{
/// Helper struct containing function to interface between C++ types and data model types
template <typename T>
struct Helpers {
    /// Returns the data::Type of a C++ type
    static data::Type getType();
    /// Returns the content of a C++ type as a data::Value
    static data::Value createValueFromType(const T &v);
    /// Updates the content of a C++ type using a data::Value
    static void updateTypeWithValue(T &data, const data::Value &value);
};
}    // namespace pivot::ecs::component::helpers

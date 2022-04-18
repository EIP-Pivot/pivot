#pragma once

#include <string>

namespace pivot::ecs::data
{
struct Asset {
    std::string name;

    auto operator<=>(const Asset &rhs) const = default;
};
}    // namespace pivot::ecs::data

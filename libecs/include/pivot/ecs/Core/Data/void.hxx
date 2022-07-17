#pragma once

#include <variant>

namespace pivot::ecs::data
{
/** \brief A type containing no value
 *
 * Used to represent a component or an event transmitting no value
 */
struct Void : public std::monostate {
    using monostate::monostate;
};
}    // namespace pivot::ecs::data

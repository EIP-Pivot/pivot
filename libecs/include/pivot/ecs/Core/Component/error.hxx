#pragma once

#include <magic_enum.hpp>
#include <stdexcept>
#include <typeinfo>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{

// /// Error thrown when an invalid component is received by a functor in Description
// struct InvalidComponent : public std::logic_error {
//     /// The expected type of the component
//     const std::type_info &expected;
//     /// The type of the component received
//     const std::type_info &received;

//     /// Creates an InvalidComponent error
//     InvalidComponent(const char *expectedName, const std::type_info &expected, const std::type_info &received)
//         : std::logic_error(std::string("Received invalid component: expected ") + expectedName + "(" +
//         expected.name() +
//                            "), but received " + received.name() + "."),
//           expected(expected),
//           received(received){};
// };

/// Error thrown by the component array when a component is set to a value not matching its type
struct InvalidComponentValue : public std::logic_error {
    /// Creates an InvalidComponentValue error
    InvalidComponentValue(const std::string &component_name, const data::Type &expected, const data::Type &received)
        : std::logic_error(std::string("Invalid value for component ") + component_name + ". Got " +
                           received.toString() + " but expected " + expected.toString() + "."){};
};

}    // namespace pivot::ecs::component

#pragma once

#include <magic_enum.hpp>
#include <stdexcept>
#include <typeinfo>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{

/// Error thrown when an invalid component is received by a functor in Description
struct InvalidComponent : public std::logic_error {
    /// The expected type of the component
    const std::type_info &expected;
    /// The type of the component received
    const std::type_info &received;

    /// Creates an InvalidComponent error
    InvalidComponent(const char *expectedName, const std::type_info &expected, const std::type_info &received)
        : std::logic_error(std::string("Received invalid component: expected ") + expectedName + "(" + expected.name() +
                           "), but received " + received.name() + "."),
          expected(expected),
          received(received){};
};

/// Error thrown when an unknown property of a component is accessed
struct UnknownProperty : public std::logic_error {
    /// Creates an UnknownProperty error
    UnknownProperty(const std::string &property): std::logic_error(std::string("Unknown property ") + property){};
};

/// Error thrown when an invalid type is provided for a property of a component
struct PropertyTypeError : public std::logic_error {
    /// Creates an PropertyTypeError error
    PropertyTypeError(const std::string &property, Description::Property::Type expected,
                      Description::Property::Type received)
        : std::logic_error(property + " has type " + std::string(magic_enum::enum_name(expected)) + " but received " +
                           std::string(magic_enum::enum_name(received))){};
};

/// Error thrown when a property is missing while trying to create a component
struct MissingProperty : public std::logic_error {
    /// Creates an MissingProperty error
    MissingProperty(const char *property): std::logic_error(std::string("Missing property ") + property){};
};

}    // namespace pivot::ecs::component

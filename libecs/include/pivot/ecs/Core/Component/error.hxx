#pragma once

#include <magic_enum.hpp>
#include <stdexcept>
#include <typeinfo>

#include <pivot/ecs/Core/Component/description.hxx>

namespace pivot::ecs::component
{
struct InvalidComponent : public std::logic_error {
    const std::type_info &expected;
    const std::type_info &received;

    InvalidComponent(const char *expectedName, const std::type_info &expected, const std::type_info &received)
        : std::logic_error(std::string("Received invalid component: expected ") + expectedName + "(" + expected.name() +
                           "), but received " + received.name() + "."),
          expected(expected),
          received(received){};
};

struct UnknownProperty : public std::logic_error {
    UnknownProperty(const std::string &property): std::logic_error(std::string("Unknown property ") + property){};
};

struct PropertyTypeError : public std::logic_error {
    PropertyTypeError(const std::string &property, Description::Property::Type expected,
                      Description::Property::Type received)
        : std::logic_error(property + " has type " + std::string(magic_enum::enum_name(expected)) + " but received " +
                           std::string(magic_enum::enum_name(received))){};
};

struct MissingProperty : public std::logic_error {
    MissingProperty(const char *property): std::logic_error(std::string("Missing property ") + property){};
};

}    // namespace pivot::ecs::component

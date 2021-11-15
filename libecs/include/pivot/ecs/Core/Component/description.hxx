#pragma once

#include <any>
#include <functional>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

namespace pivot::ecs::component
{

struct Description {
    struct Property {
        std::string name;
        enum class Type {
            STRING,
            NUMBER,
            ASSET,
        } type;

        using ValueType = std::variant<int, std::string>;
    };

    std::string name;
    std::vector<Property> properties;

    using GetPropertyType = Property::ValueType (&)(std::any component, std::string property);
    using SetPropertyType = void (&)(std::any component, std::string property, Property::ValueType value);
    using CreateType = std::any (&)(std::map<std::string, Property::ValueType> properties);

    GetPropertyType getProperty;
    SetPropertyType setProperty;
    CreateType create;

    void validate() const;

        class ValidationError : public std::logic_error  {
                using std::logic_error::logic_error;
        };
};
}    // namespace pivot::ecs::component

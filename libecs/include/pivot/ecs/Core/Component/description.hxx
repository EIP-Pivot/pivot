#pragma once

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace pivot::ecs::component
{

class IComponentArray;

struct Description {
    struct Property {
        std::string name;
        enum class Type {
            STRING,
            NUMBER,
            ASSET,
        } type;

        // TODO: Store the type with the value
        using ValueType = std::variant<int, std::string>;
    };

    std::string name;
    std::vector<Property> properties;

    using GetPropertyType = Property::ValueType(std::any component, std::string property);
    using SetPropertyType = void(std::any component, std::string property, Property::ValueType value);
    using CreateType = std::any(std::map<std::string, Property::ValueType> properties);
    using CreateContainerType = std::unique_ptr<IComponentArray>();

    GetPropertyType &getProperty;
    SetPropertyType &setProperty;
    CreateType &create;
    CreateContainerType &createContainer;

    void validate() const;

    class ValidationError : public std::logic_error
    {
        using std::logic_error::logic_error;
    };
};
}    // namespace pivot::ecs::component

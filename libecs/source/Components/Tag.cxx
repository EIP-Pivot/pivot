#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/error.hxx>

using namespace pivot::ecs::component;

namespace
{

Description::Property::ValueType getProperty(std::any component, std::string property)
{
    if (property != "name") { throw UnknownProperty(property); }
    auto *tag = std::any_cast<Tag>(&component);
    if (tag == nullptr) { throw InvalidComponent("Tag", typeid(tag), component.type()); }
    return Description::Property::ValueType(tag->name);
}

void setProperty(std::any &component, std::string property, Description::Property::ValueType value)
{
    if (property != "name") { throw UnknownProperty(property); }
    auto *tag = std::any_cast<Tag>(&component);
    if (tag == nullptr) { throw InvalidComponent("Tag", typeid(tag), component.type()); }
    auto new_name = std::get_if<std::string>(&value);
    if (new_name == nullptr) {
        throw PropertyTypeError(property, Description::Property::Type::STRING, Description::Property::Type::NUMBER);
    }
    tag->name = *new_name;
}

std::any create(std::map<std::string, Description::Property::ValueType> properties)
{
    auto name_it = properties.find("name");

    if (name_it == properties.end()) { throw MissingProperty("name"); }

    if (properties.size() != 1) {
        for (const auto &property: properties) {
            if (property.first != "name") { throw UnknownProperty(property.first); }
        }
    }

    auto name = std::get_if<std::string>(&name_it->second);
    if (name == nullptr) {
        throw PropertyTypeError("name", Description::Property::Type::STRING, Description::Property::Type::NUMBER);
    }

    return std::make_any<Tag>(*name);
}

std::unique_ptr<IComponentArray> createContainer() { return std::unique_ptr<IComponentArray>(nullptr); }
}    // namespace

Description Tag::description{"Tag",       {{"name", pivot::ecs::component::Description::Property::Type::STRING}},
                             getProperty, setProperty,
                             create,      createContainer};

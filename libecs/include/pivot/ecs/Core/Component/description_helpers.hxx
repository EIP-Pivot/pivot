#pragma once

#include <iostream>
#include <set>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/range_c.hpp>
#include <glm/vec3.hpp>

#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/error.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

namespace pivot::ecs::component::helpers
{

namespace
{
    template <typename T>
    constexpr std::optional<Description::Property::Type> cpp_type_to_property_type = std::nullopt;

    template <>
    constexpr std::optional<Description::Property::Type> cpp_type_to_property_type<std::string> =
        Description::Property::Type::String;
    template <>
    constexpr std::optional<Description::Property::Type> cpp_type_to_property_type<int> =
        Description::Property::Type::Number;
    template <>
    constexpr std::optional<Description::Property::Type> cpp_type_to_property_type<glm::vec3> =
        Description::Property::Type::Vec3;

    Description::Property::Type value_to_type(const Description::Property::ValueType &value)
    {
        return std::visit([](auto v) { return cpp_type_to_property_type<decltype(v)>.value(); }, value);
    }
}    // namespace

template <typename T>
constexpr const char *component_name = nullptr;

template <typename T>
std::vector<Description::Property> buildPropertyArray()
{
    std::vector<Description::Property> properties;
    using Indices = boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value>;
    boost::fusion::for_each(Indices(), [&](auto i) {
        std::string field_name = boost::fusion::extension::struct_member_name<T, decltype(i)::value>::call();
        using value_type = typename boost::fusion::result_of::value_at_c<T, decltype(i)::value>::type;
        auto type = cpp_type_to_property_type<value_type>;
        if (type.has_value()) { properties.push_back({field_name, *type}); }
    });
    return properties;
}

template <typename T>
Description::Property::ValueType getProperty(std::any component, std::string property)
{
    static_assert(component_name<T> != nullptr);

    auto *tag = std::any_cast<T>(&component);
    if (tag == nullptr) { throw InvalidComponent(component_name<T>, typeid(tag), component.type()); }

    std::optional<Description::Property::ValueType> value;

    using Indices = boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value>;
    boost::fusion::for_each(Indices(), [&](auto i) {
        std::string field_name = boost::fusion::extension::struct_member_name<T, decltype(i)::value>::call();
        if (field_name == property) {
            auto it = boost::fusion::advance_c<decltype(i)::value>(boost::fusion::begin(*tag));
            value = std::make_optional(*it);
        }
    });

    if (!value.has_value()) { throw UnknownProperty(property); }
    return *value;
}

template <typename T>
void setProperty(std::any &component, std::string property, Description::Property::ValueType value)
{
    static_assert(component_name<T> != nullptr);

    auto *tag = std::any_cast<T>(&component);
    if (tag == nullptr) { throw InvalidComponent(component_name<T>, typeid(tag), component.type()); }

    bool found = false;

    using Indices = boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value>;
    boost::fusion::for_each(Indices(), [&](auto i) {
        std::string field_name = boost::fusion::extension::struct_member_name<T, decltype(i)::value>::call();
        if (field_name == property) {
            assert(!found);
            auto it = boost::fusion::advance_c<decltype(i)::value>(boost::fusion::begin(*tag));
            using value_type = typename boost::fusion::result_of::value_at_c<T, decltype(i)::value>::type;
            auto new_value = std::get_if<value_type>(&value);

            if (new_value == nullptr) {
                throw PropertyTypeError(property, *cpp_type_to_property_type<value_type>, value_to_type(value));
            }
            *it = *new_value;
            found = true;
        }
    });

    if (!found) { throw UnknownProperty(property); }
}

template <typename T>
std::any create(std::map<std::string, Description::Property::ValueType> properties)
{
    T component;
    auto comp = [](auto a, auto b) { return a.get() < b.get(); };
    std::set<std::reference_wrapper<const std::string>, decltype(comp)> remaining_properties;
    for (auto &pair: properties) { remaining_properties.insert(std::ref(pair.first)); }

    using Indices = boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value>;
    boost::fusion::for_each(Indices(), [&](auto i) {
        const char *field_name = boost::fusion::extension::struct_member_name<T, decltype(i)::value>::call();
        auto property = properties.find(field_name);
        if (property == properties.end()) { throw MissingProperty(field_name); }
        auto it = boost::fusion::advance_c<decltype(i)::value>(boost::fusion::begin(component));
        using value_type = typename boost::fusion::result_of::value_at_c<T, decltype(i)::value>::type;
        auto new_value = std::get_if<value_type>(&property->second);

        if (new_value == nullptr) {
            throw PropertyTypeError(field_name, *cpp_type_to_property_type<value_type>,
                                    value_to_type(property->second));
        }
        *it = *new_value;
        remaining_properties.erase(std::ref(property->first));
    });

    if (!remaining_properties.empty()) { throw UnknownProperty(remaining_properties.begin()->get()); }

    return std::any(component);
}

template <typename A>
std::unique_ptr<IComponentArray> createContainer(Description description)
{
    return std::make_unique<A>(description);
}

template <typename T, typename A>
Description build_component_description(const char *name, bool registerComponent = true)
{
    Description description{name,
                            helpers::buildPropertyArray<T>(),
                            helpers::getProperty<T>,
                            helpers::setProperty<T>,
                            helpers::create<T>,
                            helpers::createContainer<A>};
    if (registerComponent) {
        std::clog << "Registering " << name << std::endl;
        GlobalIndex::getSingleton().registerComponentWithType<T>(description);
    }
    return description;
}
}    // namespace pivot::ecs::component::helpers

/// Registers a component
#define PIVOT_REGISTER_COMPONENT(component_type, array_type)                                                      \
    namespace pivot::ecs::component::helpers                                                                      \
    {                                                                                                             \
        template <>                                                                                               \
        constexpr const char *component_name<component_type> = #component_type;                                   \
        static const auto description = build_component_description<component_type, array_type>(#component_type); \
    }
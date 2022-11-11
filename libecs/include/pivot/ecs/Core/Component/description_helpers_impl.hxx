#pragma once

#include <iostream>
#include <set>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/range_c.hpp>
#include <glm/vec3.hpp>

#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/error.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

namespace pivot::ecs::component::helpers
{

template <typename T>
constexpr const char *component_name = nullptr;

template <typename T>
concept ComponentBaseType = data::basic_type_representation<T> !=
std::nullopt;

template <typename T>
concept ComponentComplexType =
    (data::basic_type_representation<T> == std::nullopt) && boost::fusion::traits::is_sequence<T>::value;

template <typename T>
concept ComponentType = ComponentBaseType<T> || ComponentComplexType<T>;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* code that must be skipped by doxygen */

template <ComponentComplexType T>
struct Helpers<T> {
    static data::Type getType()
    {
        data::RecordType record;
        using Indices = boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value>;
        boost::fusion::for_each(Indices(), [&](auto i) {
            std::string field_name = boost::fusion::extension::struct_member_name<T, decltype(i)::value>::call();
            using value_type = typename boost::fusion::result_of::value_at_c<T, decltype(i)::value>::type;
            data::Type type = Helpers<value_type>::getType();
            record.insert({field_name, type});
        });
        return {record};
    }

    static data::Value createValueFromType(const T &v)
    {
        data::Record record;
        using Indices = boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value>;
        boost::fusion::for_each(Indices(), [&](auto i) {
            std::string field_name = boost::fusion::extension::struct_member_name<T, decltype(i)::value>::call();
            using value_type = typename boost::fusion::result_of::value_at_c<T, decltype(i)::value>::type;
            auto it = boost::fusion::advance_c<decltype(i)::value>(boost::fusion::begin(v));
            record.insert({field_name, Helpers<value_type>::createValueFromType(*it)});
        });
        return {record};
    }

    static void updateTypeWithValue(T &data, const data::Value &value)
    {
        data = T{};
        auto &record = std::get<data::Record>(value);
        using Indices = boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value>;
        boost::fusion::for_each(Indices(), [&](auto i) {
            std::string field_name = boost::fusion::extension::struct_member_name<T, decltype(i)::value>::call();
            using value_type = typename boost::fusion::result_of::value_at_c<T, decltype(i)::value>::type;
            auto it = boost::fusion::advance_c<decltype(i)::value>(boost::fusion::begin(data));
            value_type &member = *it;
            if (record.contains(field_name)) {
                const data::Value &member_value = record.at(field_name);
                Helpers<value_type>::updateTypeWithValue(member, member_value);
            }
        });
    }
};

/// Specialization of the helpers for basic types
template <ComponentBaseType T>
struct Helpers<T> {
    static data::Type getType() { return data::Type{data::basic_type_representation<T>.value()}; }
    static data::Value createValueFromType(const T &v) { return data::Value(v); }
    static void updateTypeWithValue(T &data, const data::Value &value) { data = std::get<T>(value); }
};
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

template <typename A>
std::unique_ptr<IComponentArray> createContainer(Description description)
{
    return std::make_unique<A>(description);
}

template <typename T, typename A>
Description build_component_description(const char *name)
{
    Description description{name, helpers::Helpers<T>::getType(), Provenance::builtin(),
                            helpers::Helpers<T>::createValueFromType(T{}), helpers::createContainer<A>};
    return description;
}
}    // namespace pivot::ecs::component::helpers

/// Registers a component
#define PIVOT_REGISTER_WRAPPED_COMPONENT(wrapper_type, component_type, array_type)  \
    namespace pivot::ecs::component::helpers                                        \
    {                                                                               \
        template <>                                                                 \
        constexpr const char *component_name<wrapper_type> = #wrapper_type;         \
                                                                                    \
        template struct Helpers<component_type>;                                    \
        static auto description_value_##wrapper_type =                              \
            build_component_description<component_type, array_type>(#wrapper_type); \
    }                                                                               \
    const pivot::ecs::component::Description wrapper_type::description =            \
        pivot::ecs::component::helpers::description_value_##wrapper_type;

#define PIVOT_REGISTER_COMPONENT(component_type, array_type) \
    PIVOT_REGISTER_WRAPPED_COMPONENT(component_type, component_type, array_type)

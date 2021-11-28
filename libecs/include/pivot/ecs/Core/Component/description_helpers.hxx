#pragma once

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/range_c.hpp>

#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/error.hxx>

namespace pivot::ecs::component::helpers
{

template <typename T>
std::vector<Description::Property> buildPropertyArray()
{
    std::vector<Description::Property> properties;
    typedef boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<T>::value> Indices;
    boost::fusion::for_each(Indices(), [&](auto i) {
        std::string field_name = boost::fusion::extension::struct_member_name<T, i>::call();
        using value_type = typename boost::fusion::result_of::value_at_c<T, i>::type;
        if constexpr (std::is_same_v<value_type, std::string>) {
            properties.push_back({field_name, Description::Property::Type::STRING});
        }
    });
    return properties;
}
}    // namespace pivot::ecs::component::helpers

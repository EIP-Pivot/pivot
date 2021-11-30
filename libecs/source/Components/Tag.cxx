#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/range_c.hpp>

#include <iostream>

#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/error.hxx>

using namespace pivot::ecs::component;

BOOST_FUSION_ADAPT_STRUCT(Tag, name);

namespace pivot::ecs::component::helpers
{
template <>
constexpr const char *component_name<Tag> = "Tag";
}

Description Tag::description{"Tag",
                             helpers::buildPropertyArray<Tag>(),
                             helpers::getProperty<Tag>,
                             helpers::setProperty<Tag>,
                             helpers::create<Tag>,
                             helpers::createContainer<Tag>};

#include <boost/fusion/include/adapt_struct.hpp>
#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>

BOOST_FUSION_ADAPT_STRUCT(Tag, name);
PIVOT_REGISTER_COMPONENT(Tag);

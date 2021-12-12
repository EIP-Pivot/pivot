#include <boost/fusion/include/adapt_struct.hpp>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>

BOOST_FUSION_ADAPT_STRUCT(Gravity, force);
PIVOT_REGISTER_COMPONENT(Gravity);

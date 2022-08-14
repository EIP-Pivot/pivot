#include <boost/fusion/include/adapt_struct.hpp>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(Gravity, force);
PIVOT_REGISTER_COMPONENT(Gravity, DenseTypedComponentArray<Gravity>);

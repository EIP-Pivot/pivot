#include <boost/fusion/include/adapt_struct.hpp>

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>

#include <pivot/builtins/components/Transform2D.hxx>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(Transform2D, position, rotation, scale);
PIVOT_REGISTER_COMPONENT(Transform2D, DenseTypedComponentArray<Transform2D>);

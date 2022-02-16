#include <boost/fusion/include/adapt_struct.hpp>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>
#include <pivot/graphics/types/Transform.hxx>

BOOST_FUSION_ADAPT_STRUCT(Transform, position, rotation, scale);
PIVOT_REGISTER_COMPONENT(Transform, DenseTypedComponentArray<Transform>);

#include "pivot/graphics/types/RenderObject.hxx"
#include "pivot/ecs/Core/Component/DenseComponentArray.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"
#include "pivot/graphics/types/Transform.hxx"

#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(Transform, position, rotation, scale);
BOOST_FUSION_ADAPT_STRUCT(RenderObject, meshID, transform);
PIVOT_REGISTER_COMPONENT(RenderObject, DenseTypedComponentArray<RenderObject>);

#include "pivot/builtins/components/RenderObject.hxx"
#include "pivot/ecs/Core/Component/DenseComponentArray.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"
#include "pivot/graphics/types/Transform.hxx"

#include <boost/fusion/include/adapt_struct.hpp>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(Transform, position, rotation, scale);

template struct pivot::ecs::component::helpers::Helpers<Transform>;

BOOST_FUSION_ADAPT_STRUCT(RenderObject, meshID, pipelineID, materialIndex, transform);
PIVOT_REGISTER_COMPONENT(RenderObject, DenseTypedComponentArray<RenderObject>);

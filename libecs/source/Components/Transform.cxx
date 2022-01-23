#include <boost/fusion/include/adapt_struct.hpp>
#include <pivot/ecs/Components/Transform.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>

using namespace pivot::ecs::component;

BOOST_FUSION_ADAPT_STRUCT(pivot::ecs::component::Transform, position, rotation, scale);
PIVOT_REGISTER_COMPONENT(Transform, DenseTypedComponentArray<Transform>);

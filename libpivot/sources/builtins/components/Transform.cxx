#include "pivot/builtins/components/Transform.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"
#include "pivot/graphics/types/Transform.hxx"
#include <pivot/ecs/Core/Component/SynchronizedComponentArray.hxx>

#include <boost/fusion/include/adapt_struct.hpp>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::Transform, position, rotation, scale);
PIVOT_REGISTER_WRAPPED_COMPONENT(Transform, pivot::graphics::Transform,
                                 SynchronizedTypedComponentArray<pivot::graphics::Transform>);

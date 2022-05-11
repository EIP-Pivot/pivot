#include "pivot/builtins/components/PointLight.hxx"
#include "pivot/ecs/Core/Component/DenseComponentArray.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"
#include "pivot/graphics/types/Light.hxx"

#include <boost/fusion/include/adapt_struct.hpp>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::PointLight, color, intensity, falloff);
PIVOT_REGISTER_WRAPPED_COMPONENT(PointLight, pivot::graphics::PointLight,
                                 DenseTypedComponentArray<pivot::graphics::PointLight>);

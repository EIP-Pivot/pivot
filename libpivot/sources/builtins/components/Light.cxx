#include "pivot/graphics/types/Light.hxx"
#include "pivot/builtins/components/Light.hxx"
#include "pivot/ecs/Core/Component/DenseComponentArray.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"

#include <boost/fusion/include/adapt_struct.hpp>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::DirectionalLight, color, intensity);
PIVOT_REGISTER_WRAPPED_COMPONENT(DirectionalLight, pivot::graphics::DirectionalLight,
                                 DenseTypedComponentArray<pivot::graphics::DirectionalLight>);

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::PointLight, color, intensity, falloff);
PIVOT_REGISTER_WRAPPED_COMPONENT(PointLight, pivot::graphics::PointLight,
                                 DenseTypedComponentArray<pivot::graphics::PointLight>);

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::SpotLight, color, cutOff, outerCutOff);
PIVOT_REGISTER_WRAPPED_COMPONENT(SpotLight, pivot::graphics::SpotLight,
                                 DenseTypedComponentArray<pivot::graphics::SpotLight>);

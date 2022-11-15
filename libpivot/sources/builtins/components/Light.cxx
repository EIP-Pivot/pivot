#include "pivot/graphics/types/Light.hxx"
#include "pivot/builtins/components/Light.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"
#include <pivot/ecs/Core/Component/SynchronizedComponentArray.hxx>

#include <boost/fusion/include/adapt_struct.hpp>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::DirectionalLight, color, intensity);
PIVOT_REGISTER_WRAPPED_COMPONENT(DirectionalLight, pivot::graphics::DirectionalLight,
                                 SynchronizedTypedComponentArray<pivot::graphics::DirectionalLight>);

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::PointLight, color, intensity, falloff);
PIVOT_REGISTER_WRAPPED_COMPONENT(PointLight, pivot::graphics::PointLight,
                                 SynchronizedTypedComponentArray<pivot::graphics::PointLight>);

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::SpotLight, color, cutOff, outerCutOff, intensity);
PIVOT_REGISTER_WRAPPED_COMPONENT(SpotLight, pivot::graphics::SpotLight,
                                 SynchronizedTypedComponentArray<pivot::graphics::SpotLight>);

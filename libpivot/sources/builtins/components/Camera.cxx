#include <boost/fusion/include/adapt_struct.hpp>

#include <pivot/builtins/components/Camera.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>
#include <pivot/internal/CameraArray.hxx>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(Camera, fov);
PIVOT_REGISTER_COMPONENT(Camera, internals::CameraArray);

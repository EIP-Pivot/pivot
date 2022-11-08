#include <catch2/catch_test_macros.hpp>

#include <pivot/internal/CameraArray.hxx>

using namespace pivot::internals;
using namespace pivot::builtins::components;
using namespace pivot::ecs::component::helpers;

TEST_CASE("Camera array works", "[camera][component]")
{
    CameraArray array(Camera::description);

    REQUIRE(!array.getCurrentCamera().has_value());

    auto camera_value = [](Camera camera) { return Helpers<Camera>::createValueFromType(camera); };

    Camera camera0{0};
    REQUIRE_NOTHROW(array.setValueForEntity(0, camera_value(camera0)));
    array.setCurrentCamera(0);
    auto current_camera = array.getCurrentCamera().value();
    REQUIRE(current_camera.first == 0);
    REQUIRE(current_camera.second.get() == camera0);
}

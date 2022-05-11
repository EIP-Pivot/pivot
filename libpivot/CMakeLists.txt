project(pivot)

add_library(
    ${PROJECT_NAME} STATIC
    sources/engine.cxx
    sources/internal/camera.cxx
    sources/builtins/systems/ControlSystem.cxx
    sources/builtins/systems/PhysicSystem.cxx
    sources/builtins/components/RenderObject.cxx
    sources/builtins/components/Light.cxx
    sources/builtins/components/Transform.cxx
)

target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC pivot-graphics pivot-ecs pivot-script)

build_tests(
    ${PROJECT_NAME} tests/components/test_render_object_component.cxx tests/components/test_transform_component.cxx
)

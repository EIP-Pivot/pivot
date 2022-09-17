project(pivot)

add_library(
    ${PROJECT_NAME} STATIC
    sources/engine.cxx
    sources/internal/camera.cxx
    sources/builtins/systems/ControlSystem.cxx
    sources/builtins/systems/PhysicSystem.cxx
    sources/builtins/systems/CollisionSystem.cxx
    sources/builtins/systems/DrawTextSystem.cxx
    sources/builtins/components/RenderObject.cxx
    sources/builtins/components/Light.cxx
    sources/builtins/components/Transform.cxx
    sources/builtins/components/Collidable.cxx
    sources/builtins/components/Text.cxx
    sources/builtins/components/Transform2D.cxx
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PIVOT_ASSET_SEARCH_DEPTH=4)

target_pivot_compile_option(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC pivot-graphics pivot-ecs pivot-script)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM pivot-common)

build_tests(
    ${PROJECT_NAME} tests/components/test_render_object_component.cxx tests/components/test_transform_component.cxx
    tests/systems/test_collision_system.cxx
)

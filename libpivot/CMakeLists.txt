project(pivot)

add_library(${PROJECT_NAME} STATIC
  sources/game_engine.cxx
  sources/internal/camera.cxx
  sources/systems/ControlSystem.cxx
  )

target_sources(${PROJECT_NAME} INTERFACE
  sources/components/RenderObject.cxx
  sources/components/Transform.cxx
  )

target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC
  pivot-graphics
  pivot-ecs
  )

build_tests(${PROJECT_NAME}
  tests/main.cxx
  tests/components/test_render_object_component.cxx
  )

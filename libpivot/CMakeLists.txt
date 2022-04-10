project(pivot)

add_library(${PROJECT_NAME} STATIC
  sources/internal/camera.cxx
  sources/systems/ControlSystem.cxx
  sources/components/RenderObject.cxx
  )

target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC
  pivot-graphics
  pivot-ecs
  )

build_tests(${PROJECT_NAME}
  tests/components/test_render_object_component.cxx
  )

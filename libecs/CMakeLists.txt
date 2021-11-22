project(pivot-ecs)

include(FetchContent)

add_library(${PROJECT_NAME} source/Systems/ControlSystem.cxx
                            source/Core/ComponentManager.cxx
                            source/Core/Scene.cxx
                            source/Core/EntityManager.cxx
                            source/Core/EventManager.cxx
                            source/Core/SystemManager.cxx
                            source/Core/SceneManager.cxx
                            source/Core/Component/description.cxx
                            source/Core/Component/index.cxx
)
target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC glm magic_enum)

# Unit tests building
if (BUILD_TESTING)
  message(STATUS "Building unit tests for ${PROJECT_NAME}")
  set(TEST_NAME ${PROJECT_NAME}_test)
  add_executable(${TEST_NAME}
    tests/main.cxx
    tests/Core/Component/test_index.cxx
    )
  target_include_directories(${TEST_NAME} PRIVATE tests/)
  target_link_libraries(${TEST_NAME} Catch2 ${PROJECT_NAME})

  add_test(NAME "${PROJECT_NAME} tests"
    COMMAND ${TEST_NAME})
endif()

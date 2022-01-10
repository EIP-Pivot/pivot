project(pivot-ecs)

include(FetchContent)

function(fetch_boost_library library)
FetchContent_Declare(
    boost_${library}
    GIT_REPOSITORY https://github.com/boostorg/${library}.git
    GIT_TAG boost-1.77.0
)
if (NOT boost_${library}_POPULATED)
    message(STATUS "Populating Boost ${library}")
    FetchContent_Populate(boost_${library})
    add_subdirectory(${boost_${library}_SOURCE_DIR} ${boost_${library}_BINARY_DIR})
endif()
endfunction()

fetch_boost_library(fusion)
fetch_boost_library(core)
fetch_boost_library(config)
fetch_boost_library(container_hash)
fetch_boost_library(detail)
fetch_boost_library(integer)
fetch_boost_library(static_assert)
fetch_boost_library(type_traits)
fetch_boost_library(assert)
fetch_boost_library(preprocessor)
fetch_boost_library(throw_exception)
fetch_boost_library(function_types)
fetch_boost_library(mpl)
fetch_boost_library(tuple)
fetch_boost_library(typeof)
fetch_boost_library(utility)
fetch_boost_library(predef)
fetch_boost_library(io)

add_library(${PROJECT_NAME} source/Systems/ControlSystem.cxx
                            source/Core/Scene.cxx
                            source/Core/EntityManager.cxx
                            source/Core/EventManager.cxx
                            source/Core/SystemManager.cxx
                            source/Core/SceneManager.cxx
                            source/Core/Component/description.cxx
                            source/Core/Component/index.cxx
                            source/Core/Systems/description.cxx
                            source/Core/Systems/index.cxx
                            source/Core/Component/manager.cxx
                            source/Components/TagArray.cxx
)
target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC glm magic_enum boost_fusion)
target_sources(${PROJECT_NAME} INTERFACE source/Components/Tag.cxx
                                         source/Components/RigidBody.cxx
                                         source/Components/Gravity.cxx
                                         source/Components/Transform.cxx)

# Unit tests building
if (BUILD_TESTING)
  message(STATUS "Building unit tests for ${PROJECT_NAME}")
  set(TEST_NAME ${PROJECT_NAME}_test)
  add_executable(${TEST_NAME}
    tests/main.cxx
    tests/Core/Component/test_index.cxx
    tests/Core/Component/test_tag.cxx
    tests/Core/Component/test_global_registration.cxx
    tests/Core/test_scene.cxx
    tests/Core/Systems/test_description.cxx
    )
  target_include_directories(${TEST_NAME} PRIVATE tests/)
  target_link_libraries(${TEST_NAME} Catch2 ${PROJECT_NAME})

  add_test(NAME "${PROJECT_NAME} tests"
    COMMAND ${TEST_NAME})
endif()

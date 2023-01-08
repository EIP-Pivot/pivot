project(pivot-ecs)

fetchcontent_declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.10.5
)
if(NOT nlohmann_json_POPULATED)
    message(STATUS "Populating json")
    fetchcontent_populate(nlohmann_json)
    add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR})
endif()

add_library(
    ${PROJECT_NAME}
    source/Core/Scene.cxx
    source/Core/EntityManager.cxx
    source/Core/SceneManager.cxx
    source/Core/Component/description.cxx
    source/Core/Component/index.cxx
    source/Core/Systems/description.cxx
    source/Core/Systems/index.cxx
    source/Core/Systems/manager.cxx
    source/Core/Component/manager.cxx
    source/Core/Component/ref.cxx
    source/Core/Component/combination.cxx
    source/Core/Component/ScriptingComponentArray.cxx
    source/Core/Data/value.cxx
    source/Core/Data/value_serialization.cxx
    source/Core/Data/type.cxx
    source/Core/Event/index.cxx
    source/Core/Event/description.cxx
    source/Core/Event/manager.cxx
    source/Components/Tag.cxx
    source/Components/RigidBody.cxx
    source/Components/Gravity.cxx
    source/Core/Component/FlagComponentStorage.cxx
)

target_pivot_compile_option(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC pivot-common boost_fusion boost_dll nlohmann_json::nlohmann_json)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM pivot-common)

build_tests(
    ${PROJECT_NAME}
    tests/Core/Component/test_index.cxx
    tests/Core/Component/test_tag.cxx
    tests/Core/Component/test_dense_component_array.cxx
    tests/Core/Component/test_ref.cxx
    tests/Core/Component/test_combination.cxx
    tests/Core/Data/test_value_and_type.cxx
    tests/Core/Data/test_serialization.cxx
    tests/Core/test_scene.cxx
    tests/Core/Systems/test_description.cxx
    tests/Core/Event/test_description.cxx
    tests/Core/Event/test_manager.cxx
    tests/Core/Event/test_child_event.cxx
    tests/Core/Scene/test_load.cxx
    tests/Core/Scene/test_save.cxx
    tests/Core/Component/test_flag_component_storage.cxx
)

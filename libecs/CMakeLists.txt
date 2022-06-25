project(pivot-ecs)

include(FetchContent)

function(fetch_boost_library library)
    fetchcontent_declare(
        boost_${library}
        GIT_REPOSITORY https://github.com/boostorg/${library}.git
        GIT_TAG boost-1.79.0
    )
    if(NOT boost_${library}_POPULATED)
        message(STATUS "Populating Boost ${library}")
        fetchcontent_populate(boost_${library})
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
    source/Components/TagArray.cxx
    source/Components/Tag.cxx
    source/Components/RigidBody.cxx
    source/Components/Gravity.cxx
)
target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall
            -Wextra>
            $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /wd4267
            /wd4201
            /wd4244>
)

target_link_libraries(${PROJECT_NAME} PUBLIC pivot-common boost_fusion nlohmann_json::nlohmann_json)

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
    tests/Core/Scene/test_load.cxx
    tests/Core/Scene/test_save.cxx
)

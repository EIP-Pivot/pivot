project(pivot-ecs)

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
fetch_boost_library(dll)

fetch_boost_library(filesystem)
fetch_boost_library(atomic)
fetch_boost_library(align)
fetch_boost_library(iterator)
fetch_boost_library(concept_check)
fetch_boost_library(conversion)
fetch_boost_library(function)
fetch_boost_library(bind)
fetch_boost_library(move)
fetch_boost_library(smart_ptr)
fetch_boost_library(spirit)
fetch_boost_library(array)
fetch_boost_library(endian)
fetch_boost_library(foreach)
fetch_boost_library(optional)
fetch_boost_library(phoenix)
fetch_boost_library(pool)
fetch_boost_library(proto)
fetch_boost_library(range)
fetch_boost_library(regex)
fetch_boost_library(thread)
fetch_boost_library(chrono)
fetch_boost_library(ratio)
fetch_boost_library(rational)
fetch_boost_library(container)
fetch_boost_library(date_time)
fetch_boost_library(numeric_conversion)
fetch_boost_library(tokenizer)
fetch_boost_library(exception)
fetch_boost_library(intrusive)
fetch_boost_library(algorithm)
fetch_boost_library(lexical_cast)
fetch_boost_library(unordered)
fetch_boost_library(variant)
fetch_boost_library(variant2)
fetch_boost_library(system)
fetch_boost_library(type_index)
fetch_boost_library(winapi)
fetch_boost_library(mp11)

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

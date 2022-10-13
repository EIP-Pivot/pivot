project(pivot-common)

fetchcontent_declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG v1.0.2
)
fetchcontent_getproperties(logger)
if(NOT logger_POPULATED)
    message(STATUS "Populating logger")
    fetchcontent_populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR})
    target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION)
endif()

fetchcontent_declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG cc98465e3508535ba8c7f6208df934c156a018dc
)
fetchcontent_getproperties(glm)
if(NOT glm_POPULATED)
    message(STATUS "Populating glm")
    fetchcontent_populate(glm)
    set(GLFW_BUILD_DOCS
        OFF
        CACHE INTERNAL "" FORCE
    )
    set(GLFW_BUILD_TESTS
        OFF
        CACHE INTERNAL "" FORCE
    )
    set(GLFW_BUILD_EXAMPLES
        OFF
        CACHE INTERNAL "" FORCE
    )
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
    target_compile_definitions(
        glm
        INTERFACE GLM_FORCE_INLINE
                  GLM_FORCE_RADIANS
                  GLM_FORCE_INTRINSICS
                  GLM_FORCE_DEPTH_ZERO_TO_ONE
                  GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    )
endif()

fetchcontent_declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG v0.8.1
)
if(NOT magic_enum_POPULATED)
    message(STATUS "Populating Magic Enum")
    fetchcontent_populate(magic_enum)
    add_subdirectory(${magic_enum_SOURCE_DIR} ${magic_enum_BINARY_DIR})
endif()

add_library(
    ${PROJECT_NAME} STATIC
    sources/lib.cxx
    sources/utility/benchmark.cxx

    # Select the correct platform to compile
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
    sources/Platforms/UnixPlatform.cxx>
    $<$<CXX_COMPILER_ID:MSVC>:
    sources/Platforms/WindowsPlatform.cxx>
)

target_pivot_compile_option(${PROJECT_NAME})

if(DEFINED ENV{CI})
    message(STATUS "Benchmark macros - disabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC NO_BENCHMARK)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC logger glm magic_enum)

target_precompile_headers(
    ${PROJECT_NAME}
    PUBLIC
    <pivot/pivot.hxx>
    <cpplogger/Logger.hpp>
    <glm/glm.hpp>
    <optional>
    <variant>
    <vector>
    <unordered_map>
    <string>
)

build_tests(${PROJECT_NAME} tests/indexed_storage.cxx tests/flags.cxx)

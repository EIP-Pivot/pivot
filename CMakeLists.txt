cmake_minimum_required(VERSION 3.20)
project(pivot)

include(FetchContent)
include(CheckIPOSupported)
include(cmake/find_linker.cmake)
include(cmake/CompileShaders.cmake)
include(cmake/tests.cmake)

find_linker()

if(MSVC
   AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129
   AND CMAKE_VERSION VERSION_GREATER 3.20.3
)
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

check_ipo_supported(RESULT supported)
if(supported)
    message(STATUS "LTO enabled")
    cmake_policy(SET CMP0069 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported")
endif()

option(BUILD_TESTING "Build the unit tests" ON)
if(BUILD_TESTING)
    if(DEFINED ENV{CI})
        message(STATUS "CI Unit tests enabled")
    else()
        message(STATUS "Unit tests enabled")
    endif()
    fetchcontent_declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2
        GIT_TAG v3.0.1
    )
    if(NOT catch2_POPULATED)
        message(STATUS "Populating Catch2")
        fetchcontent_populate(catch2)
        add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
    endif()
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
endif()

option(CODE_COVERAGE "Enable code coverage report" OFF)
if(CODE_COVERAGE
   AND BUILD_TESTING
   AND CMAKE_COMPILER_IS_GNUCXX
)
    include(cmake/CodeCoverage.cmake)
    set(COVERAGE_EXCLUDES ${CMAKE_BINARY_DIR}/_deps/* /usr/* **/tests/**)
    append_coverage_compiler_flags()
    setup_target_for_coverage_lcov(
        NAME
        coverage
        EXECUTABLE
        ctest
        DEPENDENCIES
        tests
        LCOV_ARGS
        "--no-external"
    )
    message(STATUS "Code coverage - enabled")
elseif(NOT CODE_COVERAGE)
    message(STATUS "Code coverage - disabled")
elseif(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "Code coverage - unavailable")
endif()

option(BUILD_DOCUMENTATION "Build the HTML documentation" OFF)
if(BUILD_DOCUMENTATION)
    message(STATUS "Documentation building using Doxygen enabled")
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
        set(DOXYGEN_PREDEFINED DOXYGEN_SHOULD_SKIP_THIS)
        doxygen_add_docs(
            doc
            libgraphics/include
            libecs/include
            libscript/include
            README.md
        )
    else()
        message(SEND_ERROR "Doxygen needs to be installed to build the documentation")
    endif()
endif()

option(COMPILER_PROFILING "Enable clang profiling" OFF)
if(COMPILER_PROFILING)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Clang profiling - enabled")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "GNU profiling - enabled")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-report")
    else()
        message(WARNING "Compiler does not support profiling")
    endif()
endif()

fetchcontent_declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG v0.8.0
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
    GIT_TAG v0.8.0
)
if(NOT magic_enum_POPULATED)
    message(STATUS "Populating Magic Enum")
    fetchcontent_populate(magic_enum)
    add_subdirectory(${magic_enum_SOURCE_DIR} ${magic_enum_BINARY_DIR})
endif()

add_subdirectory(libecs/)
add_subdirectory(libgraphics/)
add_subdirectory(libscript/)
add_subdirectory(libpivot/)

option(BUILD_EDITOR "Build pivot editor" ON)
if(BUILD_EDITOR)
    add_subdirectory(editor/)
endif()

option(BUILD_LAUNCHER "Build pivot launcher" ON)
if(BUILD_LAUNCHER)
    add_subdirectory(launcher/)
endif()

add_library(pivot::pivot ALIAS pivot)
add_library(pivot::graphics ALIAS pivot-graphics)
add_library(pivot::ecs ALIAS pivot-ecs)
add_library(pivot::script ALIAS pivot-script)

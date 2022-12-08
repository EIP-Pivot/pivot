cmake_minimum_required(VERSION 3.20)
project(pivot)

include(FetchContent)
include(CheckIPOSupported)
include(GNUInstallDirs)
include(cmake/find_linker.cmake)
include(cmake/CompileShaders.cmake)
include(cmake/PivotCompileOption.cmake)
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
            libcommon/include
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

function(fetch_boost_library library)
    set(BUILD_TESTING OFF)
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

add_subdirectory(libcommon/)
add_library(pivot::commom ALIAS pivot-common)

add_subdirectory(libecs/)
add_library(pivot::ecs ALIAS pivot-ecs)

add_subdirectory(libgraphics/)
add_library(pivot::graphics ALIAS pivot-graphics)

add_subdirectory(libscript/)
add_library(pivot::script ALIAS pivot-script)

add_subdirectory(libpivot/)
add_library(pivot::pivot ALIAS pivot)

option(BUILD_EDITOR "Build pivot editor" ON)
if(BUILD_EDITOR)
    add_subdirectory(editor/)
endif()

option(BUILD_LAUNCHER "Build pivot launcher" ON)
if(BUILD_LAUNCHER)
    add_subdirectory(launcher/)
endif()

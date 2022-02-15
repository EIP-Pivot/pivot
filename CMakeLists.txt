cmake_minimum_required(VERSION 3.20)
project(pivot)

include(FetchContent)
include(CTest)
include(CheckIPOSupported)

check_ipo_supported(RESULT supported)
if(supported)
    message(STATUS "LTO enabled")
    cmake_policy(SET CMP0069 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "LTO not supported")
endif()


if(MSVC)
    # see https://gitlab.kitware.com/cmake/cmake/-/issues/22606
    set(CMAKE_CXX_STANDARD 23)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COMPILER_PROFILING "Enable clang profiling" OFF)
if(COMPILER_PROFILING)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Clang profiling - enabled")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "GNU profiling - enabled")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-report")
    else()
        message(WARNING "Compiler does not support profiling")
    endif()
endif()

FetchContent_Declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG v0.7.1
)
if (NOT logger_POPULATED)
    message(STATUS "Populating logger")
    FetchContent_Populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR})
    target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION)
endif()

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 6ad79aae3eb5bf809c30bf1168171e9e55857e45
)
if (NOT glm_POPULATED)
    message(STATUS "Populating glm")
    FetchContent_Populate(glm)
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
    target_compile_definitions(glm INTERFACE
        GLM_FORCE_INLINE
        GLM_FORCE_RADIANS
        GLM_FORCE_INTRINSICS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    )
endif()

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG c4e3767e265808590986d5db6ca1b5532a7f3d13
)
if (NOT catch2_POPULATED)
    message(STATUS "Populating Catch2")
    FetchContent_Populate(catch2)
    add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
endif()

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG 3d1f6a5a2a3fbcba077e00ad0ccc2dd9fefc2ca7
)
if (NOT magic_enum_POPULATED)
    message(STATUS "Populating Magic Enum")
    FetchContent_Populate(magic_enum)
    add_subdirectory(${magic_enum_SOURCE_DIR} ${magic_enum_BINARY_DIR})
endif()

option(BUILD_DOCUMENTATION "Build the HTML documentation" OFF)
if (BUILD_DOCUMENTATION)
    message(STATUS "Documentation building using Doxygen enabled")
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
        set(DOXYGEN_PREDEFINED DOXYGEN_SHOULD_SKIP_THIS)
        doxygen_add_docs(doc
            libgraphics/include
            libecs/include
            README.md)
    else()
        message(SEND_ERROR "Doxygen needs to be installed to build the documentation")
    endif()
endif()


include(cmake/tests.cmake)

add_subdirectory(libecs/)
add_subdirectory(libgraphics/)
add_subdirectory(libpivot/)


target_compile_options(pivot-ecs PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W3 /wd4267 /wd4201 /wd4244>)

target_compile_options(pivot-graphics PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W3 /wd4267 /wd4201 /wd4244>)

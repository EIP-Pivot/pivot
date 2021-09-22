cmake_minimum_required(VERSION 3.20)
project(pivot)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG v0.6
)
if (NOT logger_POPULATED)
    message(STATUS "Populating logger")
    FetchContent_Populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR})
    target_compile_definitions(logger PUBLIC LOGGER_EXTERN_DECLARATION_PTR)
endif()

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 6ad79aae3eb5bf809c30bf1168171e9e55857e45
)
if (NOT glm_POPULATED)
    message(STATUS "Populating glm")
    FetchContent_Populate(glm)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    target_compile_definitions(glm INTERFACE
        GLM_FORCE_INLINE
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    )
endif()

option(BUILD_DOCUMENTATION "Build the HTML documentation" OFF)
if (BUILD_DOCUMENTATION)
    message(STATUS "Documentation building using Doxygen enabled")
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
        doxygen_add_docs(doc
            libgraphics/include
            libecs/include
            README.md)
    else()
        message(SEND_ERROR "Doxygen needs to be installed to build the documentation")
    endif()
endif()


add_subdirectory(libecs/)
add_subdirectory(libgraphics/)


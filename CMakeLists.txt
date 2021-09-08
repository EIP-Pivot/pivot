cmake_minimum_required(VERSION 3.20)
project(pivot)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG v0.5.1
)
if (NOT logger_POPULATED)
    message(STATUS "Populating logger")
    FetchContent_Populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR})
    target_compile_definitions(logger INTERFACE
        LOGGER_EXTERN_DECLARATION_PTR
    )
endif()

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 06ed280db4e274fa5e1f36d5ea4f7dfd654ff9b0
)
if (NOT glm_POPULATED)
    message(STATUS "Populating glm")
    FetchContent_Populate(glm)
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
    target_compile_definitions(glm INTERFACE
        GLM_FORCE_INLINE
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    )
endif()

add_subdirectory(libecs/)
add_subdirectory(libgraphics/)


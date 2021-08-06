project(pivot-graphics)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

include(shaders/CMakeRule.txt)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 201400b974b63eb7f23eb7d8563589df9c699d7c
)
if (NOT glfw_POPULATED)
    message(STATUS "Populating glfw")
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
    target_compile_definitions(glfw INTERFACE
        -DGLFW_INCLUDE_NONE
        -DGLFW_INCLUDE_VULKAN
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
        -DGLM_FORCE_INLINE
        -DGLM_FORCE_RADIANS
        -DGLM_FORCE_DEPTH_ZERO_TO_ONE
        -DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    )
endif()


FetchContent_Declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG de80acfa90259c1ba4b83fd725d969ccbed3626f
)
if (NOT logger_POPULATED)
    message(STATUS "Populating logger")
    FetchContent_Populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR})
endif()

add_library(${PROJECT_NAME} source/lib.cxx
                            source/Camera.cxx
                            source/Swapchain.cxx
                            source/PipelineBuilder.cxx
                            source/VulkanLoader.cxx
)

target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan
                                              logger
                                              glfw
                                              glm
)

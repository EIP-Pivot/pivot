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
    vma
    GIT_REPOSITORY https://github.com/malte-v/VulkanMemoryAllocator-Hpp.git
    GIT_TAG 3a61240a5354ce56c222969a69825aabb6ba0a21
)
if (NOT vma_POPULATED)
    message(STATUS "Populating vma")
    FetchContent_Populate(vma)
    add_library(VulkanMemoryAllocator STATIC ${vma_SOURCE_DIR}/vk_mem_alloc.h)
    set_source_files_properties(
        ${vma_SOURCE_DIR}/vk_mem_alloc.h PROPERTIES
        LANGUAGE CXX
    )
    target_include_directories(VulkanMemoryAllocator PUBLIC ${vma_SOURCE_DIR})
    target_compile_definitions(VulkanMemoryAllocator PRIVATE VMA_IMPLEMENTATION VMA_USE_STL_CONTAINERS)
    set_target_properties(VulkanMemoryAllocator PROPERTIES LINKER_LANGUAGE CXX)
endif()

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG 3a1174060a7dd4eb652d4e6854bc4cd98c159200
)
if (NOT stb_POPULATED)
    message(STATUS "Populating stb")
    FetchContent_Populate(stb)
    add_library(stb STATIC ${stb_SOURCE_DIR}/stb_image.h)
    set_source_files_properties(
        ${stb_SOURCE_DIR}/stb_image.h PROPERTIES
        LANGUAGE C
    )
    target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION)
    target_include_directories(stb PUBLIC ${stb_SOURCE_DIR})
    set_target_properties(stb PROPERTIES LINKER_LANGUAGE C)
endif()

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG 51908fb967baad5b98973796b32c57989915d910
)
if (NOT tinyobjloader_POPULATED)
    message(STATUS "Populating tinyobjloader")
    FetchContent_Populate(tinyobjloader)
    add_subdirectory(${tinyobjloader_SOURCE_DIR} ${tinyobjloader_BINARY_DIR})
endif()

FetchContent_Declare(
    logger
    GIT_REPOSITORY https://github.com/zcorniere/logger-cpp.git
    GIT_TAG v0.4
)
if (NOT logger_POPULATED)
    message(STATUS "Populating logger")
    FetchContent_Populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR})
    target_compile_definitions(logger INTERFACE
        -DLOGGER_EXTERN_DECLARATION_PTR
    )
endif()

add_library(${PROJECT_NAME} source/vk_utils.cxx
                            source/vk_init.cxx
                            source/Camera.cxx
                            source/Swapchain.cxx
                            source/PipelineBuilder.cxx
                            source/VulkanLoader.cxx
                            source/UniformBufferObject.cxx
                            source/VulkanException.cxx
                            source/VulkanApplication.cxx
                            source/VulkanApplication_init.cxx
                            source/VulkanApplication_static.cxx
                            source/VulkanApplication_helper.cxx
                            source/VulkanApplication_loader.cxx
)
add_shader(${PROJECT_NAME} triangle.vert)
add_shader(${PROJECT_NAME} triangle.frag)

target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan
                                      PUBLIC logger
                                      PUBLIC glfw
                                      PUBLIC glm
                                      PUBLIC VulkanMemoryAllocator
                                      PUBLIC tinyobjloader
                                      PUBLIC stb
)

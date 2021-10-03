project(pivot-graphics)

find_package(Vulkan REQUIRED)
include(shaders/CMakeRule.txt)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 814b7929c5add4b0541ccad26fb81f28b71dc4d8
)
if (NOT glfw_POPULATED)
    message(STATUS "Populating glfw")
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
    target_compile_definitions(glfw INTERFACE
        GLFW_INCLUDE_NONE
        GLFW_INCLUDE_VULKAN
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
    target_link_libraries(VulkanMemoryAllocator PUBLIC Vulkan::Vulkan)
endif()

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a
)
if (NOT stb_POPULATED)
    message(STATUS "Populating stb")
    FetchContent_Populate(stb)
    add_library(stb STATIC ${stb_SOURCE_DIR}/stb_image.h)
    set_source_files_properties(
        ${stb_SOURCE_DIR}/stb_image.h PROPERTIES
        LANGUAGE CXX
    )
    target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION)
    target_include_directories(stb PUBLIC ${stb_SOURCE_DIR})
    set_target_properties(stb PROPERTIES LINKER_LANGUAGE CXX)
endif()

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG 95fe4ef41de28ce6b0aef00bfdd7b788641a15a6
)
if (NOT tinyobjloader_POPULATED)
    message(STATUS "Populating tinyobjloader")
    FetchContent_Populate(tinyobjloader)
    add_subdirectory(${tinyobjloader_SOURCE_DIR} ${tinyobjloader_BINARY_DIR})
    set(TINYOBJLOADER_USE_DOUBLE ON)
endif()

add_library(${PROJECT_NAME} STATIC source/lib.cxx
                                   source/Window.cxx
                                   source/vk_utils.cxx
                                   source/vk_init.cxx
                                   source/Camera.cxx
                                   source/Swapchain.cxx
                                   source/SwapchainSupportDetails.cxx
                                   source/PipelineBuilder.cxx
                                   source/VulkanLoader.cxx
                                   source/UniformBufferObject.cxx
                                   source/VulkanApplication.cxx
                                   source/VulkanApplication_init.cxx
                                   source/VulkanApplication_static.cxx
                                   source/VulkanApplication_helper.cxx
                                   source/VulkanApplication_loader.cxx
                                   source/culling.cxx
)

target_precompile_headers(${PROJECT_NAME} PRIVATE <Logger.hpp> <glm/glm.hpp> <vulkan/vulkan.hpp>)

target_precompile_headers(${PROJECT_NAME} PUBLIC <pivot/graphics/VulkanApplication.hxx>)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  VULKAN_HPP_NO_CONSTRUCTORS
  VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
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

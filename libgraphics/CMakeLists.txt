project(pivot-graphics)

find_package(Vulkan REQUIRED)

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
    ktx
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software.git
    GIT_TAG v4.0.0
)
if (NOT ktx_POPULATED)
    message(STATUS "Populating ktx")
    FetchContent_Populate(ktx)
    set(KTX_FEATURE_STATIC_LIBRARY ON CACHE INTERNAL "" FORCE)
    set(KTX_FEATURE_TESTS OFF CACHE INTERNAL "" FORCE)
    set(KTX_FEATURE_TOOLS OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(${ktx_SOURCE_DIR} ${ktx_SOURCE_DIR})
endif()

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG e7f1ff5c59d3ca2489923beb239bdf93d863498f
)
if (NOT tinygltf_POPULATED)
    message(STATUS "Populating tinygltf")
    FetchContent_Populate(tinygltf)
    set(TINYGLTF_HEADER_ONLY OFF CACHE INTERNAL "" FORCE)
    set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)
    set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(${tinygltf_SOURCE_DIR} ${tinygltf_SOURCE_DIR})
    target_compile_definitions(tinygltf PUBLIC TINYGLTF_NO_STB_IMAGE
                                        TINYGLTF_NO_STB_IMAGE_WRITE
                                        TINYGLTF_NO_INCLUDE_STB_IMAGE
                                        TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
                                        TINYGLTF_NO_EXTERNAL_IMAGE
                                        TINYGLTF_NO_INCLUDE_RAPIDJSON
    )
endif()

FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/EIP-Pivot/VulkanMemoryAllocator-Hpp.git
    GIT_TAG afce2cb1a2e75b35b1c8bef230029cb032b8a2c4
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
    target_compile_definitions(VulkanMemoryAllocator PRIVATE VMA_IMPLEMENTATION)
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
endif()

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/EIP-Pivot/imgui.git
    GIT_TAG 6a52ef987ff9003f763f7afc512bc48a752f3086
)
if (NOT imgui_POPULATED)
    message(STATUS "Populating imgui")
    FetchContent_Populate(imgui)
    add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp

                    ${imgui_SOURCE_DIR}/imgui_draw.cpp
                    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
                    ${imgui_SOURCE_DIR}/imgui_tables.cpp

                    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
                    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
                    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
    )
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/)
    target_link_libraries(imgui PRIVATE Vulkan::Vulkan glfw)
endif()

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG 1.83
)
if (NOT imguizmo_POPULATED)
    message(STATUS "Populating imguizmo")
    FetchContent_Populate(imguizmo)
    add_library(imguizmo STATIC ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
                    ${imguizmo_SOURCE_DIR}/ImCurveEdit.cpp
                    ${imguizmo_SOURCE_DIR}/GraphEditor.cpp
                    ${imguizmo_SOURCE_DIR}/ImGradient.cpp
                    ${imguizmo_SOURCE_DIR}/ImSequencer.cpp
    )
    target_include_directories(imguizmo PUBLIC ${imguizmo_SOURCE_DIR}/)
    target_link_libraries(imguizmo PRIVATE imgui)
endif()

add_library(${PROJECT_NAME} STATIC source/lib.cxx
                                   source/types/Vertex.cxx
                                   source/types/UniformBufferObject.cxx
                                   source/Window.cxx
                                   source/vk_utils.cxx
                                   source/vk_init.cxx
                                   source/vk_debug.cxx
                                   source/VulkanRenderPass.cxx
                                   source/SwapchainSupportDetails.cxx
                                   source/PipelineBuilders/GraphicsPipelineBuilder.cxx
                                   source/PipelineBuilders/ComputePipelineBuilder.cxx
                                   source/VulkanAllocator.cxx
                                   source/VulkanLoader.cxx
                                   source/VulkanSwapchain.cxx
                                   source/AssetsStorage/AssetStorage.cxx
                                   source/AssetsStorage/AssetStorage_init.cxx
                                   source/AssetsStorage/Loaders/AssetStorage_obj.cxx
                                   source/AssetsStorage/Loaders/AssetStorage_png.cxx
                                   source/AssetsStorage/Loaders/AssetStorage_gltf.cxx
                                   source/AssetsStorage/Loaders/AssetStorage_ktx.cxx
                                   source/VulkanApplication/VulkanApplication.cxx
                                   source/VulkanApplication/VulkanApplication_init.cxx
                                   source/VulkanApplication/VulkanApplication_draw.cxx
                                   source/VulkanBase/VulkanBase.cxx
                                   source/VulkanBase/VulkanBase_init.cxx
                                   source/VulkanBase/VulkanBase_static.cxx
                                   source/abstract/AImmediateCommand.cxx
                                   source/culling.cxx
                                   source/DrawCallResolver.cxx
                                   source/PipelineStorage.cxx
)

target_precompile_headers(${PROJECT_NAME} PRIVATE <vulkan/vulkan.hpp> <vk_mem_alloc.hpp>)


target_compile_definitions(${PROJECT_NAME} PUBLIC
  VULKAN_HPP_NO_CONSTRUCTORS
  VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

add_shader(${PROJECT_NAME} default.vert)
add_shader(${PROJECT_NAME} default_pbr.vert)
add_shader(${PROJECT_NAME} default_lit.frag)
add_shader(${PROJECT_NAME} default_unlit.frag)
add_shader(${PROJECT_NAME} default_pbr.frag)

add_shader(${PROJECT_NAME} culling.comp)

target_include_directories(${PROJECT_NAME} PUBLIC include/)


target_link_libraries(${PROJECT_NAME} PUBLIC logger
                                      PUBLIC VulkanMemoryAllocator
                                      PUBLIC imgui
                                      PUBLIC imguizmo
                                      PUBLIC glfw
                                      PUBLIC glm
                                      PUBLIC tinyobjloader
                                      PUBLIC ktx
                                      PUBLIC tinygltf
                                      PUBLIC stb
                                      PUBLIC Vulkan::Vulkan
)

build_tests(${PROJECT_NAME}
  tests/culling.cxx
  tests/bounding_box.cxx
  tests/indexed_storage.cxx
)

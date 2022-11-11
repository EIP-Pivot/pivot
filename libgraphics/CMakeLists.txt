project(pivot-graphics)

find_package(Vulkan REQUIRED)

fetchcontent_declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.7
)
fetchcontent_getproperties(glfw)
if(NOT glfw_POPULATED)
    message(STATUS "Populating glfw")
    fetchcontent_populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
    target_compile_definitions(glfw INTERFACE GLFW_INCLUDE_NONE GLFW_INCLUDE_VULKAN)
endif()

fetchcontent_declare(
    ktx
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software.git
    GIT_TAG v4.0.0
)
fetchcontent_getproperties(ktx)
if(NOT ktx_POPULATED)
    message(STATUS "Populating ktx")
    fetchcontent_populate(ktx)
    set(KTX_FEATURE_STATIC_LIBRARY
        ON
        CACHE INTERNAL "" FORCE
    )
    set(KTX_FEATURE_TESTS
        OFF
        CACHE INTERNAL "" FORCE
    )
    set(KTX_FEATURE_TOOLS
        OFF
        CACHE INTERNAL "" FORCE
    )
    add_subdirectory(${ktx_SOURCE_DIR} ${ktx_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

fetchcontent_declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG 912856823dcb2badf2074055b6d006f7bb3800b8
)
fetchcontent_getproperties(tinygltf)
if(NOT tinygltf_POPULATED)
    message(STATUS "Populating tinygltf")
    fetchcontent_populate(tinygltf)
    set(TINYGLTF_HEADER_ONLY
        OFF
        CACHE INTERNAL "" FORCE
    )
    set(TINYGLTF_BUILD_LOADER_EXAMPLE
        OFF
        CACHE INTERNAL "" FORCE
    )
    set(TINYGLTF_INSTALL
        OFF
        CACHE INTERNAL "" FORCE
    )
    add_subdirectory(${tinygltf_SOURCE_DIR} ${tinygltf_BINARY_DIR} EXCLUDE_FROM_ALL)
    target_compile_definitions(
        tinygltf
        PUBLIC TINYGLTF_NO_STB_IMAGE
               TINYGLTF_NO_STB_IMAGE_WRITE
               TINYGLTF_NO_INCLUDE_STB_IMAGE
               TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
               TINYGLTF_NO_EXTERNAL_IMAGE
               TINYGLTF_NO_INCLUDE_RAPIDJSON
    )
endif()

fetchcontent_declare(
    vma
    GIT_REPOSITORY https://github.com/YaaZ/VulkanMemoryAllocator-Hpp.git
    GIT_TAG da6ea76eecf12a1decc76f58a3e096bcc555bd94
)
fetchcontent_getproperties(vma)
if(NOT vma_POPULATED)
    message(STATUS "Populating vma")
    fetchcontent_populate(vma)
    add_library(VulkanMemoryAllocator STATIC ${vma_SOURCE_DIR}/include/vk_mem_alloc.h)
    set_source_files_properties(${vma_SOURCE_DIR}/include/vk_mem_alloc.h PROPERTIES LANGUAGE CXX)
    target_include_directories(VulkanMemoryAllocator SYSTEM PUBLIC ${vma_SOURCE_DIR}/include/)
    target_compile_definitions(VulkanMemoryAllocator PRIVATE VMA_IMPLEMENTATION)
    set_target_properties(VulkanMemoryAllocator PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(VulkanMemoryAllocator PUBLIC Vulkan::Vulkan)
endif()

fetchcontent_declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a
)
fetchcontent_getproperties(stb)
if(NOT stb_POPULATED)
    message(STATUS "Populating stb")
    fetchcontent_populate(stb)
    add_library(stb STATIC ${stb_SOURCE_DIR}/stb_image.h)
    set_source_files_properties(${stb_SOURCE_DIR}/stb_image.h PROPERTIES LANGUAGE CXX)
    target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION)
    target_include_directories(stb PUBLIC ${stb_SOURCE_DIR})
    set_target_properties(stb PROPERTIES LINKER_LANGUAGE CXX)
endif()

fetchcontent_declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG 8322e00ae685ea623ab6ac5a6cebcfa2d22fbf93
)
fetchcontent_getproperties(tinyobjloader)
if(NOT tinyobjloader_POPULATED)
    message(STATUS "Populating tinyobjloader")
    fetchcontent_populate(tinyobjloader)
    add_subdirectory(${tinyobjloader_SOURCE_DIR} ${tinyobjloader_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

fetchcontent_declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG e9f50fb87c1037e06a9b0e93484fd794a32cb835
)
fetchcontent_getproperties(imgui)
if(NOT imgui_POPULATED)
    message(STATUS "Populating imgui")
    fetchcontent_populate(imgui)
    add_library(
        imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
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

fetchcontent_declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG e3174578bdc99c715e51c5ad88e7d50b4eeb19b0
)
fetchcontent_getproperties(imguizmo)
if(NOT imguizmo_POPULATED)
    message(STATUS "Populating imguizmo")
    fetchcontent_populate(imguizmo)
    add_library(
        imguizmo STATIC
        ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
        ${imguizmo_SOURCE_DIR}/ImCurveEdit.cpp
        ${imguizmo_SOURCE_DIR}/GraphEditor.cpp
        ${imguizmo_SOURCE_DIR}/ImGradient.cpp
        ${imguizmo_SOURCE_DIR}/ImSequencer.cpp
    )
    target_include_directories(imguizmo PUBLIC ${imguizmo_SOURCE_DIR}/)
    target_link_libraries(imguizmo PRIVATE imgui)
endif()

set(PIVOT_ASSET_DEFAULT_DIRECTORY
    "${CMAKE_SOURCE_DIR}/editor/assets/"
    CACHE STRING "Path to the default lookup directory for the asset loading"
)

add_library(
    ${PROJECT_NAME} STATIC
    source/lib.cxx
    source/types/Transform.cxx
    source/types/Vertex.cxx
    source/types/Frame.cxx
    source/types/Light.cxx
    source/types/UniformBufferObject.cxx
    source/types/QueueFamilyIndices.cxx
    source/Window.cxx
    source/vk_utils.cxx
    source/vk_init.cxx
    source/VulkanRenderPass.cxx
    source/SwapchainSupportDetails.cxx
    source/PipelineBuilders/GraphicsPipelineBuilder.cxx
    source/PipelineBuilders/ComputePipelineBuilder.cxx
    source/VulkanAllocator.cxx
    source/VulkanLoader.cxx
    source/VulkanSwapchain.cxx
    source/AssetsStorage/CPUStorage.cxx
    source/AssetsStorage/AssetStorage.cxx
    source/AssetsStorage/AssetStorage_init.cxx
    source/AssetsStorage/AssetStorage_build.cxx
    source/AssetsStorage/Loaders/AssetStorage_obj.cxx
    source/AssetsStorage/Loaders/AssetStorage_png.cxx
    source/AssetsStorage/Loaders/AssetStorage_jpg.cxx
    source/AssetsStorage/Loaders/AssetStorage_gltf.cxx
    source/AssetsStorage/Loaders/AssetStorage_ktx.cxx
    source/VulkanApplication/VulkanApplication.cxx
    source/VulkanApplication/VulkanApplication_init.cxx
    source/VulkanBase/VulkanBase.cxx
    source/VulkanBase/VulkanBase_init.cxx
    source/VulkanBase/VulkanBase_static.cxx
    source/Renderer/CullingRenderer.cxx
    source/Renderer/GraphicsRenderer.cxx
    source/Renderer/ImGuiRenderer.cxx
    source/DescriptorAllocator/DescriptorAllocator.cxx
    source/DescriptorAllocator/DescriptorBuilder.cxx
    source/DescriptorAllocator/DescriptorLayoutCache.cxx
    source/VulkanImmediateCommand.cxx
    source/culling.cxx
    source/Resolver/DrawCallResolver.cxx
    source/Resolver/LightDataResolver.cxx
    source/Resolver/AssetResolver.cxx
    source/ResolverDispatcher.cxx
    source/PipelineStorage.cxx
)

target_pivot_compile_option(${PROJECT_NAME})

target_precompile_headers(${PROJECT_NAME} REUSE_FROM pivot-common)

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC VULKAN_HPP_NO_CONSTRUCTORS
           VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
           VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
           PIVOT_SOURCE_DIRECTORY="${CMAKE_SOURCE_DIR}"
           PIVOT_ASSET_DEFAULT_DIRECTORY="${PIVOT_ASSET_DEFAULT_DIRECTORY}"
           PIVOT_WINDOW_ICON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../assets"
           PIVOT_MAX_FRAMES_IN_FLIGHT=3
)

add_shader(${PROJECT_NAME} default.vert)
add_shader(${PROJECT_NAME} default_pbr.vert)
add_shader(${PROJECT_NAME} default_lit.frag)
add_shader(${PROJECT_NAME} default_unlit.frag)
add_shader(${PROJECT_NAME} default_pbr.frag)
add_shader(${PROJECT_NAME} culling.comp)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC pivot-common
    PUBLIC VulkanMemoryAllocator
    PUBLIC imgui
    PUBLIC imguizmo
    PUBLIC glfw
    PUBLIC tinyobjloader
    PUBLIC ktx
    PUBLIC tinygltf
    PUBLIC stb
    PUBLIC Vulkan::Vulkan
    PUBLIC pivot::ecs
)

build_tests(
    ${PROJECT_NAME}
    tests/culling.cxx
    tests/bounding_box.cxx
    tests/asset_storage_cpu_storage.cxx
    tests/asset_loading_gltf.cxx
    tests/asset_loading_obj.cxx
)

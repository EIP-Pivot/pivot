#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

/// @namespace pivot
/// @brief Contains the types created by the pivot game-engine

/// @namespace pivot::graphics
/// @brief Contains the types created by the graphics library of the pivot game-engine
namespace pivot::graphics
{

/// @namespace pivot::graphics::gpu_object
/// @brief Containing all the classes that will be used in a GLSL shader
///
/// They are contained into a separate namespace because they will be used inside the shaders, so the memory layout is
/// constraint. All types inside of this namespace should have strict alignment declaration as well a static_assert for
/// their size and layout.
namespace gpu_object
{
    /// The push constant data for the vertex shader
    struct VertexPushConstant {
        /// The camera viewproj matrix
        alignas(16) glm::mat4 viewProjection;
    };
    static_assert(sizeof(VertexPushConstant) % 4 == 0);
    static_assert(sizeof(VertexPushConstant) == sizeof(float) * (4 * 4));

    /// The push constant data for the fragment shader
    struct FragmentPushConstant {
        /// The position of the camera
        alignas(16) glm::vec3 position;
    };
    static_assert(sizeof(FragmentPushConstant) % 4 == 0);
    static_assert(sizeof(FragmentPushConstant) == sizeof(float) * 4);

    /// The push constant data for the culling shader
    struct CullingPushConstant {
        /// The camera viewproj matrix
        alignas(16) glm::mat4 viewProjection;
        /// The amount of item
        alignas(4) std::uint32_t drawCount;
    };
    static_assert(sizeof(CullingPushConstant) % 4 == 0);
    static_assert(sizeof(CullingPushConstant) == (sizeof(float) * (4 * 4)) + sizeof(std::uint32_t) * 4);

    /// The size of the required pushConstants
    constexpr const auto pushConstantsSize =
        std::max(sizeof(CullingPushConstant), sizeof(VertexPushConstant) + sizeof(FragmentPushConstant));

}    // namespace gpu_object

/// @struct CameraData
///
/// @brief Hold the camera data
struct CameraData {
    /// Position of the camera.
    glm::vec3 position;
    /// The camera view matrix
    glm::mat4 view;
    /// The camera projection matrix
    glm::mat4 projection;
    /// The camera projected view matrix
    glm::mat4 viewProjection;
};

}    // namespace pivot::graphics

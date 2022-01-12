#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

/// @namespace gpuObject
/// @brief Containing all the classes that will be used in a GLSL shader
///
/// They are contained into a separate namespace because they will be used inside the shaders, so the memory layout is
/// really important. As such, they need to be separated to avoid misuse
namespace gpuObject
{
/// The push constant data for the vertex shader
struct VertexPushConstant {
    /// The camera viewproj matrix
    alignas(16) glm::mat4 viewProjection;
};

/// The push constant data for the fragment shader
struct FragmentPushConstant {
    /// The position of the camera
    alignas(16) glm::vec3 position;
};

/// The push constant data for the culling shader
struct CullingPushConstant {
    /// The camera viewproj matrix
    alignas(16) glm::mat4 viewProjection;
    /// The amount of item
    alignas(16) std::uint32_t drawCount;
};

/// The size of the required pushConstants
constexpr const auto pushConstantsSize =
    std::max(sizeof(CullingPushConstant), sizeof(VertexPushConstant) + sizeof(FragmentPushConstant));

}    // namespace gpuObject

/// @struct CameraData
///
/// @brief Hold the camera data
struct CameraData {
    /// @brief Position of the camera.
    ///
    /// This is a vec4 for easier alignment
    glm::vec3 position;
    /// The camera view matrix
    glm::mat4 view;
    /// The camera projection matrix
    glm::mat4 projection;
    /// The camera projected view matrix
    glm::mat4 viewProjection;
};

constexpr const auto MaxFrameInFlight = 3;

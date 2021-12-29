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
/// @struct CameraData
///
/// @brief Hold the camera data, ready to be send to the GPU
struct CameraData {
    /// @brief Position of the camera.
    ///
    /// This is a vec4 for easier alignment
    glm::vec4 position;
    /// The camera projected view matrix
    glm::mat4 viewproj;
};

/// The size of the required pushConstants
constexpr const auto pushConstantsSize = sizeof(CameraData);

}    // namespace gpuObject
constexpr const auto MaxFrameInFlight = 3;

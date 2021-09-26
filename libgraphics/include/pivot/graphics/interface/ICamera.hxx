#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#ifndef MAX_PROJECTION_LIMIT
#define MAX_PROJECTION_LIMIT 100.0f
#endif

#ifndef MIN_PROJECTION_LIMIT
#define MIN_PROJECTION_LIMIT 0.1f
#endif

/// @interface ICamera
///
/// @brief Interface for the graphical camera
class ICamera
{
public:
    /// @struct GPUCameraData
    ///
    /// @brief Hold the camera data, ready to be send to the GPU
    struct GPUCameraData {
        /// @brief Position of the camera.
        ///
        /// This is a vec4 for easier alignment
        glm::vec4 position;
        /// The camera projected view matrix
        glm::mat4 viewproj;
    };

public:
    /// Return the expected data required by the shaders
    virtual GPUCameraData getGPUCameraData(float fFOV, float fAspectRatio,
                                           float fCloseClippingPlane = MIN_PROJECTION_LIMIT,
                                           float fFarClippingPlane = MAX_PROJECTION_LIMIT) const = 0;
};
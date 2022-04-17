#pragma once

#include <pivot/ecs/Components/Camera.hxx>
#include <pivot/graphics/types/vk_types.hxx>

namespace pivot::internals
{
/// Get CameraData of the camera
graphics::CameraData getGPUCameraData(builtins::Camera camera, float fFOV, float fAspectRatio,
                                      float fCloseClippingPlane = PIVOT_MIN_PROJECTION_LIMIT,
                                      float fFarClippingPlane = PIVOT_MAX_PROJECTION_LIMIT);
}    // namespace pivot::internals

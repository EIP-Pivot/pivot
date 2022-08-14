#include "pivot/internal/camera.hxx"

namespace pivot::internals
{

graphics::CameraData getGPUCameraData(builtins::Camera camera, float fFOV, float fAspectRatio,
                                      float fCloseClippingPlane, float fFarClippingPlane)
{
    auto projection = camera.getProjection(fFOV, fAspectRatio, fCloseClippingPlane, fFarClippingPlane);
    projection[1][1] *= -1;
    auto view = camera.getView();
    return {
        .position = camera.position,
        .view = view,
        .projection = projection,
        .viewProjection = projection * view,
    };
}

}    // namespace pivot::internals

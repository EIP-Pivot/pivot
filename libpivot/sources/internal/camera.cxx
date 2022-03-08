#include "pivot/internal/camera.hxx"

namespace pivot::internal
{

graphics::CameraData getGPUCameraData(Camera camera, float fFOV, float fAspectRatio,
                                      float fCloseClippingPlane = PIVOT_MIN_PROJECTION_LIMIT,
                                      float fFarClippingPlane = PIVOT_MAX_PROJECTION_LIMIT)
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

}    // namespace pivot::internal

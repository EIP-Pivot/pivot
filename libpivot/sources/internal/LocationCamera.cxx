#include <pivot/internal/LocationCamera.hxx>

namespace pivot::internals
{

graphics::CameraData LocationCamera::getGPUCameraData(float fFOV, float fAspectRatio, float fCloseClippingPlane,
                                                      float fFarClippingPlane)
{
    auto projection = this->getProjection(fFOV, fAspectRatio, fCloseClippingPlane, fFarClippingPlane);
    projection[1][1] *= -1;
    auto view = this->getView();
    return {
        .position = this->transform.position,
        .view = view,
        .projection = projection,
        .viewProjection = projection * view,
    };
}

}    // namespace pivot::internals

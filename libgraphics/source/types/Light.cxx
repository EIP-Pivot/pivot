#include "pivot/graphics/types/Light.hxx"

namespace pivot::graphics::gpu_object
{

PointLight::PointLight(const graphics::PointLight &light, const Transform &transform)
    : position(glm::vec4(transform.position, 1.0f)),
      color(glm::vec4(light.color, 1.0f)),
      intensity(light.intensity),
      falloff(light.falloff)
{
}

DirectionalLight::DirectionalLight(const graphics::DirectionalLight &light, const Transform &transform)
    : orientation(glm::vec4(transform.rotation, 1)), color(glm::vec4(light.color, 1)), intensity(light.intensity)

{
}

SpotLight::SpotLight(const graphics::SpotLight &light, const Transform &transform)
    : position(glm::vec4(transform.position, 1.0f)),
      direction(glm::vec4(transform.rotation, 1.0f)),
      color(glm::vec4(light.color, 1.0f)),
      cutOff(light.cutOff),
      outerCutOff(light.outerCutOff),
      intensity(light.intensity)
{
}

}    // namespace pivot::graphics::gpu_object

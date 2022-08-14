#pragma once

#include "pivot/graphics/types/Transform.hxx"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>

namespace pivot::graphics
{

/// The CPU side point light representation
/// @cond
struct PointLight {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    double intensity = 1.0f;
    double falloff = 1.0f;
};
/// @endcond

/// The CPU side Directional light representation
/// @cond
struct DirectionalLight {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    double intensity = 1.0f;
};
/// @endcond

/// The CPU side spot light representation
/// @cond
struct SpotLight {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    double cutOff = glm::cos(glm::radians(12.5f));
    double outerCutOff = glm::cos(glm::radians(17.5f));
    double intensity = 1.0f;
};
/// @endcond

namespace gpu_object
{
    /// Represent a omnidirectional light
    /// @cond
    struct PointLight {
        PointLight(const graphics::PointLight &light, const Transform &transform);
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 color = {1.0f, 1.0f, 1.0f, 0.0f};
        alignas(4) float intensity = 1.0f;
        alignas(4) float falloff = 1.0f;
    };
    static_assert(sizeof(PointLight) % 4 == 0);
    static_assert(sizeof(PointLight) == (sizeof(float) * 4) + (sizeof(float) * 4) + (sizeof(float) * 4));
    /// @endcond

    /// Represent a Directional Light
    /// @cond
    struct DirectionalLight {
        DirectionalLight(const graphics::DirectionalLight &light, const Transform &transform);
        alignas(16) glm::vec4 orientation;
        alignas(16) glm::vec4 color = {1.0f, 1.0f, 1.0f, 0.0f};
        alignas(4) float intensity = 1.0f;
    };
    /// @endcond
    static_assert(sizeof(DirectionalLight) % 4 == 0);
    static_assert(sizeof(DirectionalLight) == (sizeof(float) * 4) + (sizeof(float) * 4) + (sizeof(float)) + 12);

    /// Represent a Spot Light
    /// @cond
    struct SpotLight {
        SpotLight(const graphics::SpotLight &light, const Transform &transform);

        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 direction;
        alignas(16) glm::vec4 color = {1.0f, 1.0f, 1.0f, 0.0f};
        alignas(4) float cutOff = glm::cos(glm::radians(12.5f));
        alignas(4) float outerCutOff = glm::cos(glm::radians(17.5f));
        alignas(4) float intensity = 1.0f;
    };
    /// @endcond
    /// TODO: Check ensure the cutOff > outerCutOff
    static_assert(sizeof(SpotLight) % 4 == 0);
    static_assert(sizeof(SpotLight) == ((sizeof(float) * 4) * 3) + (sizeof(float) * 3) + 4);

}    // namespace gpu_object

}    // namespace pivot::graphics

#pragma once

#include "pivot/graphics/types/vk_types.hxx"

#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

/// @struct Transform
///
/// @brief Hold the model matrix
class Transform
{
public:
    /// Default ctor
    Transform() = default;

    /// Constructor from vector
    inline Transform(const glm::vec3 &translation, const glm::vec3 &rotation, const glm::vec3 &scale)
        : position(translation), rotation(rotation), scale(scale)
    {
    }

    /// Get the model matrix
    inline glm::mat4 getModelMatrix() const noexcept { return recomposeMatrix(*this); }

    /// Compare transforms
    auto operator<=>(const Transform &) const = default;

private:
    inline static glm::mat4 recomposeMatrix(const Transform &tran)
    {
        return glm::translate(glm::mat4(1.0f), tran.position) * glm::toMat4(glm::quat(tran.rotation)) *
               glm::scale(glm::mat4(1.0f), tran.scale);
    }

public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

#pragma once

#include "pivot/graphics/types/vk_types.hxx"

#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace pivot::graphics
{
/// @struct Transform
///
/// @brief Hold the model matrix
class Transform
{
public:
    /// Constructor from vector
    Transform(const glm::vec3 &translation = {}, const glm::vec3 &rotation = {},
              const glm::vec3 &scale = glm::vec3(1.0f))
        : position(translation), rotation(rotation), scale(scale)
    {
    }

    /// Get the model matrix
    glm::mat4 getModelMatrix() const noexcept { return recomposeMatrix(*this); }

    /// Default equality operator
    bool operator==(const Transform &) const = default;

private:
    static glm::mat4 recomposeMatrix(const Transform &tran)
    {
        return glm::translate(glm::mat4(1.0f), tran.position) * glm::toMat4(glm::quat(tran.rotation)) *
               glm::scale(glm::mat4(1.0f), tran.scale);
    }

public:
    /// Translation or position component
    glm::vec3 position;

    /// Rotation component
    glm::vec3 rotation;

    /// Scale component
    glm::vec3 scale;
};
}    // namespace pivot::graphics

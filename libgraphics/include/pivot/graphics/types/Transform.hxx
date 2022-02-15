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
private:
    struct DecomposedMatrix {
        glm::vec3 scale;
        glm::quat orientation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
    };

public:
    /// Default ctor
    Transform() = default;

    /// Constructor from vector
    inline Transform(const glm::vec3 &translation, const glm::vec3 &rotation, const glm::vec3 &scale)
        : Transform(glm::translate(glm::mat4(1.0f), translation), glm::toMat4(glm::quat(rotation)),
                    glm::scale(glm::mat4(1.0f), scale))
    {
    }

    /// Constructor from matrices
    inline Transform(const glm::mat4 &translation, const glm::mat4 &rotation, const glm::mat4 &scale)
        : m_matrix(decomposeMatrix(translation * rotation * scale))
    {
    }

    /// Get the model matrix
    inline glm::mat4 getModelMatrix() const noexcept { return recomposeMatrix(m_matrix); }
    /// Set the model matrix
    inline void setModelMatrix(const glm::mat4 &matrix) { m_matrix = decomposeMatrix(matrix); }

    /// Set the rotation of the model matrix;
    inline void setRotation(const glm::vec3 &rotation) { m_matrix.orientation = rotation; }
    /// Set the position of the model matrix;
    inline void setPosition(const glm::vec3 &position) { m_matrix.translation = position; }
    /// Set the scale of the model matrix;
    inline void setScale(const glm::vec3 &scale) { m_matrix.scale = scale; }
    /// Add position to the model matrix;
    inline void addPosition(const glm::vec3 &position) { m_matrix.translation += position; }

    /// Set the rotation of the model matrix;
    inline glm::vec3 getRotation() const { return glm::eulerAngles(m_matrix.orientation); }
    /// Set the position of the model matrix;
    inline glm::vec3 getPosition() const { return m_matrix.translation; }
    /// Set the scale of the model matrix;
    inline glm::vec3 getScale() const { return m_matrix.scale; }

private:
    inline static DecomposedMatrix decomposeMatrix(const glm::mat4 &modelMatrix)
    {
        DecomposedMatrix ret;

        if (!glm::decompose(modelMatrix, ret.scale, ret.orientation, ret.translation, ret.skew, ret.perspective)) {
            throw std::runtime_error("Error while decomposing matrix");
        }
        return ret;
    }
    inline static glm::mat4 recomposeMatrix(const DecomposedMatrix &decom)
    {
        return glm::translate(glm::mat4(1.0f), decom.translation) * glm::toMat4(decom.orientation) *
               glm::scale(glm::mat4(1.0f), decom.scale);
    }

private:
    DecomposedMatrix m_matrix;
};

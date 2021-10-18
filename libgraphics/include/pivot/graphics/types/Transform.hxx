#pragma once

#include "pivot/graphics/math.hxx"
#include "pivot/graphics/types/vk_types.hxx"

/// @struct Transform
///
/// @brief Hold the model matrix
class Transform
{
private:
    struct DecomposedMatrix {
        glm::vec3 scale;
        glm::vec3 orientation;
        glm::vec3 translation;
    };

public:
    /// Default ctor
    Transform() = default;

    /// Constructor from vector
    Transform(const glm::vec3 &translation, const glm::vec3 &rotation, const glm::vec3 &scale);

    /// Constructor from matrices
    Transform(const glm::mat4 &translation, const glm::mat4 &rotation, const glm::mat4 &scale);

    /// Get a reference of the model matrix
    constexpr glm::mat4 &getModelMatrix() noexcept { return modelMatrix; }
    /// Get a constant reference of the model matrix
    constexpr const glm::mat4 &getModelMatrix() const noexcept { return modelMatrix; }

    /// Set the rotation of the model matrix;
    void setRotation(const glm::vec3 &rotation);
    /// Set the position of the model matrix;
    void setPosition(const glm::vec3 &position);
    /// Set the scale of the model matrix;
    void setScale(const glm::vec3 &scale);
    /// Add position to the model matrix;
    void addPosition(const glm::vec3 &position);
    /// Add scale to the model matrix;

private:
    static DecomposedMatrix decomposeMatrix(const glm::mat4 &modelMatrix);
    static glm::mat4 recomposeMatrix(const DecomposedMatrix &modelMatrix);

private:
    glm::mat4 modelMatrix;
};

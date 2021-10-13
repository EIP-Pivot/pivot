#pragma once

#include "pivot/graphics/types/vk_types.hxx"

#include <glm/mat4x4.hpp>
#include <string>
#include <variant>

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
    /// Constructor from vector
    Transform(const glm::vec3 &translation, const glm::vec3 &rotation, const glm::vec3 &scale);

    /// Constructor from matrices
    Transform(const glm::mat4 &translation, const glm::mat4 &rotation, const glm::mat4 &scale);

    /// Get a reference of the model matrix
    inline glm::mat4 &getModelMatrix() noexcept { return modelMatrix; }
    /// Get a constant reference of the model matrix
    inline const glm::mat4 &getModelMatrix() const noexcept { return modelMatrix; }

    /// Set the rotation of the model matrix;
    void setRotation(const glm::vec3 &rotation);
    /// Set the position of the model matrix;
    void setPosition(const glm::vec3 &position);
    /// Set the scale of the model matrix;
    void setScale(const glm::vec3 &scale);
    /// Add @args position to the model matrix;
    void addPosition(const glm::vec3 &position);

private:
    static DecomposedMatrix decomposeMatrix(const glm::mat4 &modelMatrix);
    static glm::mat4 recomposeMatrix(const DecomposedMatrix &modelMatrix);

private:
    glm::mat4 modelMatrix;
};

/// @class ObjectInformation
///
/// @brief Hold the information of the 3D object, on the CPU-side
struct ObjectInformation {
    /// The object transform
    Transform transform;
    /// The name of the texture to apply on the object
    std::string textureIndex;
    /// The name of the material to use on the object
    std::string materialIndex;
};

namespace gpuObject
{

/// @struct UniformBufferObject
///
/// @brief Hold all the information of the 3D object
struct UniformBufferObject {
    /// @param info The object information
    /// @param imageStor The texture storage, used to resolve the name of the texture
    /// @param materialStor The material storage, used to resolve material name
    UniformBufferObject(const ObjectInformation &info, const ImageStorage &imageStor,
                        const MaterialStorage &materialStor);
    /// The model matrix
    glm::mat4 modelMatrix;
    /// The index of the texture in the buffer
    alignas(16) uint32_t textureIndex = 0;
    /// The index of the material in the buffer
    uint32_t materialIndex = 0;
};
}    // namespace gpuObject
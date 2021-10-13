#include "pivot/graphics/types/UniformBufferObject.hxx"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iterator>

gpuObject::UniformBufferObject::UniformBufferObject(const ObjectInformation &info, const ImageStorage &stor,
                                                    const MaterialStorage &mat)
    : modelMatrix(info.transform.getModelMatrix()),
      textureIndex(std::distance(stor.begin(), stor.find(info.textureIndex))),
      materialIndex(std::distance(mat.begin(), mat.find(info.materialIndex)))
{
}

Transform::Transform(const glm::vec3 &translation, const glm::vec3 &rotation, const glm::vec3 &scale)
    : Transform(glm::translate(glm::mat4(1.0f), translation), glm::toMat4(glm::quat(rotation)),
                glm::scale(glm::mat4(1.0f), scale))
{
}

Transform::Transform(const glm::mat4 &translation, const glm::mat4 &rotation, const glm::mat4 &scale)
    : modelMatrix(translation * rotation * scale)
{
}

void Transform::setRotation(const glm::vec3 &rotation)
{
    auto tmp = decomposeMatrix(modelMatrix);
    tmp.orientation = glm::toMat4(glm::quat(rotation));
    modelMatrix = recomposeMatrix(tmp);
}

void Transform::setPosition(const glm::vec3 &position)
{
    auto tmp = decomposeMatrix(modelMatrix);
    tmp.translation = position;
    modelMatrix = recomposeMatrix(tmp);
}

void Transform::setScale(const glm::vec3 &scale)
{
    auto tmp = decomposeMatrix(modelMatrix);
    tmp.scale = scale;
    modelMatrix = recomposeMatrix(tmp);
}

void Transform::addPosition(const glm::vec3 &position)
{
    modelMatrix = glm::translate(modelMatrix, position);
}

Transform::DecomposedMatrix Transform::decomposeMatrix(const glm::mat4 &modelMatrix)
{
    DecomposedMatrix ret;

    if (!glm::decompose(modelMatrix, ret.scale, ret.orientation, ret.translation, ret.skew, ret.perspective)) {
        throw std::runtime_error("Error while decomposing matrix");
    }
    return ret;
}

glm::mat4 Transform::recomposeMatrix(const Transform::DecomposedMatrix &decom)
{
    return glm::translate(glm::mat4(1.0f), decom.translation) * glm::toMat4(decom.orientation) *
           glm::scale(glm::mat4(1.0f), decom.scale);
}

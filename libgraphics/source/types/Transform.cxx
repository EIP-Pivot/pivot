#include "pivot/graphics/types/Transform.hxx"
#include "pivot/graphics/math.hxx"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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
    tmp.orientation = rotation;
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
    auto tmp = decomposeMatrix(modelMatrix);
    tmp.translation += position;
    modelMatrix = recomposeMatrix(tmp);
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
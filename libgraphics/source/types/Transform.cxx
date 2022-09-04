#include <cpplogger/Logger.hpp>

#include <pivot/graphics/types/Transform.hxx>

namespace pivot::graphics
{
Transform Transform::from_matrix(const glm::mat4 mat)
{

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, rotation, translation, skew, perspective);

    return Transform{.position = translation, .rotation = glm::eulerAngles(rotation), .scale = scale};
}
}    // namespace pivot::graphics

#include <boost/fusion/include/adapt_struct.hpp>
#include <cpplogger/Logger.hpp>

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/SynchronizedComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>
#include <pivot/graphics/types/Transform.hxx>
#include <pivot/graphics/types/TransformArray.hxx>

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

using namespace pivot::graphics;

BOOST_FUSION_ADAPT_STRUCT(Transform, position, rotation, scale, root);
PIVOT_REGISTER_COMPONENT(Transform, SynchronizedTransformArray);

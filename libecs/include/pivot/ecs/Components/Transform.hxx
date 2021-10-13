#pragma once

#include <glm/glm.hpp>

namespace pivot::ecs::component
{
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };
}
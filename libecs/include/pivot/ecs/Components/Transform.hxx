#pragma once

#include <glm/glm.hpp>

namespace pivot::ecs::component
{
    /// @struct Transform
    ///
    /// @brief Transform component (Example component)
    struct Transform
    {
        /// Position vector
        glm::vec3 position;
        /// Rotation vector
        glm::vec3 rotation;
        /// Scale vector
        glm::vec3 scale;
    };
}
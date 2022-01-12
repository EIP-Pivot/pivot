#pragma once

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace pivot::graphics::math
{
inline glm::vec4 normalizePlane(glm::vec4 p) { return p / glm::length(glm::vec3(p)); }
}    // namespace pivot::graphics::math
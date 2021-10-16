#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace pivot::graphics::math
{
bool decomposeMatrix(const glm::mat4 &transform, glm::vec3 &translation, glm::vec3 &rotation, glm::vec3 &scale);
}    // namespace pivot::graphics::math
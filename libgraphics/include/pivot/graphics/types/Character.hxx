#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

namespace pivot::graphics::gpu_object
{

struct Character {
    alignas(4) std::int32_t textureId;
    alignas(8) glm::ivec2 bearing;
    alignas(4) std::uint32_t advance;
};
static_assert(sizeof(Character) % 4 == 0);
static_assert(sizeof(Character) == (sizeof(std::int32_t) + (sizeof(std::int32_t) * 2) + sizeof(std::uint32_t) + 8));

}    // namespace pivot::graphics::gpu_object

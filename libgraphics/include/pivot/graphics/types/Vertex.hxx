#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/PivotFlags.hxx"

namespace pivot::graphics
{

enum class VertexComponentFlagBits : FlagsType {
    Position = BIT(1),
    Normal = BIT(2),
    UV = BIT(3),
    Color = BIT(4),
    Tangent = BIT(5),
};
using VertexComponentFlags = Flags<VertexComponentFlagBits>;

/// @struct Vertex
/// @brief Represent a vertex of the 3D model
struct Vertex {
    /// Position of the vertex
    glm::vec3 pos;
    /// Normal of the vertex
    glm::vec3 normal;
    /// UV coordinate of the vertex
    glm::vec2 texCoord;
    /// Color of the vertex, ignored if a texture is provided
    glm::vec4 color;
    /// Tangent of the vertex
    glm::vec4 tangent;

    /// Equality operator overload
    /// @param other The other object to compare
    /// @return true if both object are equal
    bool operator==(const Vertex &other) const noexcept;

    /// Get the description for Vulkan pipeline input binding
    static vk::VertexInputBindingDescription getBindingDescription() noexcept;
    static vk::VertexInputAttributeDescription inputAttributeDescription(std::uint32_t binding, std::uint32_t location,
                                                                         VertexComponentFlagBits component);
    static std::vector<vk::VertexInputAttributeDescription>
    getInputAttributeDescriptions(std::uint32_t binding, const VertexComponentFlags components);
};
}    // namespace pivot::graphics

ENABLE_FLAGS_FOR_ENUM(pivot::graphics::VertexComponentFlagBits);

namespace std
{
/// @brief Specialization of std::hash for the Vertex structure
template <>
struct hash<pivot::graphics::Vertex> {
    /// @cond
    size_t operator()(pivot::graphics::Vertex const &vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
               (hash<glm::vec3>()(vertex.color) ^ (hash<glm::vec2>()(vertex.texCoord) << 1));
    }
    /// @endcond
};
}    // namespace std

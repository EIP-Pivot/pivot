#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "pivot/utility/flags.hxx"

namespace pivot::graphics
{

/// Select which Vertex component are required for the vulkan pipeline
enum class VertexComponentFlagBits : FlagsType {
    Position = BIT(1),
    Normal = BIT(2),
    UV = BIT(3),
    Color = BIT(4),
    Tangent = BIT(5),
};
/// Flag type of VertexComponentFlagBits
using VertexComponentFlags = Flags<VertexComponentFlagBits>;

/// @struct Vertex
/// @brief Represent a vertex of the 3D model
struct Vertex {
    /// Position of the vertex
    glm::vec3 pos;
    /// Normal of the vertex
    glm::vec3 normal = glm::vec3(0.0f);
    /// UV coordinate of the vertex
    glm::vec2 texCoord = glm::vec2(0.0f);
    /// Color of the vertex, ignored if a texture is provided
    glm::vec3 color = glm::vec3(1.0f);
    /// Tangent of the vertex
    glm::vec4 tangent = glm::vec4(0.0f);

    /// Equality operator overload
    /// @param other The other object to compare
    /// @return true if both object are equal
    bool operator==(const Vertex &other) const noexcept;

    /// Get the description for Vulkan pipeline input binding
    static vk::VertexInputBindingDescription getBindingDescription() noexcept;

    /// Return the input attribute for the given vertex component
    static vk::VertexInputAttributeDescription inputAttributeDescription(std::uint32_t binding, std::uint32_t location,
                                                                         VertexComponentFlagBits component);

    /// Return the vertex input attribute needed for the vulkan pipeline
    static std::vector<vk::VertexInputAttributeDescription>
    getInputAttributeDescriptions(std::uint32_t binding, const VertexComponentFlags components);
};

using Index = std::uint32_t;

}    // namespace pivot::graphics

ENABLE_FLAGS_FOR_ENUM(pivot::graphics::VertexComponentFlagBits);

namespace std
{
/// @brief Specialization of std::hash for the Vertex structure
template <>
struct hash<pivot::graphics::Vertex> {
    /// @cond
    size_t operator()(const pivot::graphics::Vertex &vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
               ((hash<glm::vec2>()(vertex.texCoord) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec4>()(vertex.tangent));
    }
    /// @endcond
};
}    // namespace std

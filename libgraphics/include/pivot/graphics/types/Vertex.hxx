#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

/// @struct Vertex
/// @brief Represent a vertex of the 3D model
struct Vertex {
    /// Position of the vertex
    glm::vec3 pos;
    /// Normal of the vertex
    glm::vec3 normal;
    /// Color of the vertex, ignored if a texture is provided
    glm::vec3 color;
    /// UV coordinate of the vertex
    glm::vec2 texCoord;

    /// Equality operator overload
    /// @param other The other object to compare
    /// @return true if both object are equal
    bool operator==(const Vertex &other) const noexcept;
    /// Inequality operator overload
    /// @param other The other object to compare
    /// @return true if both object are different
    bool operator!=(const Vertex &other) const noexcept;

    /// Get the description for Vulkan pipeline input binding
    static vk::VertexInputBindingDescription getBindingDescription() noexcept;

    /// Get the layout of the Vertex struct for Vulkan pipeline input
    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptons() noexcept;
};

namespace std
{
/// @brief Specialization of std::hash for the Vertex structure
template <>
struct hash<Vertex> {
    /// @cond
    size_t operator()(Vertex const &vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
               (hash<glm::vec3>()(vertex.color) ^ (hash<glm::vec2>()(vertex.texCoord) << 1));
    }
    /// @endcond
};
}    // namespace std

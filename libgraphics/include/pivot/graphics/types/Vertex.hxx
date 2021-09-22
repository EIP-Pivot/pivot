#pragma once

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
    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && color == other.color && normal == other.normal && texCoord == other.texCoord;
    }

    /// Get the description for Vulkan pipeline input binding
    static vk::VertexInputBindingDescription getBindingDescription() noexcept
    {
        vk::VertexInputBindingDescription bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex,
        };

        return bindingDescription;
    }

    /// Get the layout of the Vertex struct for Vulkan pipeline input
    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptons() noexcept
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(4);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[2].offset = offsetof(Vertex, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[3].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
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

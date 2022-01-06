#pragma once

#include <array>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.hpp>

/// @struct MeshBoundingBox
///
/// @brief Represents the cubic bounding box of a mesh
struct MeshBoundingBox {
    MeshBoundingBox() = delete;
    /// New bounding box for a model with only one point
    explicit constexpr MeshBoundingBox(glm::vec3 initialPoint): low(initialPoint), high(initialPoint){};
    /// New bounding box with explicitely set low and high point
    explicit constexpr MeshBoundingBox(glm::vec3 low, glm::vec3 high): low(low), high(high){};

    /// Lowest point of the bouding box
    glm::vec3 low;
    /// Highest point of the bounding box
    glm::vec3 high;

    /// Add a point to the bounding box
    constexpr void addPoint(const glm::vec3 point)
    {
        high.x = std::max(point.x, high.x);
        high.y = std::max(point.y, high.y);
        high.z = std::max(point.z, high.z);
        low.x = std::min(point.x, low.x);
        low.y = std::min(point.y, low.y);
        low.z = std::min(point.z, low.z);
    }

    /// Returns an array of the 8 vertices of the bounding box
    constexpr const std::array<glm::vec3, 8> vertices() const
    {
        return {
            low,
            {high.x, low.y, low.z},
            {low.x, high.y, low.z},
            {low.x, low.y, high.z},
            {high.x, high.y, low.z},
            {high.x, low.y, high.z},
            {low.x, high.y, high.z},
            high,

        };
    }
};

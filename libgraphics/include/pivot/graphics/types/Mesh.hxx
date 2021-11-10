#pragma once

#include "pivot/graphics/types/Vertex.hxx"
#include <vector>

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @struct CPUMesh
///
/// @brief Represent a 3D model loaded in CPU memory
struct CPUMesh {
    /// All the verticies of the models
    std::vector<Vertex> verticies;
    /// The indices of the models
    std::vector<uint32_t> indices;
};    // namespace pivot::graphicsstructCPUMesh

/// @struct GPUMesh
///
/// @brief Represent a 3D model loaded in GPU memory
struct GPUMesh {
    /// Offset of the begining of the vertices
    vk::DeviceSize verticiesOffset = 0;
    /// Number of vertex for the mesh
    vk::DeviceSize verticiesSize = 0;
    /// Offset of the begining of the indices
    vk::DeviceSize indicesOffset = 0;
    /// Number of indice for the mesh
    vk::DeviceSize indicesSize = 0;
};

};    // namespace pivot::graphics
/// @struct MeshBoundingBox
///
/// @brief Represents the cubic bounding box of a mesh
struct MeshBoundingBox {
    MeshBoundingBox() = delete;
    /// New bounding box for a model with only one point
    explicit MeshBoundingBox(glm::vec3 initialPoint): low(initialPoint), high(initialPoint){};
    /// New bounding box with explicitely set low and high point
    MeshBoundingBox(glm::vec3 low, glm::vec3 high): low(low), high(high){};

    /// Lowest point of the bouding box
    glm::vec3 low;
    /// Highest point of the bounding box
    glm::vec3 high;

    /// Add a point to the bounding box
    void addPoint(glm::vec3 point)
    {
        high.x = std::max(point.x, high.x);
        high.y = std::max(point.y, high.y);
        high.z = std::max(point.z, high.z);
        low.x = std::min(point.x, low.x);
        low.y = std::min(point.y, low.y);
        low.z = std::min(point.z, low.z);
    }

    /// Returns an array of the 8 vertices of the bounding box
    std::array<glm::vec3, 8> vertices() const
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

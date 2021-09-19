#pragma once

#include "pivot/graphics/types/Vertex.hxx"
#include <vector>

#include <vulkan/vulkan.hpp>

/// @struct CPUMesh
///
/// @brief Represent a 3D model loaded in CPU memory
struct CPUMesh {
    /// All the verticies of the models
    std::vector<Vertex> verticies;
    /// The indices of the models
    std::vector<uint32_t> indices;
};

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

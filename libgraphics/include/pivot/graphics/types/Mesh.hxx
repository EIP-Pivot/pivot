#pragma once

#include "pivot/graphics/types/Vertex.hxx"
#include <vector>

#include <vulkan/vulkan.hpp>

struct CPUMesh {
    std::vector<Vertex> verticies;
    std::vector<uint32_t> indices;
};

struct GPUMesh {
    vk::DeviceSize verticiesOffset = 0;
    vk::DeviceSize verticiesSize = 0;
    vk::DeviceSize indicesOffset = 0;
    vk::DeviceSize indicesSize = 0;
};

#pragma once

#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/Vertex.hxx"

struct GPUMesh {
    VkDeviceSize verticiesOffset = 0;
    VkDeviceSize verticiesSize = 0;
    VkDeviceSize indicesOffset = 0;
    VkDeviceSize indicesSize = 0;
};

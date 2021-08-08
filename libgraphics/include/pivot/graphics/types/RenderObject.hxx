#pragma once

#include "pivot/graphics/types/UniformBufferObject.hxx"
#include <string>

struct RenderObject {
    std::string meshID;
    std::string objectName = meshID;
    gpuObject::UniformBufferObject ubo;
};

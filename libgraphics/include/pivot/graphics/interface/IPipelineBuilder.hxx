#pragma once

#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

class IPipelineBuilder
{
public:
    virtual IPipelineBuilder &setPipelineLayout(vk::PipelineLayout &) = 0;
    virtual vk::Pipeline build(vk::Device &, vk::PipelineCache = VK_NULL_HANDLE) const = 0;
};

}    // namespace pivot::graphics
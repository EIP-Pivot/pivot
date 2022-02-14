#pragma once

#include <vulkan/vulkan.hpp>

namespace pivot::graphics::interface
{

/// @interface IPipelineBuilder
/// @brief Used to create the PipelineBuilders
class IPipelineBuilder
{
public:
    /// Set the pipeline layout, can be chained
    virtual IPipelineBuilder &setPipelineLayout(vk::PipelineLayout &) = 0;
    /// Build the pipeline
    virtual vk::Pipeline build(vk::Device &, vk::PipelineCache = VK_NULL_HANDLE) const = 0;
};

}    // namespace pivot::graphics::interface
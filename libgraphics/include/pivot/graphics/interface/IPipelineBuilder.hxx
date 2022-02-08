#pragma once

#include <vulkan/vulkan.hpp>

namespace pivot::graphics::interface
{

/// @interface IPipelineBuilder
/// @brief Used to create the PipelineBuilders
class IPipelineBuilder
{
public:
    /// @brief Get the debug name of the builders
    virtual constexpr const char *getDebugPipelineName() const noexcept = 0;
    /// @brief Set the pipeline layout, can be chained
    virtual IPipelineBuilder &setPipelineLayout(vk::PipelineLayout &) = 0;
    /// @brief Build the pipeline
    virtual vk::Pipeline build(vk::Device &, vk::PipelineCache = VK_NULL_HANDLE) const = 0;
};

}    // namespace pivot::graphics::interface
#pragma once

#include <vulkan/vulkan.hpp>

#include "pivot/graphics/ShaderStorage.hxx"

namespace pivot::graphics::internal
{

/// @interface IPipelineBuilder
/// @brief Used to create the PipelineBuilders
class IPipelineBuilder
{
public:
    /// Ctor
    IPipelineBuilder(ShaderStorage &shader): shader(shader) {}
    /// @brief Set the pipeline layout, can be chained
    virtual IPipelineBuilder &setPipelineLayout(vk::PipelineLayout &) = 0;
    /// @brief Build the pipeline
    virtual vk::Pipeline build(vk::Device &, vk::PipelineCache = VK_NULL_HANDLE) const = 0;

protected:
    /// Reference to the ShaderStorage
    ShaderStorage &shader;
};

}    // namespace pivot::graphics::internal

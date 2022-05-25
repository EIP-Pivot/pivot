#pragma once

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DrawCallResolver.hxx"

#include <vulkan/vulkan.hpp>

namespace pivot::graphics::trs
{

struct RenderContext {
    DrawCallResolver &resolver;
    AssetStorage &assetStorage;

    vk::Device device;
    vk::CommandBuffer buffer;
};

}    // namespace pivot::graphics::trs

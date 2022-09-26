#include "pivot/graphics/Resolver/AssetResolver.hxx"

#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics
{

bool AssetResolver::initialize(VulkanBase &base, const AssetStorage &stor, DescriptorBuilder &)
{

    DEBUG_FUNCTION
    base_ref = base;
    storage_ref = stor;
    return true;
}

bool AssetResolver::destroy(VulkanBase &)
{
    DEBUG_FUNCTION
    return true;
}

bool AssetResolver::prepareForDraw(const DrawSceneInformation &)
{
    DEBUG_FUNCTION;
    return true;
}

void AssetResolver::bind(vk::CommandBuffer &cmd)
{
    DEBUG_FUNCTION
    vk::DeviceSize offset = 0;
    cmd.bindVertexBuffers(0, storage_ref->get().getVertexBuffer().buffer, offset);
    cmd.bindIndexBuffer(storage_ref->get().getIndexBuffer().buffer, 0, vk::IndexType::eUint32);
}

}    // namespace pivot::graphics

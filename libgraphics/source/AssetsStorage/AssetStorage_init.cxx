#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/pivot.hxx"
#include "pivot/graphics/types/vk_types.hxx"
#include "pivot/graphics/vk_debug.hxx"

#include <ranges>

namespace pivot::graphics
{

void AssetStorage::createTextureSampler()
{
    DEBUG_FUNCTION
    const auto &properties = base_ref->get().physical_device.getProperties();

    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eNearest,
        .minFilter = vk::Filter::eNearest,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 100,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = VK_FALSE,
    };
    textureSampler = base_ref->get().device.createSampler(samplerInfo);
    vk_debug::setObjectName(base_ref->get().device, textureSampler, "Texture Sampler");
    vulkanDeletionQueue.push([&] { base_ref->get().device.destroySampler(textureSampler); });
}

void AssetStorage::createDescriptorSet(DescriptorBuilder &builder)
{
    DEBUG_FUNCTION
    std::vector<vk::DescriptorImageInfo> imagesInfos;
    imagesInfos.reserve(textureStorage.size());
    std::ranges::transform(textureStorage.getStorage(), std::back_inserter(imagesInfos),
                           [this](const auto &i) { return i.getImageInfo(textureSampler); });
    builder
        .bindBuffer(0, boundingboxBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                    vk::ShaderStageFlagBits::eCompute)
        .bindBuffer(1, materialBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                    vk::ShaderStageFlagBits::eFragment)
        .bindImages(2, imagesInfos, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
        .build(base_ref->get().device, descriptorSet, descriptorSetLayout);
}

}    // namespace pivot::graphics

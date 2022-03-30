#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/types/vk_types.hxx"
#include "pivot/graphics/vk_debug.hxx"

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

void AssetStorage::createDescriptorPool()
{
    DEBUG_FUNCTION
    vk::DescriptorPoolSize poolSize[] = {{
                                             .type = vk::DescriptorType::eStorageBuffer,
                                             .descriptorCount = MaxFrameInFlight * 2,
                                         },
                                         {
                                             .type = vk::DescriptorType::eCombinedImageSampler,
                                             .descriptorCount = static_cast<uint32_t>(textureStorage.size()),
                                         }};

    vk::DescriptorPoolCreateInfo poolInfo{
        .maxSets = MaxFrameInFlight,
        .poolSizeCount = std::size(poolSize),
        .pPoolSizes = poolSize,
    };
    descriptorPool = base_ref->get().device.createDescriptorPool(poolInfo);
    vk_debug::setObjectName(base_ref->get().device, descriptorPool, "Asset DescriptorPool");
    vulkanDeletionQueue.push([&] { base_ref->get().device.destroyDescriptorPool(descriptorPool); });
}

void AssetStorage::createDescriptorSetLayout()
{
    DEBUG_FUNCTION
    std::vector<vk::DescriptorBindingFlags> flags{
        {},
        {},
        vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound,
    };

    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingInfo{
        .bindingCount = static_cast<uint32_t>(flags.size()),
        .pBindingFlags = flags.data(),
    };
    vk::DescriptorSetLayoutBinding boundingBoxBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eCompute,
    };
    vk::DescriptorSetLayoutBinding materialLayoutBinding{
        .binding = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
    };
    vk::DescriptorSetLayoutBinding samplerLayoutBiding{
        .binding = 2,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = static_cast<uint32_t>(textureStorage.size()),
        .stageFlags = vk::ShaderStageFlagBits::eFragment,
    };
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings = {
        boundingBoxBinding,
        materialLayoutBinding,
        samplerLayoutBiding,
    };
    vk::DescriptorSetLayoutCreateInfo ressourcesSetLayoutInfo{
        .pNext = &bindingInfo,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    descriptorSetLayout = base_ref->get().device.createDescriptorSetLayout(ressourcesSetLayoutInfo);
    vk_debug::setObjectName(base_ref->get().device, descriptorSetLayout, "AssetStorage Descriptor Set Layout");
    vulkanDeletionQueue.push([&] { base_ref->get().device.destroyDescriptorSetLayout(descriptorSetLayout); });
}

void AssetStorage::createDescriptorSet()
{
    DEBUG_FUNCTION
    const std::uint32_t counts = textureStorage.size();
    vk::DescriptorSetVariableDescriptorCountAllocateInfo set_counts{
        .descriptorSetCount = 1,
        .pDescriptorCounts = &counts,
    };
    vk::DescriptorSetAllocateInfo allocInfo{
        .pNext = &set_counts,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };
    descriptorSet = base_ref->get().device.allocateDescriptorSets(allocInfo).front();

    std::vector<vk::DescriptorImageInfo> imagesInfos;
    for (auto &t: textureStorage.getStorage()) {
        imagesInfos.push_back({
            .sampler = textureSampler,
            .imageView = t.imageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        });
    }
    vk::DescriptorBufferInfo materialInfo{
        .buffer = materialBuffer.buffer,
        .offset = 0,
        .range = materialBuffer.getSize(),
    };
    vk::DescriptorBufferInfo boundingBoxInfo{
        .buffer = boundingboxBuffer.buffer,
        .offset = 0,
        .range = boundingboxBuffer.getSize(),
    };
    std::vector<vk::WriteDescriptorSet> descriptorWrite{
        {
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &boundingBoxInfo,
        },
        {
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &materialInfo,
        },
        {
            .dstSet = descriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(imagesInfos.size()),
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = imagesInfos.data(),
        },
    };
    base_ref->get().device.updateDescriptorSets(descriptorWrite, 0);
}

}    // namespace pivot::graphics

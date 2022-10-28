#include "pivot/graphics/Resolver/LightDataResolver.hxx"

#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics
{

vk::DescriptorSetLayout LightDataResolver::descriptorSetLayout = VK_NULL_HANDLE;

bool LightDataResolver::initialize(VulkanBase &base, const AssetStorage &stor, DescriptorBuilder &builder)
{

    DEBUG_FUNCTION();
    base_ref = base;
    storage_ref = stor;
    createLightBuffer();

    auto success = builder
                       .bindBuffer(0, frame.directLightBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eFragment)
                       .bindBuffer(1, frame.spotLightBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eFragment)
                       .bindBuffer(2, frame.omniLightBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eFragment)
                       .build(base_ref->get().device, descriptorSet, descriptorSetLayout);

    if (!verifyAlwaysMsg(success, "Descriptor set failed to build.")) { return success; }

    vk_debug::setObjectName(base_ref->get().device, descriptorSet,
                            "Light Descriptor Set " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    vk_debug::setObjectName(base_ref->get().device, descriptorSetLayout,
                            "Light Descriptor Set Layout " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    updateDescriptorSet();
    return true;
}

bool LightDataResolver::destroy(VulkanBase &)
{
    DEBUG_FUNCTION();
    base_ref->get().allocator.destroyBuffer(frame.omniLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.directLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.spotLightBuffer);
    return true;
}

template <class T, class G>
requires std::is_constructible_v<G, const T &, const Transform &> && BufferValid<G>
static vk::DeviceSize handleLights(VulkanBase &base_ref, AllocatedBuffer<G> &buffer, const Object<T> &lights,
                                   const Object<Transform> &transforms)
{
    PROFILE_FUNCTION();
    verifyMsg(lights.objects.get().size() == lights.exist.get().size(), "Light ECS data are incorrect");
    verifyMsg(transforms.objects.get().size() == transforms.exist.get().size(), "Transform ECS data are incorrect");

    std::vector<G> lightsData;
    for (unsigned i = 0; i < lights.objects.get().size() && i < transforms.objects.get().size(); i++) {
        if (!lights.exist.get().at(i) || !lights.exist.get().at(i)) continue;
        const auto &light = lights.objects.get().at(i);
        const auto &transform = transforms.objects.get().at(i);

        lightsData.emplace_back(light, transform);
    }
    if (buffer.getSize() < lightsData.size()) {
        if (buffer) base_ref.allocator.destroyBuffer(buffer);
        buffer = base_ref.allocator.createMappedBuffer<G>(lightsData.size(), buffer.name);
    }
    base_ref.allocator.copyBuffer(buffer, std::span(lightsData));
    return lightsData.size();
}

bool LightDataResolver::prepareForDraw(const DrawSceneInformation &sceneInformation)
{
    PROFILE_FUNCTION();

    std::int32_t iCount = 0;
    verifyMsg(sceneInformation.pointLight.objects.get().size() == sceneInformation.pointLight.exist.get().size(),
              "ECS Point tights arrays are invalid.");
    verifyMsg(sceneInformation.directionalLight.objects.get().size() ==
                  sceneInformation.directionalLight.exist.get().size(),
              "ECS Directional lights arrays are invalid.");
    verifyMsg(sceneInformation.spotLight.objects.get().size() == sceneInformation.spotLight.exist.get().size(),
              "ECS Spot light arrays are invalid.");

    iCount += frame.pointLightCount;
    frame.pointLightCount =
        handleLights(base_ref.value(), frame.omniLightBuffer, sceneInformation.pointLight, sceneInformation.transform);
    iCount -= frame.pointLightCount;

    iCount += frame.directionalLightCount;
    frame.directionalLightCount = handleLights(base_ref.value(), frame.directLightBuffer,
                                               sceneInformation.directionalLight, sceneInformation.transform);
    iCount += frame.directionalLightCount;

    iCount += frame.spotLightCount;
    frame.spotLightCount =
        handleLights(base_ref.value(), frame.spotLightBuffer, sceneInformation.spotLight, sceneInformation.transform);
    iCount += frame.spotLightCount;

    if (iCount != 0) updateDescriptorSet();
    return true;
}

void LightDataResolver::createLightBuffer(std::size_t size)
{
    DEBUG_FUNCTION();
    base_ref->get().allocator.destroyBuffer(frame.omniLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.directLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.spotLightBuffer);

    frame.omniLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::PointLight>(
        size, "Point light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.directLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::DirectionalLight>(
        size, "Directional light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.spotLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::SpotLight>(
        size, "Spot light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
}

void LightDataResolver::updateDescriptorSet()
{
    DEBUG_FUNCTION();
    auto omniLightInfo = frame.omniLightBuffer.getBufferInfo();
    auto directLightInfo = frame.directLightBuffer.getBufferInfo();
    auto spotLightInfo = frame.spotLightBuffer.getBufferInfo();
    std::vector<vk::WriteDescriptorSet> descriptorWrites = {
        {
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &directLightInfo,
        },
        {
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &spotLightInfo,
        },
        {
            .dstSet = descriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &omniLightInfo,
        },
    };
    base_ref->get().device.updateDescriptorSets(descriptorWrites, 0);
}

}    // namespace pivot::graphics

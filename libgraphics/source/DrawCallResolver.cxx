#include "pivot/graphics/DrawCallResolver.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/pivot.hxx"
#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

DrawCallResolver::DrawCallResolver() {}

DrawCallResolver::~DrawCallResolver() {}

void DrawCallResolver::init(VulkanBase &base, AssetStorage &stor, DescriptorBuilder &builder)
{

    DEBUG_FUNCTION
    base_ref = base;
    storage_ref = stor;
    createBuffer(defaultBufferSize);
    createLightBuffer();

    auto success = builder
                       .bindBuffer(0, frame.objectBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute)
                       .bindBuffer(1, frame.indirectBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eCompute)
                       .bindBuffer(2, frame.omniLightBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eFragment)
                       .bindBuffer(3, frame.directLightBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eFragment)
                       .bindBuffer(4, frame.spotLightBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eFragment)
                       .build(base_ref->get().device, frame.objectDescriptor, descriptorSetLayout);
    pivot_assert(success);
    vk_debug::setObjectName(base_ref->get().device, frame.objectDescriptor,
                            "Object Descriptor Set " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    updateDescriptorSet(defaultBufferSize);
}

void DrawCallResolver::destroy()
{
    DEBUG_FUNCTION
    if (frame.indirectBuffer) base_ref->get().allocator.destroyBuffer(frame.indirectBuffer);
    if (frame.objectBuffer) base_ref->get().allocator.destroyBuffer(frame.objectBuffer);
    if (frame.omniLightBuffer) base_ref->get().allocator.destroyBuffer(frame.omniLightBuffer);
    if (frame.directLightBuffer) base_ref->get().allocator.destroyBuffer(frame.directLightBuffer);
    if (frame.spotLightBuffer) base_ref->get().allocator.destroyBuffer(frame.spotLightBuffer);
}

void DrawCallResolver::prepareForDraw(DrawCallResolver::DrawSceneInformation sceneInformation)
{
    std::vector<gpu_object::UniformBufferObject> objectGPUData;
    std::uint32_t drawCount = 0;

    frame.packedDraws.clear();
    frame.pipelineBatch.clear();
    // std::ranges::sort(sceneInformation, {},
    //                   [](const auto &info) { return std::make_tuple(info.get().pipelineID, info.get().meshID); });

    pivot_assert(sceneInformation.renderObjects.objects.get().size() ==
                 sceneInformation.renderObjects.exist.get().size());
    pivot_assert(sceneInformation.transform.objects.get().size() == sceneInformation.transform.exist.get().size());
    pivot_assert(sceneInformation.pointLight.objects.get().size() == sceneInformation.pointLight.exist.get().size());
    pivot_assert(sceneInformation.directionalLight.objects.get().size() ==
                 sceneInformation.directionalLight.exist.get().size());
    pivot_assert(sceneInformation.spotLight.objects.get().size() == sceneInformation.spotLight.exist.get().size());

    for (unsigned i = 0;
         i < sceneInformation.renderObjects.objects.get().size() && i < sceneInformation.transform.objects.get().size();
         i++) {
        if (!sceneInformation.renderObjects.exist.get().at(i) || !sceneInformation.renderObjects.exist.get().at(i))
            continue;
        const auto &object = sceneInformation.renderObjects.objects.get().at(i);
        const auto &transform = sceneInformation.transform.objects.get().at(i);

        // TODO: better Pipeline batch
        if (frame.pipelineBatch.empty() || frame.pipelineBatch.back().pipelineID != object.pipelineID) {
            frame.pipelineBatch.push_back({
                .pipelineID = object.pipelineID,
                .first = drawCount,
                .size = 0,
            });
        }
        auto prefab = storage_ref->get().get_optional<AssetStorage::Prefab>(object.meshID);
        if (prefab.has_value()) {
            for (const auto &model: prefab->get().modelIds) {
                frame.pipelineBatch.back().size += 1;
                frame.packedDraws.push_back({
                    .meshId = model,
                    .first = drawCount++,
                    .count = 1,
                });
                auto obj = object;
                obj.meshID = model;
                objectGPUData.emplace_back(transform, obj, storage_ref->get());
            }
        }
    }
    pivot_assert(frame.packedDraws.size() == objectGPUData.size());
    if (objectGPUData.empty()) return;
    if (objectGPUData.size() > frame.currentBufferSize || objectGPUData.size() < frame.currentBufferSize / 2) {
        createBuffer(objectGPUData.size());
    }
    updateDescriptorSet(objectGPUData.size());

    pivot_assert(frame.currentBufferSize > 0);
    pivot_assert(frame.currentDescriptorSetSize > 0);
    pivot_assert(frame.currentBufferSize > frame.currentDescriptorSetSize);

    base_ref->get().allocator.copyBuffer(frame.objectBuffer, std::span(objectGPUData));

    frame.pointLightCount = handleLights(frame.omniLightBuffer, sceneInformation.pointLight, sceneInformation.transform,
                                         "Point light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.directionalLightCount =
        handleLights(frame.directLightBuffer, sceneInformation.directionalLight, sceneInformation.transform,
                     "Directionnal light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.spotLightCount = handleLights(frame.spotLightBuffer, sceneInformation.spotLight, sceneInformation.transform,
                                        "Spot light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));

    auto sceneData = frame.indirectBuffer.getMappedSpan();
    for (uint32_t i = 0; i < frame.packedDraws.size(); i++) {
        const auto &mesh = storage_ref->get().get<AssetStorage::Mesh>(frame.packedDraws.at(i).meshId);

        sceneData[i] = vk::DrawIndexedIndirectCommand{
            .indexCount = mesh.indicesSize,
            .instanceCount = 0,
            .firstIndex = mesh.indicesOffset,
            .vertexOffset = static_cast<int32_t>(mesh.vertexOffset),
            .firstInstance = i,
        };
    }
}

void DrawCallResolver::createBuffer(vk::DeviceSize bufferSize)
{
    destroy();

    frame.indirectBuffer = base_ref->get().allocator.createMappedBuffer<vk::DrawIndexedIndirectCommand>(
        bufferSize, "Indirect Command Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)),
        vk::BufferUsageFlagBits::eIndirectBuffer);

    frame.objectBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::UniformBufferObject>(
        bufferSize, "Uniform Buffer Object Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.currentBufferSize = bufferSize;
}

void DrawCallResolver::createLightBuffer()
{
    frame.omniLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::PointLight>(
        1, "Point light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.directLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::DirectionalLight>(
        1, "Directional light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.spotLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::SpotLight>(
        1, "Spot light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
}

void DrawCallResolver::updateDescriptorSet(vk::DeviceSize bufferSize)
{
    pivot_assert(bufferSize > 0);
    auto bufferInfo = frame.objectBuffer.getBufferInfo();
    auto indirectInfo = frame.indirectBuffer.getBufferInfo();
    auto omniLightInfo = frame.omniLightBuffer.getBufferInfo();
    auto directLightInfo = frame.directLightBuffer.getBufferInfo();
    auto spotLightInfo = frame.spotLightBuffer.getBufferInfo();
    std::vector<vk::WriteDescriptorSet> descriptorWrites{
        {
            .dstSet = frame.objectDescriptor,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &bufferInfo,
        },
        {
            .dstSet = frame.objectDescriptor,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &indirectInfo,
        },
        {
            .dstSet = frame.objectDescriptor,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &omniLightInfo,
        },
        {
            .dstSet = frame.objectDescriptor,
            .dstBinding = 3,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &directLightInfo,
        },
        {
            .dstSet = frame.objectDescriptor,
            .dstBinding = 4,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &spotLightInfo,
        },
    };
    base_ref->get().device.updateDescriptorSets(descriptorWrites, 0);
    frame.currentDescriptorSetSize = bufferSize;
}

}    // namespace pivot::graphics

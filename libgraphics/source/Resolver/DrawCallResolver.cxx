#include "pivot/graphics/Resolver/DrawCallResolver.hxx"

#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics
{

vk::DescriptorSetLayout DrawCallResolver::descriptorSetLayout = VK_NULL_HANDLE;

bool DrawCallResolver::initialize(VulkanBase &base, const AssetStorage &stor, DescriptorBuilder &builder)
{

    DEBUG_FUNCTION
    base_ref = base;
    storage_ref = stor;
    createBuffer(defaultBufferSize);

    auto success = builder
                       .bindBuffer(0, frame.indirectBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eCompute)
                       .bindBuffer(1, frame.objectBuffer.getBufferInfo(), vk::DescriptorType::eStorageBuffer,
                                   vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute)
                       .build(base_ref->get().device, frame.objectDescriptor, descriptorSetLayout);

    if (!verifyAlwaysMsg(success, "Descriptor set failed to build.")) { return success; }

    vk_debug::setObjectName(base_ref->get().device, frame.objectDescriptor,
                            "Object Descriptor Set " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    vk_debug::setObjectName(base_ref->get().device, frame.objectDescriptor,
                            "Object Descriptor Set Layout " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    updateDescriptorSet();
    return true;
}

bool DrawCallResolver::destroy(VulkanBase &)
{
    DEBUG_FUNCTION
    base_ref->get().allocator.destroyBuffer(frame.indirectBuffer);
    base_ref->get().allocator.destroyBuffer(frame.objectBuffer);
    return true;
}

bool DrawCallResolver::prepareForDraw(const DrawSceneInformation &sceneInformation)
{
    DEBUG_FUNCTION
    std::vector<gpu_object::UniformBufferObject> objectGPUData;
    std::uint32_t drawCount = 0;

    frame.packedDraws.clear();
    frame.pipelineBatch.clear();

    pivotAssertMsg(sceneInformation.renderObjects.objects.get().size() ==
                       sceneInformation.renderObjects.exist.get().size(),
                   "ECS Render Object arrays are invalid.");
    pivotAssertMsg(sceneInformation.transform.objects.get().size() == sceneInformation.transform.exist.get().size(),
                   "ECS Transform arrays are invalid.");

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
        auto prefab = storage_ref->get().get_optional<asset::Prefab>(object.meshID);
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
    pivotAssertMsg(frame.packedDraws.size() == objectGPUData.size(),
                   "Incorrect size between draw call and buffer data");
    if (objectGPUData.empty()) return true;
    if (objectGPUData.size() > frame.currentBufferSize || objectGPUData.size() < frame.currentBufferSize / 2) {
        createBuffer(objectGPUData.size());
    }
    updateDescriptorSet();

    pivotAssertMsg(frame.currentBufferSize > 0, "Buffer size is 0");

    base_ref->get().allocator.copyBuffer(frame.objectBuffer, std::span(objectGPUData));

    auto sceneData = frame.indirectBuffer.getMappedSpan();
    for (uint32_t i = 0; i < frame.packedDraws.size(); i++) {
        const auto &mesh = storage_ref->get().get<asset::Mesh>(frame.packedDraws.at(i).meshId);

        sceneData[i] = vk::DrawIndexedIndirectCommand{
            .indexCount = mesh.indicesSize,
            .instanceCount = 0,
            .firstIndex = mesh.indicesOffset,
            .vertexOffset = static_cast<int32_t>(mesh.vertexOffset),
            .firstInstance = i,
        };
    }
    return true;
}

void DrawCallResolver::createBuffer(vk::DeviceSize bufferSize)
{
    DEBUG_FUNCTION
    base_ref->get().allocator.destroyBuffer(frame.indirectBuffer);
    base_ref->get().allocator.destroyBuffer(frame.objectBuffer);

    frame.indirectBuffer = base_ref->get().allocator.createMappedBuffer<vk::DrawIndexedIndirectCommand>(
        bufferSize, "Indirect Command Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)),
        vk::BufferUsageFlagBits::eIndirectBuffer);

    frame.objectBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::UniformBufferObject>(
        bufferSize, "Uniform Buffer Object Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.currentBufferSize = bufferSize;
}

void DrawCallResolver::updateDescriptorSet()
{
    DEBUG_FUNCTION
    auto bufferInfo = frame.objectBuffer.getBufferInfo();
    auto indirectInfo = frame.indirectBuffer.getBufferInfo();

    verifyMsg(bufferInfo.range != 0, "The bufferInfo is empty ! This will trigger validation warnings");
    verifyMsg(indirectInfo.range != 0, "The indirectInfo is empty ! This will trigger validation warnings");

    std::vector<vk::WriteDescriptorSet> descriptorWrites{
        {
            .dstSet = frame.objectDescriptor,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &indirectInfo,
        },
        {
            .dstSet = frame.objectDescriptor,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &bufferInfo,
        },
    };
    base_ref->get().device.updateDescriptorSets(descriptorWrites, 0);
}

}    // namespace pivot::graphics

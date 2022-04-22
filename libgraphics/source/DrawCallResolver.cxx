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

    vk::DescriptorBufferInfo bufferInfo{
        .buffer = frame.objectBuffer.buffer,
        .offset = 0,
        .range = sizeof(gpu_object::UniformBufferObject) * defaultBufferSize,
    };
    vk::DescriptorBufferInfo indirectInfo{
        .buffer = frame.indirectBuffer.buffer,
        .offset = 0,
        .range = sizeof(vk::DrawIndexedIndirectCommand) * defaultBufferSize,
    };
    auto success =
        builder
            .bindBuffer(0, bufferInfo, vk::DescriptorType::eStorageBuffer,
                        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute)
            .bindBuffer(1, indirectInfo, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute)
            .build(base_ref->get().device, frame.objectDescriptor, descriptorSetLayout);
    assert(success);
    vk_debug::setObjectName(base_ref->get().device, frame.objectDescriptor,
                            "Object Descriptor Set " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    updateDescriptorSet(defaultBufferSize);
}

void DrawCallResolver::destroy()
{
    DEBUG_FUNCTION
    if (frame.indirectBuffer) base_ref->get().allocator.destroyBuffer(frame.indirectBuffer);
    if (frame.objectBuffer) base_ref->get().allocator.destroyBuffer(frame.objectBuffer);
}

void DrawCallResolver::prepareForDraw(std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation)
{
    frame.packedDraws.clear();
    frame.pipelineBatch.clear();
    std::vector<gpu_object::UniformBufferObject> objectGPUData;
    std::uint32_t drawCount = 0;

    std::ranges::sort(sceneInformation, {},
                      [](const auto &info) { return std::make_tuple(info.get().pipelineID, info.get().meshID); });

    for (const auto &object: sceneInformation) {
        if (frame.pipelineBatch.empty() || frame.pipelineBatch.back().pipelineID != object.get().pipelineID) {
            frame.pipelineBatch.push_back({
                .pipelineID = object.get().pipelineID,
                .first = drawCount,
                .size = 0,
            });
        }
        auto prefab = storage_ref->get().get_optional<AssetStorage::Prefab>(object.get().meshID);
        if (prefab.has_value()) {
            for (const auto &model: prefab->get().modelIds) {
                frame.pipelineBatch.back().size += 1;
                frame.packedDraws.push_back({
                    .meshId = model,
                    .first = drawCount++,
                    .count = 1,
                });
                auto obj = object.get();
                obj.meshID = model;
                objectGPUData.push_back(gpu_object::UniformBufferObject(obj, *storage_ref));
            }
        }
    }
    assert(frame.packedDraws.size() == objectGPUData.size());

    auto bufferCmp = objectGPUData.size() <=> frame.currentBufferSize;
    auto descriptorCmp = objectGPUData.size() <=> frame.currentDescriptorSetSize;
    if (std::is_gt(bufferCmp)) {
        createBuffer(objectGPUData.size());
        updateDescriptorSet(objectGPUData.size());
    }
    if (objectGPUData.size() > 0 && (std::is_lt(bufferCmp) || std::is_gt(descriptorCmp))) {
        updateDescriptorSet(objectGPUData.size());
    }

    if (frame.currentBufferSize > 0) {
        base_ref->get().allocator.copyBuffer(frame.objectBuffer, std::span(objectGPUData));

        auto sceneData = frame.indirectBuffer.getMappedSpan();
        for (uint32_t i = 0; i < frame.packedDraws.size(); i++) {
            const auto &mesh = storage_ref->get().get<AssetStorage::Mesh>(frame.packedDraws.at(i).meshId);

            sceneData[i].firstIndex = mesh.indicesOffset;
            sceneData[i].indexCount = mesh.indicesSize;
            sceneData[i].vertexOffset = mesh.vertexOffset;
            sceneData[i].instanceCount = 0;
            sceneData[i].firstInstance = i;
        }
    }
}

void DrawCallResolver::createBuffer(vk::DeviceSize bufferSize)
{
    if (frame.indirectBuffer) base_ref->get().allocator.destroyBuffer(frame.indirectBuffer);
    if (frame.objectBuffer) base_ref->get().allocator.destroyBuffer(frame.objectBuffer);

    frame.indirectBuffer = base_ref->get().allocator.createBuffer<vk::DrawIndexedIndirectCommand>(
        bufferSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer,
        vma::MemoryUsage::eCpuToGpu, vma::AllocationCreateFlagBits::eMapped);
    vk_debug::setObjectName(base_ref->get().device, frame.indirectBuffer.buffer,
                            "Indirect Command Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));

    frame.objectBuffer = base_ref->get().allocator.createBuffer<gpu_object::UniformBufferObject>(
        bufferSize, vk::BufferUsageFlagBits::eStorageBuffer, vma::MemoryUsage::eCpuToGpu);
    vk_debug::setObjectName(base_ref->get().device, frame.objectBuffer.buffer,
                            "Object Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.currentBufferSize = bufferSize;
}

void DrawCallResolver::updateDescriptorSet(vk::DeviceSize bufferSize)
{
    assert(bufferSize > 0);
    vk::DescriptorBufferInfo bufferInfo{
        .buffer = frame.objectBuffer.buffer,
        .offset = 0,
        .range = sizeof(gpu_object::UniformBufferObject) * bufferSize,
    };
    vk::DescriptorBufferInfo indirectInfo{
        .buffer = frame.indirectBuffer.buffer,
        .offset = 0,
        .range = sizeof(vk::DrawIndexedIndirectCommand) * bufferSize,
    };
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
    };
    base_ref->get().device.updateDescriptorSets(descriptorWrites, 0);
    frame.currentDescriptorSetSize = bufferSize;
}
}    // namespace pivot::graphics

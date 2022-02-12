#include "pivot/graphics/DrawCallResolver.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"

namespace pivot::graphics
{

DrawCallResolver::DrawCallResolver(VulkanBase &base, AssetStorage &stor): base_ref(base), storage_ref(stor) {}

DrawCallResolver::~DrawCallResolver() {}

void DrawCallResolver::init()
{
    DEBUG_FUNCTION
    createDescriptorPool();
    createDescriptorSetLayout();
    for (auto &frame: frames) {
        createBuffers(frame, defaultBufferSize);
        createDescriptorSets(frame, defaultBufferSize);
    }
}

void DrawCallResolver::destroy()
{
    DEBUG_FUNCTION
    for (auto &frame: frames) {
        if (frame.indirectBuffer)
            base_ref->get().allocator.destroyBuffer(frame.indirectBuffer.buffer, frame.indirectBuffer.memory);
        if (frame.objectBuffer)
            base_ref->get().allocator.destroyBuffer(frame.objectBuffer.buffer, frame.objectBuffer.memory);
    }
    if (descriptorPool) base_ref->get().device.destroyDescriptorPool(descriptorPool);
    if (descriptorSetLayout) base_ref->get().device.destroyDescriptorSetLayout(descriptorSetLayout);
}

void DrawCallResolver::prepareForDraw(std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation,
                                      const uint32_t frameIndex)
{
    auto &frame = frames.at(frameIndex);
    frame.packedDraws.clear();
    frame.pipelineBatch.clear();
    std::vector<gpu_object::UniformBufferObject> objectGPUData;
    std::uint32_t drawCount = 0;

    std::sort(sceneInformation.begin(), sceneInformation.end(), [](const auto &first, const auto &second) {
        return first.get().pipelineID == second.get().pipelineID && first.get().meshID == second.get().meshID;
    });
    for (const auto &object: sceneInformation) {
        if (frame.pipelineBatch.empty() || frame.pipelineBatch.back().pipelineID != object.get().pipelineID) {
            frame.pipelineBatch.push_back({
                .pipelineID = object.get().pipelineID,
                .first = drawCount,
                .size = 0,
            });
        }
        for (const auto &model: storage_ref->get().get<AssetStorage::Prefab>(object.get().meshID).modelIds) {
            frame.pipelineBatch.back().size += 1;
            frame.packedDraws.push_back({
                .meshId = model,
                .first = drawCount++,
                .count = 1,
            });
            objectGPUData.push_back(gpu_object::UniformBufferObject(
                {
                    .meshID = model,
                    .objectInformation = object.get().objectInformation,
                },
                *storage_ref));
        }
    }
    assert(frame.packedDraws.size() == objectGPUData.size());

    auto bufferCmp = objectGPUData.size() <=> frame.currentBufferSize;
    auto descriptorCmp = objectGPUData.size() <=> frame.currentDescriptorSetSize;
    if (std::is_gt(bufferCmp)) {
        createBuffers(frame, objectGPUData.size());
        createDescriptorSets(frame, objectGPUData.size());
    }
    if (objectGPUData.size() > 0 && (std::is_lt(bufferCmp) || std::is_gt(descriptorCmp))) {
        createDescriptorSets(frame, objectGPUData.size());
    }

    if (frame.currentBufferSize > 0) {
        frame.objectBuffer.copyBuffer(base_ref->get().allocator, objectGPUData);

        auto *sceneData =
            (vk::DrawIndexedIndirectCommand *)base_ref->get().allocator.mapMemory(frame.indirectBuffer.memory);
        for (uint32_t i = 0; i < frame.packedDraws.size(); i++) {
            const auto &mesh = storage_ref->get().get<AssetStorage::Mesh>(frame.packedDraws.at(i).meshId);

            sceneData[i].firstIndex = mesh.indicesOffset;
            sceneData[i].indexCount = mesh.indicesSize;
            sceneData[i].vertexOffset = mesh.vertexOffset;
            sceneData[i].instanceCount = 0;
            sceneData[i].firstInstance = i;
        }
        base_ref->get().allocator.unmapMemory(frame.indirectBuffer.memory);
    }
}

void DrawCallResolver::createDescriptorPool()
{
    vk::DescriptorPoolSize poolSize[] = {
        {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = MaxFrameInFlight,
        },
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MaxFrameInFlight,
        .poolSizeCount = std::size(poolSize),
        .pPoolSizes = poolSize,
    };
    descriptorPool = base_ref->get().device.createDescriptorPool(poolInfo);
    vk_debug::setObjectName(base_ref->get().device, descriptorPool, "Objects DescriptorPool");
}

void DrawCallResolver::createBuffers(Frame &frame, const auto bufferSize)
{
    if (frame.indirectBuffer)
        base_ref->get().allocator.destroyBuffer(frame.indirectBuffer.buffer, frame.indirectBuffer.memory);
    if (frame.objectBuffer)
        base_ref->get().allocator.destroyBuffer(frame.objectBuffer.buffer, frame.objectBuffer.memory);

    frame.indirectBuffer =
        AllocatedBuffer::create(base_ref->get(), sizeof(vk::DrawIndexedIndirectCommand) * bufferSize,
                                vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer,
                                vma::MemoryUsage::eCpuToGpu);
    vk_debug::setObjectName(base_ref->get().device, frame.indirectBuffer.buffer,
                            "Indirect Command Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));

    frame.objectBuffer = AllocatedBuffer::create(base_ref->get(), sizeof(gpu_object::UniformBufferObject) * bufferSize,
                                                 vk::BufferUsageFlagBits::eStorageBuffer, vma::MemoryUsage::eCpuToGpu);
    vk_debug::setObjectName(base_ref->get().device, frame.objectBuffer.buffer,
                            "Object Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.currentBufferSize = bufferSize;
}

void DrawCallResolver::createDescriptorSets(Frame &frame, const auto bufferSize)
{
    assert(bufferSize > 0);
    if (frame.objectDescriptor) base_ref->get().device.freeDescriptorSets(descriptorPool, frame.objectDescriptor);

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };

    frame.objectDescriptor = base_ref->get().device.allocateDescriptorSets(allocInfo).front();

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

void DrawCallResolver::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute,
    };
    vk::DescriptorSetLayoutBinding indirectBuffer{
        .binding = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eCompute,
    };

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {uboLayoutBinding, indirectBuffer};
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    descriptorSetLayout = base_ref->get().device.createDescriptorSetLayout(layoutInfo);
    vk_debug::setObjectName(base_ref->get().device, descriptorSetLayout, "Object DescriptorSet Layout");
}

}    // namespace pivot::graphics
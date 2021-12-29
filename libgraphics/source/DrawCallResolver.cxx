#include "pivot/graphics/DrawCallResolver.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/culling.hxx"
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

void DrawCallResolver::prepareForDraw(const std::vector<std::reference_wrapper<const RenderObject>> &sceneInformation,
                                      const gpuObject::CameraData &camera, const uint32_t frameIndex)
{
    auto &frame = frames.at(frameIndex);
    std::vector<DrawBatch> packedDraws;
    std::vector<gpuObject::UniformBufferObject> objectGPUData;
    uint32_t drawCount = 0;

    for (const auto &object: sceneInformation) {
        const auto &boundingBox = storage_ref->get().get<MeshBoundingBox>(object.get().meshID);
        packedDraws.push_back({
            .meshId = object.get().meshID,
            .first = drawCount++,
            .count = pivot::graphics::culling::should_object_be_rendered(object.get().objectInformation.transform,
                                                                         boundingBox, camera),
        });
        objectGPUData.push_back(gpuObject::UniformBufferObject(object.get().objectInformation, *storage_ref));
    }
    assert(packedDraws.size() == objectGPUData.size());
    if (objectGPUData.size() > frame.currentBufferSize) {
        recreateBuffers(objectGPUData.size(), frameIndex);
        frame.currentBufferSize = objectGPUData.size();
    }

    if (frame.currentBufferSize > 0) {
        vk_utils::copyBuffer(base_ref->get().allocator, frame.objectBuffer, objectGPUData);

        auto *sceneData =
            (VkDrawIndexedIndirectCommand *)base_ref->get().allocator.mapMemory(frame.indirectBuffer.memory);
        for (uint32_t i = 0; i < packedDraws.size(); i++) {
            const auto &mesh = storage_ref->get().get<pivot::graphics::AssetStorage::Mesh>(packedDraws[i].meshId);

            sceneData[i].firstIndex = mesh.indicesOffset;
            sceneData[i].indexCount = mesh.indicesSize;
            sceneData[i].vertexOffset = mesh.vertexOffset;
            sceneData[i].instanceCount = 1;
            sceneData[i].firstInstance = i;
        }
        base_ref->get().allocator.unmapMemory(frame.indirectBuffer.memory);
    }
}

void DrawCallResolver::recreateBuffers(const auto newSize, const std::uint32_t frameIndex)
{
    auto &frame = frames.at(frameIndex);
    base_ref->get().device.freeDescriptorSets(descriptorPool, frame.objectDescriptor);
    base_ref->get().allocator.destroyBuffer(frame.indirectBuffer.buffer, frame.indirectBuffer.memory);
    base_ref->get().allocator.destroyBuffer(frame.objectBuffer.buffer, frame.objectBuffer.memory);

    createBuffers(frame, newSize);
    createDescriptorSets(frame, newSize);
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
}

void DrawCallResolver::createBuffers(Frame &frame, const auto bufferSize)
{
    frame.indirectBuffer =
        vk_utils::createBuffer(base_ref->get().allocator, sizeof(vk::DrawIndexedIndirectCommand) * bufferSize,
                               vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer,
                               vma::MemoryUsage::eCpuToGpu);
    frame.objectBuffer =
        vk_utils::createBuffer(base_ref->get().allocator, sizeof(gpuObject::UniformBufferObject) * bufferSize,
                               vk::BufferUsageFlagBits::eStorageBuffer, vma::MemoryUsage::eCpuToGpu);
}

void DrawCallResolver::createDescriptorSets(Frame &frame, const auto bufferSize)
{
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };

    frame.objectDescriptor = base_ref->get().device.allocateDescriptorSets(allocInfo).front();

    vk::DescriptorBufferInfo bufferInfo{
        .buffer = frame.objectBuffer.buffer,
        .offset = 0,
        .range = sizeof(gpuObject::UniformBufferObject) * bufferSize,
    };
    std::vector<vk::WriteDescriptorSet> descriptorWrites{{
        .dstSet = frame.objectDescriptor,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &bufferInfo,
    }};
    base_ref->get().device.updateDescriptorSets(descriptorWrites, 0);
}

void DrawCallResolver::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr,
    };

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {uboLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    descriptorSetLayout = base_ref->get().device.createDescriptorSetLayout(layoutInfo);
}

}    // namespace pivot::graphics
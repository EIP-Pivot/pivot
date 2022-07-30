#include "pivot/graphics/DrawCallResolver.hxx"

#include "pivot/graphics/types/UniformBufferObject.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_utils.hxx"
#include "pivot/pivot.hxx"

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
    pivot_assert(success, "Descriptor set failed to build.");
    vk_debug::setObjectName(base_ref->get().device, frame.objectDescriptor,
                            "Object Descriptor Set " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    updateDescriptorSet(defaultBufferSize);
}

void DrawCallResolver::destroy()
{
    DEBUG_FUNCTION
    base_ref->get().allocator.destroyBuffer(frame.indirectBuffer);
    base_ref->get().allocator.destroyBuffer(frame.objectBuffer);
    base_ref->get().allocator.destroyBuffer(frame.omniLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.directLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.spotLightBuffer);
}

void DrawCallResolver::prepareForDraw(DrawCallResolver::DrawSceneInformation sceneInformation)
{
    std::vector<gpu_object::UniformBufferObject> objectGPUData;

    frame.packedDraws.clear();
    frame.pipelineBatch.clear();

    pivot_assert(sceneInformation.renderObjects.objects.get().size() ==
                     sceneInformation.renderObjects.exist.get().size(),
                 "ECS Render Object arrays are invalid.");
    pivot_assert(sceneInformation.transform.objects.get().size() == sceneInformation.transform.exist.get().size(),
                 "ECS Transform arrays are invalid.");
    pivot_assert(sceneInformation.pointLight.objects.get().size() == sceneInformation.pointLight.exist.get().size(),
                 "ECS Point tights arrays are invalid.");
    pivot_assert(sceneInformation.directionalLight.objects.get().size() ==
                     sceneInformation.directionalLight.exist.get().size(),
                 "ECS Directional lights arrays are invalid.");
    pivot_assert(sceneInformation.spotLight.objects.get().size() == sceneInformation.spotLight.exist.get().size(),
                 "ECS Spot light arrays are invalid.");

    std::uint32_t drawCount = 0;
    for (unsigned i = 0;
         i < sceneInformation.renderObjects.objects.get().size() && i < sceneInformation.transform.objects.get().size();
         i++) {
        if (!sceneInformation.renderObjects.exist.get()[i] || !sceneInformation.transform.exist.get()[i]) continue;
        const auto &object = sceneInformation.renderObjects.objects.get()[i];
        const auto &transform = sceneInformation.transform.objects.get()[i];

        // TODO: better Pipeline batch
        if (frame.pipelineBatch.empty() || frame.pipelineBatch.back().pipelineID != object.pipelineID) {
            frame.pipelineBatch.push_back({
                .pipelineID = object.pipelineID,
                .first = drawCount,
                .size = 0,
            });
        }
        auto model = storage_ref->get().get_optional<asset::ModelPtr>(object.meshID);
        if (!model.has_value()) continue;
        model.value().get()->traverseDown([&](const auto &prefab) mutable {
            glm::mat4 modelMatrix = transform.getModelMatrix() * prefab.value.localMatrix;
            for (const auto &primitive: prefab.value.primitives) {
                frame.pipelineBatch.back().size += 1;
                frame.packedDraws.push_back(DrawBatch{
                    .primitive = primitive,
                    .first = drawCount++,
                    .count = 1,
                });
                std::uint32_t materialIndex = -1;
                if (!object.materialIndex.empty()) {
                    materialIndex = storage_ref->get().getIndex<gpu_object::Material>(object.materialIndex);
                } else if (primitive.default_material) {
                    materialIndex =
                        storage_ref->get().getIndex<gpu_object::Material>(primitive.default_material.value());
                } else {
                    materialIndex = storage_ref->get().getIndex<gpu_object::Material>(asset::missing_material_name);
                }
                objectGPUData.push_back({
                    .modelMatrix = modelMatrix,
                    .materialIndex = materialIndex,
                    .boundingBoxIndex = storage_ref->get().getIndex<gpu_object::AABB>(primitive.name),
                });
            }
        });
    }
    pivot_assert(frame.packedDraws.size() == objectGPUData.size(), "Incorrect size between draw call and buffer data");
    if (objectGPUData.empty()) return;
    if (objectGPUData.size() > frame.currentBufferSize || objectGPUData.size() < frame.currentBufferSize / 2) {
        createBuffer(objectGPUData.size());
    }
    updateDescriptorSet(objectGPUData.size());

    pivot_assert(frame.currentBufferSize > 0, "Buffer size is 0");
    pivot_assert(frame.currentDescriptorSetSize > 0, "Descriptor set size are 0");

    base_ref->get().allocator.copyBuffer(frame.objectBuffer, std::span(objectGPUData));

    frame.pointLightCount = handleLights(frame.omniLightBuffer, sceneInformation.pointLight, sceneInformation.transform,
                                         "Point light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.directionalLightCount =
        handleLights(frame.directLightBuffer, sceneInformation.directionalLight, sceneInformation.transform,
                     "Directionnal light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.spotLightCount = handleLights(frame.spotLightBuffer, sceneInformation.spotLight, sceneInformation.transform,
                                        "Spot light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));

    auto sceneData = frame.indirectBuffer.getMappedSpan();
    std::uint32_t index = 0;
    for (const auto &packedDraw: frame.packedDraws) {
        // pivot_assert(index < frame.indirectBuffer.size, "Indirect buffer size is too small !");
        sceneData[index] = vk::DrawIndexedIndirectCommand{
            .indexCount = packedDraw.primitive.indicesSize,
            .instanceCount = 0,
            .firstIndex = packedDraw.primitive.indicesOffset,
            .vertexOffset = static_cast<int32_t>(packedDraw.primitive.vertexOffset),
            .firstInstance = index,
        };
        index += 1;
    }
}

void DrawCallResolver::createBuffer(vk::DeviceSize bufferSize)
{
    base_ref->get().allocator.destroyBuffer(frame.indirectBuffer);
    base_ref->get().allocator.destroyBuffer(frame.objectBuffer);

    frame.indirectBuffer = base_ref->get().allocator.createMappedBuffer<vk::DrawIndexedIndirectCommand>(
        bufferSize, "Indirect Command Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)),
        vk::BufferUsageFlagBits::eIndirectBuffer);

    frame.objectBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::UniformBufferObject>(
        bufferSize, "Uniform Buffer Object Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.currentBufferSize = bufferSize;
}

void DrawCallResolver::createLightBuffer()
{
    base_ref->get().allocator.destroyBuffer(frame.omniLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.directLightBuffer);
    base_ref->get().allocator.destroyBuffer(frame.spotLightBuffer);

    frame.omniLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::PointLight>(
        1, "Point light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.directLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::DirectionalLight>(
        1, "Directional light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
    frame.spotLightBuffer = base_ref->get().allocator.createMappedBuffer<gpu_object::SpotLight>(
        1, "Spot light Buffer " + std::to_string(reinterpret_cast<intptr_t>(&frame)));
}

void DrawCallResolver::updateDescriptorSet(vk::DeviceSize bufferSize)
{
    pivot_assert(bufferSize > 0, "Buffer size is 0");
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

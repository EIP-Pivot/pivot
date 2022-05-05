#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"

namespace pivot::graphics
{

void AssetStorage::build(DescriptorBuilder builder, CpuKeepFlags cpuKeep, GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    // check for incorrect combination of flags
    assert(std::popcount(static_cast<std::underlying_type_t<AssetStorage::GpuRessourceFlagBits>>(gpuFlag)) == 1);
    if (cpuKeep & CpuKeepFlagBits::eNone) {
        assert(std::popcount(static_cast<std::underlying_type_t<AssetStorage::CpuKeepFlagBits>>(cpuKeep)) == 1);
    }

    assert(cpuStorage.materialStaging.contains(missing_material_name));
    assert(cpuStorage.textureStaging.contains(missing_texture_name));
    vulkanDeletionQueue.flush();
    createTextureSampler();

    logger.info("Asset Storage") << prefabStorage.size() << " prefab loaded";
    logger.info("Asset Storage") << "Pushing " << modelStorage.size() << " models onto the GPU";
    pushModelsOnGPU(gpuFlag);

    logger.info("Asset Storage") << "Pushing " << cpuStorage.textureStaging.size() << " textures onto the GPU";
    pushTexturesOnGPU(gpuFlag);

    logger.info("Asset Storage") << "Pushing " << cpuStorage.materialStaging.size() << " materials onto the GPU";
    pushMaterialOnGPU(gpuFlag);

    createDescriptorSet(builder);

    CPUStorage newStorage;
    if (cpuKeep & CpuKeepFlagBits::eAll) {
        newStorage = std::move(cpuStorage);
    } else {
        if (cpuKeep & CpuKeepFlagBits::eTexture) { newStorage.textureStaging = std::move(cpuStorage.textureStaging); }
        if (cpuKeep & CpuKeepFlagBits::eMaterial) {
            newStorage.materialStaging = std::move(cpuStorage.materialStaging);
        }
        if (cpuKeep & CpuKeepFlagBits::eVertex) {
            newStorage.vertexStagingBuffer = std::move(cpuStorage.vertexStagingBuffer);
        }
        if (cpuKeep & CpuKeepFlagBits::eIndice) {
            newStorage.indexStagingBuffer = std::move(cpuStorage.indexStagingBuffer);
        }
    }
    cpuStorage = std::move(newStorage);
}

void AssetStorage::destroy()
{
    DEBUG_FUNCTION
    base_ref->get().allocator.destroyBuffer(vertexBuffer);
    base_ref->get().allocator.destroyBuffer(indicesBuffer);
    base_ref->get().allocator.destroyBuffer(materialBuffer);
    base_ref->get().allocator.destroyBuffer(AABBBuffer);

    for (auto &image: textureStorage.getStorage()) {
        base_ref->get().device.destroyImageView(image.imageView);
        base_ref->get().allocator.destroyImage(image);
    }
    vulkanDeletionQueue.flush();
}

template <typename T>
static void copy_with_staging_buffer(VulkanBase &base_ref, AssetStorage::GpuRessourceFlags gpuFlag,
                                     vk::BufferUsageFlags flag, std::vector<T> &data, AllocatedBuffer<T> &buffer)
{
    // check for incorrect combination of flags
    assert(std::popcount(static_cast<std::underlying_type_t<AssetStorage::GpuRessourceFlagBits>>(gpuFlag)) == 1);

    auto true_size = data.size();
    if (buffer &&
        (gpuFlag & (AssetStorage::GpuRessourceFlagBits::eAfter | AssetStorage::GpuRessourceFlagBits::eBefore))) {
        true_size += buffer.getSize();
    }
    auto staging = base_ref.allocator.createBuffer<T>(
        true_size, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eCpuToGpu);

    auto offset = 0;
    if (buffer && (gpuFlag & AssetStorage::GpuRessourceFlagBits::eAfter)) {
        base_ref.copyBuffer(buffer, staging);
        offset += buffer.getSize();
    }
    base_ref.allocator.copyBuffer(staging, std::span(data), offset);
    if (buffer && (gpuFlag & AssetStorage::GpuRessourceFlagBits::eBefore))
        base_ref.copyBuffer(buffer, staging, 0, data.size() * sizeof(T));
    if (true_size > buffer.getSize()) {
        base_ref.allocator.destroyBuffer(buffer);
        buffer = base_ref.allocator.createBuffer<T>(
            true_size, flag | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
            vma::MemoryUsage::eGpuOnly);
    }
    base_ref.copyBuffer(staging, buffer);
    base_ref.allocator.destroyBuffer(staging);
}

void AssetStorage::pushModelsOnGPU(GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    if (gpuFlag & GpuRessourceFlagBits::eClear) {
        modelPaths.clear();
    } else if (gpuFlag & (AssetStorage::GpuRessourceFlagBits::eBefore | AssetStorage::GpuRessourceFlagBits::eAfter)) {
        modelPaths.insert(cpuStorage.modelPaths.begin(), cpuStorage.modelPaths.end());
    }

    meshAABBStorage.clear();
    meshAABBStorage.reserve(modelStorage.size());
    for (const auto &[name, model]: modelStorage) {
        meshAABBStorage.add(name,
                            gpu_object::AABB(std::span(cpuStorage.vertexStagingBuffer.begin() + model.mesh.vertexOffset,
                                                       model.mesh.vertexSize)));
    }

    assert(modelStorage.size() == meshAABBStorage.size());
    if (cpuStorage.vertexStagingBuffer.empty()) {
        logger.warn("Asset Storage") << "No model to push";
        return;
    }
    if (meshAABBStorage.empty()) {
        logger.warn("Asset Storage") << "No AABB to push";
        return;
    }

    copy_with_staging_buffer(base_ref->get(), gpuFlag, vk::BufferUsageFlagBits::eVertexBuffer,
                             cpuStorage.vertexStagingBuffer, vertexBuffer);
    vk_debug::setObjectName(base_ref->get().device, vertexBuffer.buffer, "Vertex Buffer");

    copy_with_staging_buffer(base_ref->get(), gpuFlag, vk::BufferUsageFlagBits::eIndexBuffer,
                             cpuStorage.indexStagingBuffer, indicesBuffer);
    vk_debug::setObjectName(base_ref->get().device, indicesBuffer.buffer, "Indices Buffer");

    copy_with_staging_buffer(base_ref->get(), gpuFlag, vk::BufferUsageFlagBits::eStorageBuffer,
                             meshAABBStorage.getStorage(), AABBBuffer);
    vk_debug::setObjectName(base_ref->get().device, AABBBuffer.buffer, "AABB Buffer");
}

void AssetStorage::pushTexturesOnGPU(GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    if (gpuFlag & GpuRessourceFlagBits::eClear) {
        texturePaths.clear();
        for (auto &image: textureStorage.getStorage()) {
            base_ref->get().device.destroyImageView(image.imageView);
            base_ref->get().allocator.destroyImage(image);
        }
        textureStorage.clear();
    }
    bool isComplicated =
        bool(gpuFlag & (AssetStorage::GpuRessourceFlagBits::eBefore | AssetStorage::GpuRessourceFlagBits::eAfter));
    texturePaths.insert(cpuStorage.texturePaths.begin(), cpuStorage.texturePaths.end());

    if (cpuStorage.textureStaging.size() == 0) {
        logger.warn("Asset Storage") << "No textures to push";
        return;
    }
    for (auto &[name, idx]: cpuStorage.textureStaging) {
        if (name == missing_texture_name && isComplicated) continue;

        auto &img = cpuStorage.textureStaging.get(idx);
        auto stagingBuffer = base_ref->get().allocator.createBuffer<std::byte>(
            img.image.size(), vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
        base_ref->get().allocator.copyBuffer(stagingBuffer, std::span(img.image));

        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Srgb,
            .extent = img.size,
            .mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(img.size.width, img.size.height))) + 1),
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc |
                     vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined,
        };
        vma::AllocationCreateInfo allocInfo;
        allocInfo.usage = vma::MemoryUsage::eGpuOnly;
        auto image = base_ref->get().allocator.createImage(imageInfo, allocInfo);
        image.createImageView(base_ref->get().device);
        vk_debug::setObjectName(base_ref->get().device, image.image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, image.imageView, "Texture " + name + " ImageView");

        base_ref->get().transitionLayout(image, vk::ImageLayout::eTransferDstOptimal);
        base_ref->get().copyBufferToImage(stagingBuffer, image);
        base_ref->get().generateMipmaps(image, image.mipLevels);

        base_ref->get().allocator.destroyBuffer(stagingBuffer);
        logger.trace("Push Texture") << name;
        textureStorage.add(name, std::move(image));
    }
}

void AssetStorage::pushMaterialOnGPU(GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    if (gpuFlag & GpuRessourceFlagBits::eClear) materialStorage.clear();

    if (cpuStorage.materialStaging.empty()) {
        logger.warn("Asset Storage") << "No material to push";
        return;
    }
    bool isComplicated =
        bool(gpuFlag & (AssetStorage::GpuRessourceFlagBits::eBefore | AssetStorage::GpuRessourceFlagBits::eAfter));

    for (auto &[name, idx]: cpuStorage.materialStaging) {
        if (name == missing_material_name && isComplicated) continue;

        const auto &mat = cpuStorage.materialStaging.getStorage()[idx];
        materialStorage.add(
            name,
            {
                .baseColor = mat.baseColor,
                .metallic = mat.metallic,
                .roughness = mat.roughness,
                .baseColorTexture =
                    (mat.baseColorTexture.empty()) ? (-1) : (textureStorage.getIndex(mat.baseColorTexture)),
                .metallicRoughnessTexture = (mat.metallicRoughnessTexture.empty())
                                                ? (-1)
                                                : (textureStorage.getIndex(mat.metallicRoughnessTexture)),
                .normalTexture = (mat.normalTexture.empty()) ? (-1) : (textureStorage.getIndex(mat.normalTexture)),
                .occlusionTexture =
                    (mat.occlusionTexture.empty()) ? (-1) : (textureStorage.getIndex(mat.occlusionTexture)),
                .emissiveTexture =
                    (mat.emissiveTexture.empty()) ? (-1) : (textureStorage.getIndex(mat.emissiveTexture)),
            });
    }
    copy_with_staging_buffer(base_ref->get(), gpuFlag, vk::BufferUsageFlagBits::eStorageBuffer,
                             materialStorage.getStorage(), materialBuffer);
    vk_debug::setObjectName(base_ref->get().device, materialBuffer.buffer, "Material Buffer");
}

}    // namespace pivot::graphics

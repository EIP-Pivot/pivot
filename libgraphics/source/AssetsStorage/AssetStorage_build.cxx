#include "pivot/graphics/AssetStorage/AssetStorage.hxx"

#include "pivot/graphics/vk_debug.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics
{

static asset::CPUStorage
batch_load(const std::unordered_map<std::string, std::filesystem::path> &storage_map,
           const std::function<std::optional<asset::CPUStorage>(unsigned, const std::filesystem::path &)> load,
           const std::string &debug_name, ThreadPool &threadPool)
{
    asset::CPUStorage cpuStorage;
    std::vector<std::pair<std::filesystem::path, std::future<std::optional<asset::CPUStorage>>>> futures;
    futures.reserve(storage_map.size());

    /// Model must be loaded first, as they may add new texture to load
    for (const auto &i: storage_map) futures.emplace_back(i.second, threadPool.push(load, i.second));
    for (auto &[name, f]: futures) {
        auto storage = f.get();
        if (!storage.has_value())
            logger.warn("Asset Storage") << debug_name << " failed to load: " << name;
        else
            cpuStorage.merge(storage.value());
    }
    return cpuStorage;
}

void AssetStorage::build(DescriptorBuilder builder, BuildFlags flags)
{
    DEBUG_FUNCTION
    // check for incorrect combination of flags
    pivot_assert(std::popcount(static_cast<std::underlying_type_t<AssetStorage::BuildFlagBits>>(flags)) == 1,
                 "More than one BuildFlag is set !");

    // TODO: better separation of loading ressources
    modelStorage.clear();
    prefabStorage.clear();
    materialStorage.clear();
    meshAABBStorage.clear();
    for (auto &image: textureStorage.getStorage()) {
        base_ref->get().device.destroyImageView(image.imageView);
        base_ref->get().allocator.destroyImage(image);
    }
    textureStorage.clear();
    vulkanDeletionQueue.flush();

    createTextureSampler();

    cpuStorage.merge(asset::CPUStorage::default_assets());
    if (flags & (BuildFlagBits::eReloadOldAssets)) {
        cpuStorage.modelPaths.insert(modelPaths.begin(), modelPaths.end());
        cpuStorage.texturePaths.insert(texturePaths.begin(), texturePaths.end());
    }

    threadPool.start();
    cpuStorage += batch_load(cpuStorage.modelPaths, loadModel, "Model", threadPool);
    cpuStorage += batch_load(cpuStorage.texturePaths, loadTexture, "Texture", threadPool);

    modelStorage.swap(cpuStorage.modelStorage);
    prefabStorage.swap(cpuStorage.prefabStorage);
    modelPaths.swap(cpuStorage.modelPaths);
    texturePaths.swap(cpuStorage.texturePaths);

    logger.info("Asset Storage") << prefabStorage.size() << " prefab loaded";
    logger.info("Asset Storage") << "Pushing " << modelStorage.size() << " models onto the GPU";
    pushModelsOnGPU();

    logger.info("Asset Storage") << "Pushing " << cpuStorage.textureStaging.size() << " textures onto the GPU";
    pushTexturesOnGPU();

    logger.info("Asset Storage") << "Pushing " << cpuStorage.materialStaging.size() << " materials onto the GPU";
    pushMaterialOnGPU();

    createDescriptorSet(builder);

    cpuStorage = {};
    threadPool.stop();
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
    textureStorage.clear();
    vulkanDeletionQueue.flush();
}

template <typename T>
static void copy_with_staging_buffer(VulkanBase &base_ref, vk::BufferUsageFlags flag, std::vector<T> &data,
                                     AllocatedBuffer<T> &buffer)
{
    auto true_size = data.size();
    auto staging = base_ref.allocator.createBuffer<T>(
        true_size, vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eAuto, vma::AllocationCreateFlagBits::eHostAccessSequentialWrite);

    base_ref.allocator.copyBuffer(staging, std::span(data));
    if (true_size > buffer.getSize()) {
        base_ref.allocator.destroyBuffer(buffer);
        buffer = base_ref.allocator.createBuffer<T>(
            true_size, flag | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
            vma::MemoryUsage::eAutoPreferDevice);
    }
    base_ref.copyBuffer(staging, buffer);
    base_ref.allocator.destroyBuffer(staging);
}

void AssetStorage::pushModelsOnGPU()
{
    DEBUG_FUNCTION
    meshAABBStorage.clear();
    meshAABBStorage.reserve(modelStorage.size());

    if (cpuStorage.vertexStagingBuffer.empty()) {
        logger.warn("Asset Storage") << "No model to push";
        return;
    }

    for (const auto &[name, model]: modelStorage) {
        meshAABBStorage.add(name,
                            gpu_object::AABB(std::span(cpuStorage.vertexStagingBuffer.begin() + model.mesh.vertexOffset,
                                                       model.mesh.vertexSize)));
    }
    pivot_assert(modelStorage.size() == meshAABBStorage.size(), "The Model Storage is bigger than the AABB Storage.");

    copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eVertexBuffer, cpuStorage.vertexStagingBuffer,
                             vertexBuffer);
    vk_debug::setObjectName(base_ref->get().device, vertexBuffer.buffer, "Vertex Buffer");

    copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eIndexBuffer, cpuStorage.indexStagingBuffer,
                             indicesBuffer);
    vk_debug::setObjectName(base_ref->get().device, indicesBuffer.buffer, "Indices Buffer");

    copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer, meshAABBStorage.getStorage(),
                             AABBBuffer);
    vk_debug::setObjectName(base_ref->get().device, AABBBuffer.buffer, "AABB Buffer");
}

void AssetStorage::pushTexturesOnGPU()
{
    DEBUG_FUNCTION
    if (cpuStorage.textureStaging.size() == 0) {
        logger.warn("Asset Storage") << "No textures to push";
        return;
    }
    for (auto &[name, idx]: cpuStorage.textureStaging) {
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
        base_ref->get().allocator.setAllocationName(image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, image.image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, image.imageView, "Texture " + name + " ImageView");

        base_ref->get().transitionLayout(image, vk::ImageLayout::eTransferDstOptimal);
        base_ref->get().copyBufferToImage(stagingBuffer, image);
        base_ref->get().generateMipmaps(image, image.mipLevels);

        base_ref->get().allocator.destroyBuffer(stagingBuffer);
        textureStorage.add(name, std::move(image));
    }
}

void AssetStorage::pushMaterialOnGPU()
{
    DEBUG_FUNCTION
    if (cpuStorage.materialStaging.empty()) {
        logger.warn("Asset Storage") << "No material to push";
        return;
    }
    for (auto &[name, idx]: cpuStorage.materialStaging) {
        const auto &mat = cpuStorage.materialStaging.getStorage()[idx];
        materialStorage.add(
            name,
            {
                .alphaCutOff = mat.alphaCutOff,
                .metallicFactor = mat.metallicFactor,
                .roughnessFactor = mat.roughnessFactor,
                .baseColor = mat.baseColor,
                .baseColorFactor = mat.baseColorFactor,
                .emissiveFactor = mat.emissiveFactor,
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
    copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer, materialStorage.getStorage(),
                             materialBuffer);
    vk_debug::setObjectName(base_ref->get().device, materialBuffer.buffer, "Material Buffer");
}

}    // namespace pivot::graphics

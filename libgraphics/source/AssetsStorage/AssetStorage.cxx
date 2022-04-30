#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"

#include <Logger.hpp>

namespace pivot::graphics
{

const std::unordered_map<std::string, AssetStorage::AssetHandler> AssetStorage::supportedTexture = {
    {".png", &AssetStorage::loadPngTexture},
    {".jpg", &AssetStorage::loadJpgTexture},
    {".ktx", &AssetStorage::loadKtxImage},
};

const std::unordered_map<std::string, AssetStorage::AssetHandler> AssetStorage::supportedObject = {
    {".obj", &AssetStorage::loadObjModel},
    {".gltf", &AssetStorage::loadGltfModel},
};

static const AssetStorage::CPUTexture default_texture_data{

    .image =
        {
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0xff),

            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),

            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),

            std::byte(0x00),
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0xff),
        },
    .size = {2, 2, 1},
};

AssetStorage::CPUStorage::CPUStorage()
{
    textureStaging.add(missing_texture_name, default_texture_data);
    materialStaging.add(missing_material_name, {
                                                   .baseColorTexture = missing_material_name,
                                               });

    materialStaging.add("white", {});
}

AssetStorage::AssetStorage(VulkanBase &base): base_ref(base) {}

AssetStorage::~AssetStorage() {}

void AssetStorage::build(DescriptorBuilder builder, CpuKeepFlags cpuKeep, GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    assert(cpuStorage.materialStaging.contains(missing_material_name));
    assert(cpuStorage.textureStaging.contains(missing_texture_name));
    createTextureSampler();

    for (const auto &[name, model]: modelStorage) {
        meshAABBStorage.add(name,
                            gpu_object::AABB(std::span(cpuStorage.vertexStagingBuffer.begin() + model.mesh.vertexOffset,
                                                       model.mesh.vertexSize)));
    }
    assert(modelStorage.size() == meshAABBStorage.size());

    logger.info("Asset Storage") << prefabStorage.size() << " prefab loaded";
    logger.info("Asset Storage") << "Pushing " << modelStorage.size() << " models onto the GPU";
    pushModelsOnGPU(gpuFlag);
    vk_debug::setObjectName(base_ref->get().device, vertexBuffer.buffer, "Vertex Buffer");
    vk_debug::setObjectName(base_ref->get().device, indicesBuffer.buffer, "Indices Buffer");

    logger.info("Asset Storage") << "Pushing " << meshAABBStorage.size() << " AABB onto the GPU";
    pushAABBOnGPU(gpuFlag);
    vk_debug::setObjectName(base_ref->get().device, AABBBuffer.buffer, "AABB Buffer");

    logger.info("Asset Storage") << "Pushing " << cpuStorage.textureStaging.size() << " textures onto the GPU";
    pushTexturesOnGPU(gpuFlag);
    for (const auto &[name, idex]: textureStorage) {
        const auto &im = textureStorage.get(idex);
        vk_debug::setObjectName(base_ref->get().device, im.image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, im.imageView, "Texture " + name + " ImageView");
    }

    logger.info("Asset Storage") << "Pushing " << cpuStorage.materialStaging.size() << " materials onto the GPU";
    pushMaterialOnGPU(gpuFlag);
    vk_debug::setObjectName(base_ref->get().device, materialBuffer.buffer, "Material Buffer");

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

bool AssetStorage::bindForGraphics(vk::CommandBuffer &cmd, const vk::PipelineLayout &pipelineLayout,
                                   std::uint32_t descriptorSetNb)
{
    if (!vertexBuffer || !indicesBuffer || !descriptorSet) return false;

    vk::DeviceSize offset = 0;
    cmd.bindVertexBuffers(0, vertexBuffer.buffer, offset);
    cmd.bindIndexBuffer(indicesBuffer.buffer, 0, vk::IndexType::eUint32);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorSetNb, descriptorSet, nullptr);
    return true;
}

bool AssetStorage::bindForCompute(vk::CommandBuffer &cmd, const vk::PipelineLayout &pipelineLayout,
                                  std::uint32_t descriptorSetNb)
{
    if (!descriptorSet) return false;
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, descriptorSetNb, descriptorSet, nullptr);
    return true;
}

bool AssetStorage::loadAsset(const std::filesystem::path &path)
{
    auto iterModel = supportedObject.find(path.extension().string());
    if (iterModel != supportedObject.end()) { return loadModel(path); }

    auto iterTexture = supportedTexture.find(path.extension().string());
    if (iterTexture != supportedTexture.end()) { return loadTexture(path); }

    logger.err("LOAD ASSET") << "Not supported asset extension: " << path.extension();
    return false;
}

bool AssetStorage::loadModel(const std::filesystem::path &path)
{
    auto iter = supportedObject.find(path.extension().string());
    if (iter == supportedObject.end()) {
        logger.err("LOAD MODEL") << "Not supported model extension: " << path.extension();
        return false;
    }
    logger.info("Asset Storage") << "Loading model at : " << path;
    auto ret = std::apply(iter->second, std::make_tuple(this, path));
    if (ret) modelPaths[path.stem().string()] = path;
    return ret;
}

bool AssetStorage::loadTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    const auto iter = supportedTexture.find(path.extension().string());
    if (iter == supportedTexture.end()) {
        logger.err("Load Texture") << "Not supported texture extension: " << path.extension();
        return false;
    }
    logger.info("Asset Storage") << "Loading texture at : " << path;
    auto ret = std::apply(iter->second, std::make_tuple(this, path));
    if (ret) texturePaths[path.stem().string()] = path;
    return ret;
}

template <typename T>
static void copy_with_staging_buffer(VulkanBase &base_ref, AssetStorage::GpuRessourceFlags gpuFlag,
                                     vk::BufferUsageFlags flag, std::vector<T> &data, AllocatedBuffer<T> &buffer)
{
    bool isComplicated = bool(gpuFlag & (AssetStorage::GpuRessourceFlags(AssetStorage::GpuRessourceFlagBits::eAfter) |
                                         AssetStorage::GpuRessourceFlagBits::eBefore));
    auto true_size = data.size();
    if (buffer && isComplicated) true_size += buffer.getSize();
    auto staging = base_ref.allocator.createBuffer<T>(true_size, vk::BufferUsageFlagBits::eTransferSrc,
                                                      vma::MemoryUsage::eCpuToGpu);

    if (gpuFlag & AssetStorage::GpuRessourceFlagBits::eAfter && buffer) base_ref.copyBuffer(buffer, staging);

    base_ref.allocator.copyBuffer(staging, std::span(data));

    if (gpuFlag & AssetStorage::GpuRessourceFlagBits::eBefore && buffer)
        base_ref.copyBuffer(buffer, staging, 0, data.size() * sizeof(T));

    if (true_size > buffer.getSize()) {
        base_ref.allocator.destroyBuffer(buffer);
        buffer = base_ref.allocator.createBuffer<T>(true_size, flag | vk::BufferUsageFlagBits::eTransferDst,
                                                    vma::MemoryUsage::eGpuOnly);
    }
    base_ref.copyBuffer(staging, buffer);
    base_ref.allocator.destroyBuffer(staging);
}

void AssetStorage::pushModelsOnGPU(GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    if (cpuStorage.vertexStagingBuffer.empty()) {
        logger.warn("Asset Storage") << "No model to push";
        return;
    }

    copy_with_staging_buffer(base_ref->get(), gpuFlag, vk::BufferUsageFlagBits::eVertexBuffer,
                             cpuStorage.vertexStagingBuffer, vertexBuffer);
    copy_with_staging_buffer(base_ref->get(), gpuFlag, vk::BufferUsageFlagBits::eIndexBuffer,
                             cpuStorage.indexStagingBuffer, indicesBuffer);
}

void AssetStorage::pushTexturesOnGPU(GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    if (gpuFlag & GpuRessourceFlagBits::eClear) {
        for (auto &image: textureStorage.getStorage()) {
            base_ref->get().device.destroyImageView(image.imageView);
            base_ref->get().allocator.destroyImage(image);
        }
    }
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
        base_ref->get().transitionLayout(image, vk::ImageLayout::eTransferDstOptimal);
        base_ref->get().copyBufferToImage(stagingBuffer, image);
        base_ref->get().generateMipmaps(image, image.mipLevels);

        base_ref->get().allocator.destroyBuffer(stagingBuffer);
        textureStorage.add(name, std::move(image));
    }
}

void AssetStorage::pushMaterialOnGPU(GpuRessourceFlags gpuFlag)
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
}

void AssetStorage::pushAABBOnGPU(GpuRessourceFlags gpuFlag)
{
    DEBUG_FUNCTION
    if (meshAABBStorage.empty()) {
        logger.warn("Asset Storage") << "No AABB to push";
        return;
    }
    copy_with_staging_buffer(base_ref->get(), gpuFlag, vk::BufferUsageFlagBits::eStorageBuffer,
                             meshAABBStorage.getStorage(), AABBBuffer);
}

}    // namespace pivot::graphics

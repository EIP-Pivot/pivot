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

void AssetStorage::build(DescriptorBuilder builder)
{
    DEBUG_FUNCTION
    assert(cpuStorage.materialStaging.contains(missing_material_name));
    assert(cpuStorage.textureStaging.contains(missing_texture_name));
    createTextureSampler();

    for (const auto &[name, model]: modelStorage) {
        meshBoundingBoxStorage.add(
            name, gpu_object::MeshBoundingBox(std::span(
                      cpuStorage.vertexStagingBuffer.begin() + model.mesh.vertexOffset, model.mesh.vertexSize)));
    }
    assert(modelStorage.size() == meshBoundingBoxStorage.size());

    logger.info("Asset Storage") << prefabStorage.size() << " prefab loaded";
    logger.info("Asset Storage") << "Pushing " << modelStorage.size() << " models onto the GPU";
    pushModelsOnGPU();
    vk_debug::setObjectName(base_ref->get().device, vertexBuffer.buffer, "Vertex Buffer");
    vk_debug::setObjectName(base_ref->get().device, indicesBuffer.buffer, "Indices Buffer");

    logger.info("Asset Storage") << "Pushing " << meshBoundingBoxStorage.size() << " bounding boxes onto the GPU";
    pushBoundingBoxesOnGPU();
    vk_debug::setObjectName(base_ref->get().device, boundingboxBuffer.buffer, "BoundingBox Buffer");

    logger.info("Asset Storage") << "Pushing " << cpuStorage.textureStaging.size() << " textures onto the GPU";
    pushTexturesOnGPU();
    for (const auto &[name, idex]: textureStorage) {
        const auto &im = textureStorage.get(idex);
        vk_debug::setObjectName(base_ref->get().device, im.image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, im.imageView, "Texture " + name + " ImageView");
    }

    logger.info("Asset Storage") << "Pushing " << cpuStorage.materialStaging.size() << " materials onto the GPU";
    pushMaterialOnGPU();
    vk_debug::setObjectName(base_ref->get().device, materialBuffer.buffer, "Material Buffer");

    cpuStorage = {};
    createDescriptorSet(builder);
}

void AssetStorage::destroy()
{
    DEBUG_FUNCTION
    if (vertexBuffer && indicesBuffer) {
        base_ref->get().allocator.destroyBuffer(vertexBuffer);
        base_ref->get().allocator.destroyBuffer(indicesBuffer);
    }
    if (boundingboxBuffer) base_ref->get().allocator.destroyBuffer(boundingboxBuffer);
    if (materialBuffer) base_ref->get().allocator.destroyBuffer(materialBuffer);

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

bool AssetStorage::loadModel(const std::filesystem::path &path)
{
    auto iter = supportedObject.find(path.extension().string());
    if (iter == supportedObject.end()) {
        logger.err("LOAD MODEL") << "Not supported model extension: " << path.extension();
        return false;
    }
    logger.info("Asset Storage") << "Loading model at : " << path;
    return std::apply(iter->second, std::make_tuple(this, path));
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
    return std::apply(iter->second, std::make_tuple(this, path));
}

template <typename T>
static AllocatedBuffer<T> copy_with_staging_buffer(VulkanBase &base_ref, vk::BufferUsageFlags flag,
                                                   std::vector<T> &data)
{
    auto size = data.size() * sizeof(T);
    auto staging =
        base_ref.allocator.createBuffer<T>(size, vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
    auto ret = base_ref.allocator.createBuffer<T>(size, flag | vk::BufferUsageFlagBits::eTransferDst,
                                                  vma::MemoryUsage::eGpuOnly);
    base_ref.allocator.copyBuffer(staging, std::span(data));
    base_ref.copyBuffer(staging, ret);
    base_ref.allocator.destroyBuffer(staging);
    return ret;
}

void AssetStorage::pushModelsOnGPU()
{
    DEBUG_FUNCTION
    if (cpuStorage.vertexStagingBuffer.empty()) {
        logger.warn("Asset Storage") << "No model to push";
        return;
    }

    vertexBuffer = copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eVertexBuffer,
                                            cpuStorage.vertexStagingBuffer);
    indicesBuffer =
        copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eIndexBuffer, cpuStorage.indexStagingBuffer);
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
    materialBuffer = copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer,
                                              materialStorage.getStorage());
}

void AssetStorage::pushBoundingBoxesOnGPU()
{
    DEBUG_FUNCTION
    if (meshBoundingBoxStorage.empty()) {
        logger.warn("Asset Storage") << "No bounding box to push";
        return;
    }
    boundingboxBuffer = copy_with_staging_buffer(base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer,
                                                 meshBoundingBoxStorage.getStorage());
}

}    // namespace pivot::graphics

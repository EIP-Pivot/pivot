#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>

namespace pivot::graphics
{

const std::unordered_map<std::string, bool (AssetStorage::*)(const std::filesystem::path &)>
    AssetStorage::supportedTexture = {
        {".png", &AssetStorage::loadPngTexture},
        {".ktx", &AssetStorage::loadKtxImage},
};

const std::unordered_map<std::string, bool (AssetStorage::*)(const std::filesystem::path &)>
    AssetStorage::supportedObject = {
        {".obj", &AssetStorage::loadObjModel},
        {".gltf", &AssetStorage::loadGltfModel},
};

AssetStorage::AssetStorage(VulkanBase &base): base_ref(base) { cpuStorage.materialStaging.add("white", {}); }

AssetStorage::~AssetStorage() {}

void AssetStorage::build()
{
    DEBUG_FUNCTION
    assert(modelStorage.size() == meshBoundingBoxStorage.size());

    logger.info("Asset Storage") << prefabStorage.size() << " prefab loaded";
    logger.info("Asset Storage") << "Pushing " << modelStorage.size() << " models onto the GPU";
    pushModelsOnGPU();
    vk_debug::setObjectName(base_ref->get().device, vertexBuffer.buffer, "Vertex Buffer");
    vk_debug::setObjectName(base_ref->get().device, indicesBuffer.buffer, "Indices Buffer");

    logger.info("Asset Storage") << "Pushing " << meshBoundingBoxStorage.size() << " bounding boxes onto the GPU";
    pushBoundingBoxesOnGPU();
    vk_debug::setObjectName(base_ref->get().device, boundingboxbuffer.buffer, "BoundingBox Buffer");

    logger.info("Asset Storage") << "Pushing " << cpuStorage.textureStaging.size() << " textures onto the GPU";
    pushTexturesOnGPU();
    for (const auto &[name, idex]: textureStorage) {
        const auto &im = textureStorage.get(idex);
        vk_debug::setObjectName(base_ref->get().device, im.image.image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, im.image.imageView, "Texture " + name + " ImageView");
    }

    logger.info("Asset Storage") << "Pushing " << cpuStorage.materialStaging.size() << " materials onto the GPU";
    pushMaterialOnGPU();
    vk_debug::setObjectName(base_ref->get().device, materialBuffer.buffer, "Material Buffer");

    cpuStorage = {};
}

void AssetStorage::destroy()
{
    DEBUG_FUNCTION
    if (vertexBuffer && indicesBuffer) {
        AllocatedBuffer::destroy(base_ref->get(), vertexBuffer);
        AllocatedBuffer::destroy(base_ref->get(), indicesBuffer);
    }
    if (boundingboxbuffer) AllocatedBuffer::destroy(base_ref->get(), boundingboxbuffer);
    if (materialBuffer) AllocatedBuffer::destroy(base_ref->get(), materialBuffer);
    for (auto &image: textureStorage.getStorage()) { AllocatedImage::destroy(base_ref->get(), image.image); }
}

bool AssetStorage::loadModel(const std::filesystem::path &path)
{
    const auto extension = path.extension();
    auto iter = supportedObject.find(extension.string());
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
    const auto extension = path.extension();
    const auto iter = supportedTexture.find(extension.string().c_str());
    if (iter == supportedTexture.end()) {
        logger.err("Load Texture") << "Not supported texture extension: " << path.extension();
        return false;
    }
    logger.info("Asset Storage") << "Loading texture at : " << path;
    return std::apply(iter->second, std::make_tuple(this, path));
}

void AssetStorage::pushModelsOnGPU()
{
    DEBUG_FUNCTION

    auto vertexSize = cpuStorage.vertexStagingBuffer.size() * sizeof(Vertex);

    if (vertexSize == 0) {
        logger.warn("Asset Storage") << "No model to push";
        return;
    }
    auto stagingVertex = AllocatedBuffer::create(base_ref->get(), vertexSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                 vma::MemoryUsage::eCpuToGpu);
    stagingVertex.copyBuffer(base_ref->get().allocator, cpuStorage.vertexStagingBuffer);
    vertexBuffer = stagingVertex.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), stagingVertex);

    auto indexSize = cpuStorage.indexStagingBuffer.size() * sizeof(uint32_t);
    auto stagingIndex = AllocatedBuffer::create(base_ref->get(), indexSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                vma::MemoryUsage::eCpuToGpu);
    stagingIndex.copyBuffer(base_ref->get().allocator, cpuStorage.indexStagingBuffer);
    indicesBuffer = stagingIndex.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), stagingIndex);

    // clear CPU storage, as it is not needed anymore
    cpuStorage.vertexStagingBuffer.clear();
    cpuStorage.vertexStagingBuffer.shrink_to_fit();
    cpuStorage.indexStagingBuffer.clear();
    cpuStorage.indexStagingBuffer.shrink_to_fit();
}

void AssetStorage::pushTexturesOnGPU()
{
    DEBUG_FUNCTION
    if (cpuStorage.textureStaging.size() == 0) {
        logger.warn("Asset Storage") << "No textures to push";
        return;
    }
    for (const auto &[name, idx]: cpuStorage.textureStaging) {
        const auto &img = cpuStorage.textureStaging.get(idx);
        AllocatedBuffer stagingBuffer = AllocatedBuffer::create(
            base_ref->get(), img.image.size(), vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
        stagingBuffer.copyBuffer(base_ref->get().allocator, img.image);

        AllocatedImage image;
        image.size = img.size;
        image.format = vk::Format::eR8G8B8A8Srgb;
        image.mipLevels = std::floor(std::log2(std::max(img.size.width, img.size.height))) + 1;
        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = image.format,
            .extent = img.size,
            .mipLevels = image.mipLevels,
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
        std::tie(image.image, image.memory) = base_ref->get().allocator.createImage(imageInfo, allocInfo);
        auto createInfo = vk_init::populateVkImageViewCreateInfo(image.image, image.format, image.mipLevels);
        image.imageView = base_ref->get().device.createImageView(createInfo);

        image.transitionLayout(base_ref->get(), vk::ImageLayout::eTransferDstOptimal);
        stagingBuffer.copyToImage(base_ref->get(), image);
        image.generateMipmaps(base_ref->get(), image.mipLevels);

        AllocatedBuffer::destroy(base_ref->get(), stagingBuffer);
        textureStorage.add(name, Texture{
                                     .image = std::move(image),
                                     .size = img.size,
                                 });
    }
}

void AssetStorage::pushMaterialOnGPU()
{
    DEBUG_FUNCTION
    auto size = sizeof(gpu_object::Material) * cpuStorage.materialStaging.size();
    if (size == 0) {
        logger.warn("Asset Storage") << "No material to push";
        return;
    }
    auto materialStaging = AllocatedBuffer::create(
        base_ref->get(), size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vma::MemoryUsage::eCpuToGpu);

    for (auto &[name, idx]: cpuStorage.materialStaging) {
        const auto &mat = cpuStorage.materialStaging.getStorage()[idx];
        materialStorage.add(
            name,
            {
                .metallic = mat.metallic,
                .roughness = mat.roughness,
                .baseColor = mat.baseColor,
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

    materialStaging.copyBuffer(base_ref->get().allocator, materialStorage.getStorage());
    materialBuffer = materialStaging.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), materialStaging);
}

void AssetStorage::pushBoundingBoxesOnGPU()
{
    DEBUG_FUNCTION
    auto size = sizeof(gpu_object::MeshBoundingBox) * meshBoundingBoxStorage.size();
    if (size == 0) {
        logger.warn("Asset Storage") << "No bounding box to push";
        return;
    }
    auto boundingboxStaging = AllocatedBuffer::create(
        base_ref->get(), size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vma::MemoryUsage::eCpuToGpu);

    boundingboxStaging.copyBuffer(base_ref->get().allocator, meshBoundingBoxStorage.getStorage());
    boundingboxbuffer = boundingboxStaging.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), boundingboxStaging);
}

}    // namespace pivot::graphics

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
        {".ktx", &AssetStorage::loadKtxTexture},
};

const std::unordered_map<std::string, bool (AssetStorage::*)(const std::filesystem::path &)>
    AssetStorage::supportedObject = {
        {".obj", &AssetStorage::loadObjModel},
        {".gltf", &AssetStorage::loadGltfModel},
};

AssetStorage::AssetStorage(VulkanBase &base): base_ref(base)
{
    // Initiate a default "white" material
    materialStorage["white"] = {
        .ambientColor = {1.0f, 1.0f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f},
    };
    loadMaterial(tinyobj::material_t{});
}

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

    logger.info("Asset Storage") << "Pushing " << textureStorage.size() << " textures onto the GPU";
    pushTexturesOnGPU();
    for (auto &[name, image]: textureStorage) {
        auto &im = std::get<AllocatedImage>(image.image);
        vk_debug::setObjectName(base_ref->get().device, im.image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, im.imageView, "Texture " + name + "ImageView");
    }

    logger.info("Asset Storage") << "Pushing " << materialStorage.size() << " materials onto the GPU";
    pushMaterialOnGPU();
    vk_debug::setObjectName(base_ref->get().device, materialBuffer.buffer, "Material Buffer");
}

void AssetStorage::destroy()
{
    DEBUG_FUNCTION
    if (vertexBuffer && indicesBuffer) {
        base_ref->get().allocator.destroyBuffer(vertexBuffer.buffer, vertexBuffer.memory);
        base_ref->get().allocator.destroyBuffer(indicesBuffer.buffer, indicesBuffer.memory);
    }
    if (boundingboxbuffer) base_ref->get().allocator.destroyBuffer(boundingboxbuffer.buffer, boundingboxbuffer.memory);
    if (materialBuffer) base_ref->get().allocator.destroyBuffer(materialBuffer.buffer, materialBuffer.memory);
    for (auto &[_, i]: textureStorage) {
        if (std::holds_alternative<AllocatedImage>(i.image)) {
            auto &image = std::get<AllocatedImage>(i.image);
            base_ref->get().device.destroy(image.imageView);
            base_ref->get().allocator.destroyImage(image.image, image.memory);
        }
    }
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

    auto vertexSize = vertexStagingBuffer.size() * sizeof(Vertex);

    if (vertexSize == 0) {
        logger.warn("Asset Storage") << "No model to push";
        return;
    }
    auto stagingVertex = AllocatedBuffer::create(base_ref->get(), vertexSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                 vma::MemoryUsage::eCpuToGpu);
    stagingVertex.copyBuffer(base_ref->get().allocator, vertexStagingBuffer);
    vertexBuffer = stagingVertex.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), stagingVertex);

    auto indexSize = indexStagingBuffer.size() * sizeof(uint32_t);
    auto stagingIndex = AllocatedBuffer::create(base_ref->get(), indexSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                vma::MemoryUsage::eCpuToGpu);
    stagingIndex.copyBuffer(base_ref->get().allocator, indexStagingBuffer);
    indicesBuffer = stagingIndex.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), stagingIndex);

    // clear CPU storage, as it is not needed anymore
    vertexStagingBuffer.clear();
    vertexStagingBuffer.shrink_to_fit();
    indexStagingBuffer.clear();
    indexStagingBuffer.shrink_to_fit();
}

void AssetStorage::pushTexturesOnGPU()
{
    DEBUG_FUNCTION

    if (textureStorage.empty()) {
        logger.warn("Asset Storage") << "No textures to push";
        return;
    }
    for (auto &[name, texture]: textureStorage) {

        auto &bytes = std::get<std::vector<std::byte>>(texture.image);
        AllocatedBuffer stagingBuffer = AllocatedBuffer::create(
            base_ref->get(), bytes.size(), vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
        stagingBuffer.copyBuffer(base_ref->get().allocator, bytes);

        AllocatedImage image;
        image.size = texture.size;
        image.mipLevels = std::floor(std::log2(std::max(texture.size.width, texture.size.height))) + 1;
        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Srgb,
            .extent = texture.size,
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
        auto createInfo =
            vk_init::populateVkImageViewCreateInfo(image.image, vk::Format::eR8G8B8A8Srgb, image.mipLevels);
        image.imageView = base_ref->get().device.createImageView(createInfo);

        image.transitionLayout(base_ref->get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal);
        stagingBuffer.copyToImage(base_ref->get(), image);
        image.generateMipmaps(base_ref->get(), vk::Format::eR8G8B8A8Srgb, image.mipLevels);

        AllocatedBuffer::destroy(base_ref->get(), stagingBuffer);
        texture.image = std::move(image);
    }
}

void AssetStorage::pushMaterialOnGPU()
{
    DEBUG_FUNCTION
    auto size = sizeof(gpuObject::Material) * materialStorage.size();
    if (size == 0) {
        logger.warn("Asset Storage") << "No material to push";
        return;
    }
    auto materialStaging = AllocatedBuffer::create(
        base_ref->get(), size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vma::MemoryUsage::eCpuToGpu);
    std::vector<gpuObject::Material> materialStor;
    std::transform(materialStorage.begin(), materialStorage.end(), std::back_inserter(materialStor),
                   [](const auto &i) { return i.second; });

    materialStaging.copyBuffer(base_ref->get().allocator, materialStor);
    materialBuffer = materialStaging.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), materialStaging);
}

void AssetStorage::pushBoundingBoxesOnGPU()
{
    DEBUG_FUNCTION
    auto size = sizeof(MeshBoundingBox) * materialStorage.size();
    if (size == 0) {
        logger.warn("Asset Storage") << "No material to push";
        return;
    }
    auto boundingboxStaging = AllocatedBuffer::create(
        base_ref->get(), size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vma::MemoryUsage::eCpuToGpu);
    std::vector<MeshBoundingBox> boundingStor;
    std::transform(meshBoundingBoxStorage.begin(), meshBoundingBoxStorage.end(), std::back_inserter(boundingStor),
                   [](const auto &i) { return i.second; });

    boundingboxStaging.copyBuffer(base_ref->get().allocator, boundingStor);
    boundingboxbuffer = boundingboxStaging.cloneBuffer(
        base_ref->get(), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuOnly);
    AllocatedBuffer::destroy(base_ref->get(), boundingboxStaging);
}

}    // namespace pivot::graphics

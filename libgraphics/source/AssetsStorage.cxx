#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <stb_image.h>
#include <tiny_obj_loader.h>

namespace pivot::graphics
{

AssetStorage::AssetStorage(VulkanBase &base): base_ref(base)
{
    // Initiate a default "white" material
    materialStorage["white"] = {
        .ambientColor = {1.0f, 1.0f, 1.0f},
        .diffuse = {1.0f, 1.0f, 1.0f},
        .specular = {1.0f, 1.0f, 1.0f},
    };
}

AssetStorage::~AssetStorage() {}

void AssetStorage::build()
{
    DEBUG_FUNCTION
    logger->info("ASSET STORAGE") << "Pushing models onto the GPU";
    LOGGER_ENDL;
    pushModelsOnGPU();
    vk_debug::setObjectName(base_ref->get().device, vertexBuffer.buffer, "Vertex Buffer");
    vk_debug::setObjectName(base_ref->get().device, indicesBuffer.buffer, "Indices Buffer");

    logger->info("ASSET STORAGE") << "Pushing bounding boxes onto the GPU";
    LOGGER_ENDL;
    pushBoundingBoxesOnGPU();
    vk_debug::setObjectName(base_ref->get().device, boundingboxbuffer.buffer, "BoundingBox Buffer");

    logger->info("ASSET STORAGE") << "Pushing textures onto the GPU";
    LOGGER_ENDL;
    pushTexturesOnGPU();
    for (auto &[name, image]: textureStorage) {
        auto &im = std::get<AllocatedImage>(image.image);
        vk_debug::setObjectName(base_ref->get().device, im.image, "Texture " + name);
        vk_debug::setObjectName(base_ref->get().device, im.imageView, "Texture " + name + "ImageView");
    }

    logger->info("ASSET STORAGE") << "Pushing materials onto the GPU";
    LOGGER_ENDL;
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
    DEBUG_FUNCTION
    if (std::find(supportedObject.begin(), supportedObject.end(), path.extension()) == supportedObject.end()) {
        logger->err("LOAD MODEL") << "Non supported model extension: " << path.extension();
        LOGGER_ENDL;
        return false;
    }
    std::vector<Vertex> currentVertexBuffer;
    std::vector<uint32_t> currentIndexBuffer;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    std::optional<MeshBoundingBox> bounding_box;

    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), nullptr);
    if (!warn.empty()) {
        logger->warn("LOADING_OBJ") << warn;
        LOGGER_ENDL;
    }
    if (!err.empty()) {
        logger->err("LOADING_OBJ") << err;
        LOGGER_ENDL;
        return false;
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    Mesh mesh{
        .vertexOffset = static_cast<uint32_t>(vertexStagingBuffer.size()),
        .indicesOffset = static_cast<uint32_t>(indexStagingBuffer.size()),
    };

    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{
                .pos =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    },
                .color = {1.0f, 1.0f, 1.0f},
            };
            if (!attrib.normals.empty()) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if (!attrib.texcoords.empty()) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = vertexStagingBuffer.size() - mesh.vertexOffset;
                vertexStagingBuffer.push_back(vertex);
            }
            indexStagingBuffer.push_back(uniqueVertices.at(vertex));

            // Update bounding box if point is outside of it
            if (!bounding_box) {
                bounding_box = std::make_optional(MeshBoundingBox(vertex.pos));
            } else {
                bounding_box->addPoint(vertex.pos);
            }
        }
    }
    mesh.indicesSize = indexStagingBuffer.size() - mesh.indicesOffset;
    mesh.vertexSize = vertexStagingBuffer.size() - mesh.vertexOffset;
    meshStorage.insert({path.stem().string(), mesh});
    meshBoundingBoxStorage.insert({path.stem().string(), bounding_box.value()});

    vertexStagingBuffer.insert(vertexStagingBuffer.end(), currentVertexBuffer.begin(), currentVertexBuffer.end());
    indexStagingBuffer.insert(indexStagingBuffer.end(), currentIndexBuffer.begin(), currentIndexBuffer.end());
    return true;
}

bool AssetStorage::loadTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    if (std::find(supportedTexture.begin(), supportedTexture.end(), path.extension()) == supportedTexture.end()) {
        logger->err("LOAD MODEL") << "Non supported texture extension: " << path.extension();
        LOGGER_ENDL;
        return false;
    }

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) return false;

    std::vector<std::byte> image(imageSize);
    std::memcpy(image.data(), pixels, imageSize);
    stbi_image_free(pixels);

    textureStorage.insert({
        path.stem().string(),
        Texture{
            .image = std::move(image),
            .size =
                {
                    .width = static_cast<uint32_t>(texWidth),
                    .height = static_cast<uint32_t>(texHeight),
                    .depth = 1,
                },
        },
    });
    return true;
}

void AssetStorage::pushModelsOnGPU()
{
    DEBUG_FUNCTION

    auto vertexSize = vertexStagingBuffer.size() * sizeof(Vertex);

    if (vertexSize == 0) {
        logger->warn("Asset Storage") << "No model to push";
        LOGGER_ENDL;
        return;
    }
    auto stagingVertex = vk_utils::createBuffer(base_ref->get().allocator, vertexSize,
                                                vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
    vertexBuffer = vk_utils::createBuffer(
        base_ref->get().allocator, vertexSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vma::MemoryUsage::eGpuOnly);
    vk_utils::copyBuffer(base_ref->get().allocator, stagingVertex, vertexStagingBuffer);
    vk_utils::copyBufferToBuffer(*base_ref, stagingVertex.buffer, vertexBuffer.buffer, vertexSize);

    auto indexSize = indexStagingBuffer.size() * sizeof(uint32_t);
    auto stagingIndex = vk_utils::createBuffer(base_ref->get().allocator, indexSize,
                                               vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
    indicesBuffer = vk_utils::createBuffer(
        base_ref->get().allocator, indexSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vma::MemoryUsage::eGpuOnly);
    vk_utils::copyBuffer(base_ref->get().allocator, stagingIndex, indexStagingBuffer);
    vk_utils::copyBufferToBuffer(*base_ref, stagingIndex.buffer, indicesBuffer.buffer, indexSize);

    base_ref->get().allocator.destroyBuffer(stagingVertex.buffer, stagingVertex.memory);
    base_ref->get().allocator.destroyBuffer(stagingIndex.buffer, stagingIndex.memory);

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
        logger->warn("Asset Storage") << "No textures to push";
        LOGGER_ENDL;
        return;
    }
    for (auto &[name, texture]: textureStorage) {

        auto &bytes = std::get<std::vector<std::byte>>(texture.image);
        AllocatedBuffer stagingBuffer =
            vk_utils::createBuffer(base_ref->get().allocator, bytes.size(), vk::BufferUsageFlagBits::eTransferSrc,
                                   vma::MemoryUsage::eCpuToGpu);
        vk_utils::copyBuffer(base_ref->get().allocator, stagingBuffer, bytes);

        uint32_t mipLevels =
            static_cast<uint32_t>(std::floor(std::log2(std::max(texture.size.width, texture.size.height)))) + 1;
        AllocatedImage image;
        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Srgb,
            .extent = texture.size,
            .mipLevels = mipLevels,
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
        auto createInfo = vk_init::populateVkImageViewCreateInfo(image.image, vk::Format::eR8G8B8A8Srgb, mipLevels);
        image.imageView = base_ref->get().device.createImageView(createInfo);

        vk_utils::transitionImageLayout(base_ref->get(), image.image, vk::Format::eR8G8B8A8Srgb,
                                        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        vk_utils::copyBufferToImage(*base_ref, stagingBuffer.buffer, image.image, texture.size);

        base_ref->get().allocator.destroyBuffer(stagingBuffer.buffer, stagingBuffer.memory);
        vk_utils::generateMipmaps(*base_ref, image.image, vk::Format::eR8G8B8A8Srgb, texture.size, mipLevels);
        texture.image = std::move(image);
    }
}

void AssetStorage::pushMaterialOnGPU()
{
    DEBUG_FUNCTION
    auto size = sizeof(gpuObject::Material) * materialStorage.size();
    if (size == 0) {
        logger->warn("Asset Storage") << "No material to push";
        LOGGER_ENDL;
        return;
    }
    auto materialStaging = vk_utils::createBuffer(
        base_ref->get().allocator, size,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
    materialBuffer = vk_utils::createBuffer(
        base_ref->get().allocator, size,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vma::MemoryUsage::eGpuOnly);

    std::vector<gpuObject::Material> materialStor;
    std::transform(materialStorage.begin(), materialStorage.end(), std::back_inserter(materialStor),
                   [](const auto &i) { return i.second; });
    vk_utils::copyBuffer(base_ref->get().allocator, materialStaging, materialStor);
    vk_utils::copyBufferToBuffer(*base_ref, materialStaging.buffer, materialBuffer.buffer, size);
    base_ref->get().allocator.destroyBuffer(materialStaging.buffer, materialStaging.memory);
}

void AssetStorage::pushBoundingBoxesOnGPU()
{
    DEBUG_FUNCTION
    auto size = sizeof(MeshBoundingBox) * materialStorage.size();
    if (size == 0) {
        logger->warn("Asset Storage") << "No material to push";
        LOGGER_ENDL;
        return;
    }
    auto boundingboxStaging = vk_utils::createBuffer(
        base_ref->get().allocator, size,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
    boundingboxbuffer = vk_utils::createBuffer(
        base_ref->get().allocator, size,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vma::MemoryUsage::eGpuOnly);

    std::vector<MeshBoundingBox> boundingStor;
    std::transform(meshBoundingBoxStorage.begin(), meshBoundingBoxStorage.end(), std::back_inserter(boundingStor),
                   [](const auto &i) { return i.second; });
    vk_utils::copyBuffer(base_ref->get().allocator, boundingboxStaging, boundingStor);
    vk_utils::copyBufferToBuffer(*base_ref, boundingboxStaging.buffer, boundingboxbuffer.buffer, size);
    base_ref->get().allocator.destroyBuffer(boundingboxStaging.buffer, boundingboxStaging.memory);
}

}    // namespace pivot::graphics

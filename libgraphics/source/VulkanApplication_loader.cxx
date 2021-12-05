#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <stb_image.h>
#include <tiny_obj_loader.h>

namespace pivot::graphics
{
size_t VulkanApplication::loadTextures(const std::vector<std::filesystem::path> &textures)
{
    DEBUG_FUNCTION
    auto bar = logger->newProgressBar("Textures", textures.size());
    for (const auto &f: textures) {
        ++bar;
        logger->info("LOADING") << "Loading texture: " << f;
        LOGGER_ENDL;

        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(f.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) throw std::runtime_error("failed to load texture image");

        std::vector<std::byte> image(imageSize);
        std::memcpy(image.data(), pixels, imageSize);
        cpuStorage.loadedTexturesSize.insert({
            f.stem().string(),
            {
                .width = static_cast<uint32_t>(texWidth),
                .height = static_cast<uint32_t>(texHeight),
                .depth = 1,
            },
        });
        cpuStorage.loadedTextures.insert({f.stem().string(), std::move(image)});
    }
    logger->deleteProgressBar(bar);
    return cpuStorage.loadedTextures.size();
}

size_t VulkanApplication::load3DModels(const std::vector<std::filesystem::path> &models)
{
    DEBUG_FUNCTION
    std::vector<Vertex> vertexStagingBuffer;
    std::vector<uint32_t> indexStagingBuffer;
    auto bar = logger->newProgressBar("Models", models.size());
    for (const auto &f: models) {
        logger->info("LOADING") << "Loading object: " << f;
        LOGGER_ENDL;

        GPUMesh mesh{
            .verticiesOffset = vertexStagingBuffer.size(),
            .indicesOffset = indexStagingBuffer.size(),
        };
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        std::optional<MeshBoundingBox> bounding_box;

        tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, f.string().c_str(), nullptr);
        if (!warn.empty()) {
            logger->warn("LOADING_OBJ") << warn;
            LOGGER_ENDL;
        }
        if (!err.empty()) {
            logger->err("LOADING_OBJ") << err;
            LOGGER_ENDL;
            throw std::runtime_error("Error while loading obj file");
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

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
                    uniqueVertices[vertex] = vertexStagingBuffer.size() - mesh.verticiesOffset;
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
        mesh.verticiesSize = vertexStagingBuffer.size() - mesh.verticiesOffset;
        loadedMeshes[f.stem().string()] = mesh;
        meshesBoundingBoxes.insert({f.stem().string(), bounding_box.value()});
    }
    cpuStorage.vertexBuffer.swap(vertexStagingBuffer);
    cpuStorage.indexBuffer.swap(indexStagingBuffer);
    logger->deleteProgressBar(bar);
    return loadedMeshes.size();
}

void VulkanApplication::pushModelsToGPU()
{
    DEBUG_FUNCTION
    logger->info("GPU") << "Loading Models onto the GPU";
    LOGGER_ENDL;
    auto vertexSize = cpuStorage.vertexBuffer.size() * sizeof(Vertex);
    auto stagingVertex = vk_utils::createBuffer(allocator, vertexSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                vma::MemoryUsage::eCpuToGpu);
    vertexBuffers = vk_utils::createBuffer(
        allocator, vertexSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vma::MemoryUsage::eGpuOnly);
    vk_utils::copyBuffer(allocator, stagingVertex, cpuStorage.vertexBuffer);
    copyBufferToBuffer(stagingVertex.buffer, vertexBuffers.buffer, vertexSize);

    auto indexSize = cpuStorage.indexBuffer.size() * sizeof(uint32_t);
    auto stagingIndex = vk_utils::createBuffer(allocator, indexSize, vk::BufferUsageFlagBits::eTransferSrc,
                                               vma::MemoryUsage::eCpuToGpu);
    indicesBuffers = vk_utils::createBuffer(
        allocator, indexSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vma::MemoryUsage::eGpuOnly);
    vk_utils::copyBuffer(allocator, stagingIndex, cpuStorage.indexBuffer);
    copyBufferToBuffer(stagingIndex.buffer, indicesBuffers.buffer, indexSize);

    allocator.destroyBuffer(stagingVertex.buffer, stagingVertex.memory);
    allocator.destroyBuffer(stagingIndex.buffer, stagingIndex.memory);

    // clear CPU storage, as it is not needed anymore
    cpuStorage.vertexBuffer.clear();
    cpuStorage.vertexBuffer.shrink_to_fit();
    cpuStorage.indexBuffer.clear();
    cpuStorage.indexBuffer.shrink_to_fit();
    mainDeletionQueue.push([&] {
        allocator.destroyBuffer(vertexBuffers.buffer, vertexBuffers.memory);
        allocator.destroyBuffer(indicesBuffers.buffer, indicesBuffers.memory);
    });
}

void VulkanApplication::pushTexturesToGPU()
{
    DEBUG_FUNCTION
    logger->info("GPU") << "Loading Textures onto the GPU";
    LOGGER_ENDL;
    auto bar = logger->newProgressBar("GPU Textures", cpuStorage.loadedTextures.size());
    for (const auto &[name, texture]: cpuStorage.loadedTextures) {

        AllocatedBuffer stagingBuffer = vk_utils::createBuffer(
            allocator, texture.size(), vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
        vk_utils::copyBuffer(allocator, stagingBuffer, texture);

        mipLevels =
            static_cast<uint32_t>(std::floor(std::log2(std::max(cpuStorage.loadedTexturesSize.at(name).width,
                                                                cpuStorage.loadedTexturesSize.at(name).height)))) +
            1;
        AllocatedImage image;
        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Srgb,
            .extent = cpuStorage.loadedTexturesSize.at(name),
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
        std::tie(image.image, image.memory) = allocator.createImage(imageInfo, allocInfo);
        auto createInfo = vk_init::populateVkImageViewCreateInfo(image.image, vk::Format::eR8G8B8A8Srgb, mipLevels);
        image.imageView = device.createImageView(createInfo);

        transitionImageLayout(image.image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(stagingBuffer.buffer, image.image, cpuStorage.loadedTexturesSize.at(name));
        allocator.destroyBuffer(stagingBuffer.buffer, stagingBuffer.memory);
        generateMipmaps(image.image, vk::Format::eR8G8B8A8Srgb, cpuStorage.loadedTexturesSize.at(name), mipLevels);
        loadedTextures.insert({name, std::move(image)});
        ++bar;
    }
    mainDeletionQueue.push([&] {
        for (auto &[_, i]: loadedTextures) {
            device.destroy(i.imageView);
            allocator.destroyImage(i.image, i.memory);
        }
    });
    logger->deleteProgressBar(bar);
    cpuStorage.loadedTextures.clear();
    cpuStorage.loadedTexturesSize.clear();
}

}    // namespace pivot::graphics
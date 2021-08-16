#include "pivot/graphics/VulkanApplication.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <stb_image.h>
#include <tiny_obj_loader.h>

size_t VulkanApplication::loadTexturess(const std::vector<std::filesystem::path> &textures)
{
    auto &bar = logger->newProgressBar("Textures", textures.size());
    for (const auto &f: textures) {
        ++bar;
        logger->info("LOADING") << "Loading texture: " << f;
        LOGGER_ENDL;

        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(f.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) throw std::runtime_error("failed to load texture image");

        std::vector<std::byte> image(imageSize);
        std::memcpy(image.data(), pixels, imageSize);
        cpuStorage.loadedTexturesSize.insert({
            f.stem(),
            {
                .width = static_cast<uint32_t>(texWidth),
                .height = static_cast<uint32_t>(texHeight),
                .depth = 1,
            },
        });
        cpuStorage.loadedTextures.insert({f.stem(), std::move(image)});
    }
    logger->deleteProgressBar(bar);
    return cpuStorage.loadedTextures.size();
}

size_t VulkanApplication::load3DModels(const std::vector<std::filesystem::path> &models)
{
    std::vector<Vertex> vertexStagingBuffer;
    std::vector<uint32_t> indexStagingBuffer;
    auto &bar = logger->newProgressBar("Models", models.size());
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

        tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, f.c_str(), nullptr);
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
            }
        }
        mesh.indicesSize = indexStagingBuffer.size() - mesh.indicesOffset;
        mesh.verticiesSize = vertexStagingBuffer.size() - mesh.verticiesOffset;
        loadedMeshes[f.stem()] = mesh;
    }
    cpuStorage.vertexBuffer.swap(vertexStagingBuffer);
    cpuStorage.indexBuffer.swap(indexStagingBuffer);
    logger->deleteProgressBar(bar);
    return loadedMeshes.size();
}

void VulkanApplication::pushModelsToGPU()
{
    logger->info("GPU") << "Loading Models onto the GPU";
    LOGGER_ENDL;
    auto vertexSize = cpuStorage.vertexBuffer.size() * sizeof(Vertex);
    auto stagingVertex = createBuffer(vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    vertexBuffers = createBuffer(vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                 VMA_MEMORY_USAGE_GPU_ONLY);
    copyBuffer(stagingVertex, cpuStorage.vertexBuffer);
    copyBuffer(stagingVertex.buffer, vertexBuffers.buffer, vertexSize);

    auto indexSize = cpuStorage.indexBuffer.size() * sizeof(uint32_t);
    auto stagingIndex = createBuffer(indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    indicesBuffers = createBuffer(indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                  VMA_MEMORY_USAGE_GPU_ONLY);
    copyBuffer(stagingIndex, cpuStorage.indexBuffer);
    copyBuffer(stagingIndex.buffer, indicesBuffers.buffer, indexSize);

    vmaDestroyBuffer(allocator, stagingVertex.buffer, stagingVertex.memory);
    vmaDestroyBuffer(allocator, stagingIndex.buffer, stagingIndex.memory);

    // clear CPU storage, as it is not needed anymore
    cpuStorage.vertexBuffer.clear();
    cpuStorage.vertexBuffer.shrink_to_fit();
    cpuStorage.indexBuffer.clear();
    cpuStorage.indexBuffer.shrink_to_fit();
    mainDeletionQueue.push([=] {
        vmaDestroyBuffer(allocator, vertexBuffers.buffer, vertexBuffers.memory);
        vmaDestroyBuffer(allocator, indicesBuffers.buffer, indicesBuffers.memory);
    });
}

void VulkanApplication::pushTexturesToGPU()
{
    auto &bar = logger->newProgressBar("GPU Textures", cpuStorage.loadedTextures.size());
    for (const auto &[name, texture]: cpuStorage.loadedTextures) {
        logger->info("GPU") << "Loading Textures onto the GPU";
        LOGGER_ENDL;

        AllocatedBuffer stagingBuffer =
            createBuffer(texture.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        copyBuffer(stagingBuffer, texture);

        mipLevels =
            static_cast<uint32_t>(std::floor(std::log2(std::max(cpuStorage.loadedTexturesSize.at(name).width,
                                                                cpuStorage.loadedTexturesSize.at(name).height)))) +
            1;
        AllocatedImage image{};
        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .extent = cpuStorage.loadedTexturesSize.at(name),
            .mipLevels = mipLevels,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        VmaAllocationCreateInfo allocInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        };
        VK_TRY(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image.image, &image.memory, nullptr));
        auto createInfo = vk_init::populateVkImageViewCreateInfo(image.image, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VK_TRY(vkCreateImageView(device, &createInfo, nullptr, &image.imageView));

        transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        copyBufferToImage(stagingBuffer.buffer, image.image, cpuStorage.loadedTexturesSize.at(name));
        vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.memory);
        generateMipmaps(image.image, VK_FORMAT_R8G8B8A8_SRGB, cpuStorage.loadedTexturesSize.at(name), mipLevels);
        loadedTextures.insert({name, std::move(image)});
        ++bar;
    }
    mainDeletionQueue.push([&] {
        for (auto &[_, i]: loadedTextures) {
            vkDestroyImageView(device, i.imageView, nullptr);
            vmaDestroyImage(allocator, i.image, i.memory);
        }
    });
    logger->deleteProgressBar(bar);
    cpuStorage.loadedTextures.clear();
    cpuStorage.loadedTexturesSize.clear();
}

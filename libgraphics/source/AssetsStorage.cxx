#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <Logger.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

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
    DEBUG_FUNCTION
    if (std::find(supportedObject.begin(), supportedObject.end(), path.extension()) == supportedObject.end()) {
        logger.err("LOAD MODEL") << "Non supported model extension: " << path.extension();

        return false;
    }
    logger.info("Asset Storage") << "Loading model at : " << path;
    auto base_dir = path.parent_path();
    std::vector<Vertex> currentVertexBuffer;
    std::vector<uint32_t> currentIndexBuffer;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    /// TODO: check return value
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), base_dir.string().c_str(), false,
                     false);
    if (!warn.empty()) { logger.warn("LOADING_OBJ") << warn; }
    if (!err.empty()) {
        logger.err("LOADING_OBJ") << err;
        return false;
    }

    for (const auto &m: materials) {
        loadMaterial(m);
        if (!m.diffuse_texname.empty() && !getTextures().contains(m.diffuse_texname)) {
            loadTexture(base_dir / m.diffuse_texname);
        }
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    Prefab prefab;
    for (const auto &shape: shapes) {
        Model model{
            .mesh =
                {
                    .vertexOffset = static_cast<uint32_t>(vertexStagingBuffer.size()),
                    .indicesOffset = static_cast<uint32_t>(indexStagingBuffer.size()),
                },
        };
        if (!shape.mesh.material_ids.empty() && shape.mesh.material_ids.at(0) >= 0) {
            model.default_material = materials.at(shape.mesh.material_ids.at(0)).name;
            if (std::filesystem::path name = materials.at(shape.mesh.material_ids.at(0)).diffuse_texname;
                !name.empty()) {
                model.default_texture = name.stem().string();
            } else if (!prefab.modelIds.empty()) {
                model.default_texture = get<Model>(prefab.modelIds.at(0)).default_texture;
            }
        }
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
                uniqueVertices[vertex] = vertexStagingBuffer.size() - model.mesh.vertexOffset;
                vertexStagingBuffer.push_back(vertex);
            }
            indexStagingBuffer.push_back(uniqueVertices.at(vertex));
        }
        model.mesh.indicesSize = indexStagingBuffer.size() - model.mesh.indicesOffset;
        model.mesh.vertexSize = vertexStagingBuffer.size() - model.mesh.vertexOffset;
        prefab.modelIds.push_back(shape.name);
        modelStorage.insert({shape.name, model});
        meshBoundingBoxStorage.insert(
            {shape.name,
             MeshBoundingBox(std::span(vertexStagingBuffer.begin() + model.mesh.vertexOffset, model.mesh.vertexSize))});
    }
    prefabStorage.insert({path.stem().string(), prefab});
    vertexStagingBuffer.insert(vertexStagingBuffer.end(), currentVertexBuffer.begin(), currentVertexBuffer.end());
    indexStagingBuffer.insert(indexStagingBuffer.end(), currentIndexBuffer.begin(), currentIndexBuffer.end());
    return true;
}

bool AssetStorage::loadMaterial(const tinyobj::material_t &material)
{
    gpuObject::Material mat{
        .shininess = material.shininess,
        .ambientColor = glm::make_vec3(material.ambient),
        .diffuse = glm::make_vec3(material.diffuse),
        .specular = glm::make_vec3(material.specular),
    };
    materialStorage.insert({material.name, mat});
    return true;
}

bool AssetStorage::loadTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    if (std::find(supportedTexture.begin(), supportedTexture.end(), path.extension()) == supportedTexture.end()) {
        logger.err("Load model") << "Non supported texture extension: " << path.extension();

        return false;
    }
    logger.info("Asset Storage") << "Loading texture at : " << path;

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) return false;

    std::vector<std::byte> image(imageSize);
    std::memcpy(image.data(), pixels, imageSize);
    stbi_image_free(pixels);

    textureStorage.insert(std::make_pair(path.stem().string(), Texture{
                                                                   .image = std::move(image),
                                                                   .size =
                                                                       {
                                                                           .width = static_cast<uint32_t>(texWidth),
                                                                           .height = static_cast<uint32_t>(texHeight),
                                                                           .depth = 1,
                                                                       },
                                                               }));
    return true;
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
    vertexBuffer = AllocatedBuffer::create(
        base_ref->get(), vertexSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vma::MemoryUsage::eGpuOnly);
    vk_utils::copyBuffer(base_ref->get().allocator, stagingVertex, vertexStagingBuffer);
    vk_utils::copyBufferToBuffer(*base_ref, stagingVertex.buffer, vertexBuffer.buffer, vertexSize);

    auto indexSize = indexStagingBuffer.size() * sizeof(uint32_t);
    auto stagingIndex = AllocatedBuffer::create(base_ref->get(), indexSize, vk::BufferUsageFlagBits::eTransferSrc,
                                                vma::MemoryUsage::eCpuToGpu);
    indicesBuffer = AllocatedBuffer::create(
        base_ref->get(), indexSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vma::MemoryUsage::eGpuOnly);
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
        logger.warn("Asset Storage") << "No textures to push";
        return;
    }
    for (auto &[name, texture]: textureStorage) {

        auto &bytes = std::get<std::vector<std::byte>>(texture.image);
        AllocatedBuffer stagingBuffer = AllocatedBuffer::create(
            base_ref->get(), bytes.size(), vk::BufferUsageFlagBits::eTransferSrc, vma::MemoryUsage::eCpuToGpu);
        vk_utils::copyBuffer(base_ref->get().allocator, stagingBuffer, bytes);

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
        vk_utils::copyBufferToImage(*base_ref, stagingBuffer.buffer, image.image, texture.size);
        image.generateMipmaps(base_ref->get(), vk::Format::eR8G8B8A8Srgb, image.mipLevels);

        base_ref->get().allocator.destroyBuffer(stagingBuffer.buffer, stagingBuffer.memory);
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
    materialBuffer = AllocatedBuffer::create(
        base_ref->get(), size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuOnly);

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
        logger.warn("Asset Storage") << "No material to push";
        return;
    }
    auto boundingboxStaging = AllocatedBuffer::create(
        base_ref->get(), size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vma::MemoryUsage::eCpuToGpu);
    boundingboxbuffer = AllocatedBuffer::create(
        base_ref->get(), size, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuOnly);

    std::vector<MeshBoundingBox> boundingStor;
    std::transform(meshBoundingBoxStorage.begin(), meshBoundingBoxStorage.end(), std::back_inserter(boundingStor),
                   [](const auto &i) { return i.second; });
    vk_utils::copyBuffer(base_ref->get().allocator, boundingboxStaging, boundingStor);
    vk_utils::copyBufferToBuffer(*base_ref, boundingboxStaging.buffer, boundingboxbuffer.buffer, size);
    base_ref->get().allocator.destroyBuffer(boundingboxStaging.buffer, boundingboxStaging.memory);
}

}    // namespace pivot::graphics

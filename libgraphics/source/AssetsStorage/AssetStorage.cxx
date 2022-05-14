#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <type_traits>

#include <Logger.hpp>

namespace pivot::graphics
{

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

static const std::vector<Vertex> quad_vertices = {
    {
        .pos = {-0.5f, -0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .color = {1.0f, 0.0f, 0.0f},
        .texCoord = {-0.5f, -0.5f},
    },
    {
        .pos = {0.5f, -0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .color = {0.0f, 1.0f, 0.0f},
        .texCoord = {0.5f, -0.5f},
    },
    {
        .pos = {0.5f, 0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .color = {0.0f, 0.0f, 1.0f},
        .texCoord = {0.5f, 0.5f},
    },
    {
        .pos = {-0.5f, 0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .color = {1.0f, 1.0f, 1.0f},
        .texCoord = {-0.5f, 0.5f},
    },
};
static const std::vector<std::uint32_t> quad_indices = {0, 1, 2, 2, 3, 0};

AssetStorage::CPUStorage::CPUStorage()
{
    textureStaging.add(missing_texture_name, default_texture_data);
    materialStaging.add(missing_material_name, {
                                                   .baseColorTexture = missing_material_name,
                                               });

    materialStaging.add("white", {});

    vertexStagingBuffer.insert(vertexStagingBuffer.end(), quad_vertices.begin(), quad_vertices.end());
    indexStagingBuffer.insert(indexStagingBuffer.end(), quad_indices.begin(), quad_indices.end());
    modelStorage[quad_mesh] = {
        .mesh =
            {
                .vertexOffset = 0,
                .vertexSize = static_cast<uint32_t>(quad_vertices.size()),
                .indicesOffset = 0,
                .indicesSize = static_cast<uint32_t>(quad_indices.size()),
            },
        .default_material = std::nullopt,
    };
}

AssetStorage::AssetStorage(VulkanBase &base): base_ref(base) {}

AssetStorage::~AssetStorage() {}

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

bool AssetStorage::addAsset(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    auto iterModel = loaders::supportedObject.find(path.extension().string());
    if (iterModel != loaders::supportedObject.end()) { return addModel(path); }

    auto iterTexture = loaders::supportedTexture.find(path.extension().string());
    if (iterTexture != loaders::supportedTexture.end()) { return addTexture(path); }

    logger.err("AssetStorage/addAsset") << "Not supported asset extension: " << path.extension();
    return false;
}

bool AssetStorage::addAsset(const std::vector<std::filesystem::path> &path)
{
    DEBUG_FUNCTION
    unsigned load = 0;
    for (const auto &i: path) load += addAsset(i);
    return load == path.size();
}

bool AssetStorage::addModel(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    auto iter = loaders::supportedObject.find(path.extension().string());
    if (iter == loaders::supportedObject.end()) {
        logger.err("AssetStorage/addModel") << "Not supported model extension: " << path.extension();
        return false;
    }
    cpuStorage.modelPaths[path.stem().string()] = asset_dir / path;
    return true;
}

bool AssetStorage::addModel(const std::vector<std::filesystem::path> &path)
{
    DEBUG_FUNCTION
    unsigned load = 0;
    for (const auto &i: path) load += addModel(i);
    return load == path.size();
}

bool AssetStorage::addTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    const auto iter = loaders::supportedTexture.find(path.extension().string());
    if (iter == loaders::supportedTexture.end()) {
        logger.err("Load Texture") << "Not supported texture extension: " << path.extension();
        return false;
    }
    cpuStorage.texturePaths[path.stem().string()] = asset_dir / path;
    return true;
}
bool AssetStorage::addTexture(const std::vector<std::filesystem::path> &path)
{
    DEBUG_FUNCTION
    unsigned load = 0;
    for (const auto &i: path) load += addTexture(i);
    return load == path.size();
}

bool AssetStorage::loadModel(const std::filesystem::path &path)
{
    auto extension = path.extension().string();
    logger.debug("Asset Storage") << "Loading model at : " << path;
    return std::apply(loaders::supportedObject.at(extension), std::make_tuple(path, std::ref(cpuStorage)));
}

bool AssetStorage::loadTexture(const std::filesystem::path &path)
{
    auto extension = path.extension().string();
    logger.debug("Asset Storage") << "Loading texture at : " << path;
    return std::apply(loaders::supportedTexture.at(extension), std::make_tuple(path, std::ref(cpuStorage)));
}

}    // namespace pivot::graphics

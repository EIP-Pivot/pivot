#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <type_traits>

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
    auto iterModel = supportedObject.find(path.extension().string());
    if (iterModel != supportedObject.end()) { return addModel(path); }

    auto iterTexture = supportedTexture.find(path.extension().string());
    if (iterTexture != supportedTexture.end()) { return addTexture(path); }

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
    auto iter = supportedObject.find(path.extension().string());
    if (iter == supportedObject.end()) {
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
    const auto iter = supportedTexture.find(path.extension().string());
    if (iter == supportedTexture.end()) {
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
    assert(supportedObject.contains(extension));
    logger.debug("Asset Storage") << "Loading model at : " << path;
    return std::apply(supportedObject.at(extension), std::make_tuple(this, path));
}

bool AssetStorage::loadTexture(const std::filesystem::path &path)
{
    auto extension = path.extension().string();
    assert(supportedTexture.contains(extension));
    logger.debug("Asset Storage") << "Loading texture at : " << path;
    return std::apply(supportedTexture.at(extension), std::make_tuple(this, path));
}

}    // namespace pivot::graphics

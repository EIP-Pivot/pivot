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

bool AssetStorage::loadAsset(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    auto iterModel = supportedObject.find(path.extension().string());
    if (iterModel != supportedObject.end()) { return loadModel(path); }

    auto iterTexture = supportedTexture.find(path.extension().string());
    if (iterTexture != supportedTexture.end()) { return loadTexture(path); }

    logger.err("LOAD ASSET") << "Not supported asset extension: " << path.extension();
    return false;
}

bool AssetStorage::loadModel(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
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

}    // namespace pivot::graphics

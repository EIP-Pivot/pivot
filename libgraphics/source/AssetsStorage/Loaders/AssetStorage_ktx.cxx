#include "pivot/graphics/AssetStorage/Loaders.hxx"

#include <ktx.h>

namespace pivot::graphics::asset::loaders
{

std::optional<asset::CPUStorage> loadKtxImage(const std::filesystem::path &path)
try {
    asset::CPUStorage storage;
    asset::CPUTexture texture;
    ktxTexture *ktxTexture = nullptr;
    auto result =
        ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

    if (result != KTX_SUCCESS) {
        logger.err("AssetStorage/KTX") << ktxErrorString(result);
        return std::nullopt;
    }
    ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
    ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);
    texture.image.resize(ktxTextureSize);
    std::memcpy(texture.image.data(), ktxTextureData, ktxTextureSize);
    texture.size = vk::Extent3D{
        .width = ktxTexture->baseWidth,
        .height = ktxTexture->baseHeight,
        .depth = ktxTexture->baseDepth,
    };
    ktxTexture_Destroy(ktxTexture);
    storage.textureStaging.insert(path.stem().string(), std::move(texture));
    return storage;
} catch (const std::runtime_error &re) {
    logger.err("Asset Storage/KTX") << re.what();
    return std::nullopt;
}

}    // namespace pivot::graphics::loaders

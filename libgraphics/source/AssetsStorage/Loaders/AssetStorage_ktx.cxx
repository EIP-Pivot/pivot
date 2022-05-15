#include "pivot/graphics/AssetStorage.hxx"

#include <ktx.h>

namespace pivot::graphics::loaders
{

bool loadKtxImage(const std::filesystem::path &path, AssetStorage::CPUStorage &storage)
try {
    AssetStorage::CPUTexture texture;
    ktxTexture *ktxTexture = nullptr;
    auto result =
        ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

    if (result != KTX_SUCCESS) {
        logger.err("AssetStorage/KTX") << ktxErrorString(result);
        return false;
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
    storage.textureStaging.add(path.stem().string(), std::move(texture));
    return true;
} catch (const std::runtime_error &re) {
    logger.err("Asset Storage/KTX") << re.what();
    return false;
}

}    // namespace pivot::graphics::loaders

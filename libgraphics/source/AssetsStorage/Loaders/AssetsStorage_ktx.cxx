#include "pivot/graphics/AssetStorage.hxx"

#include <ktx.h>

namespace pivot::graphics
{

bool AssetStorage::loadKtxImage(const std::filesystem::path &path)
{
    CPUTexture texture;
    ktxTexture *ktxTexture = nullptr;
    auto result =
        ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
    if (result != KTX_SUCCESS) throw AssetStorage::AssetStorageException("Failed to load ktx texture");
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
    cpuStorage.textureStaging.add(path.stem().string(), std::move(texture));
    return true;
}

}    // namespace pivot::graphics
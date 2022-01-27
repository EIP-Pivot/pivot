#include "pivot/graphics/AssetStorage.hxx"

#include "pivot/graphics/DebugMacros.hxx"

#include <stb_image.h>

namespace pivot::graphics
{

bool AssetStorage::loadPngTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) return false;

    std::vector<std::byte> image(imageSize);
    std::memcpy(image.data(), pixels, imageSize);
    stbi_image_free(pixels);

    cpuStorage.textureStaging.add(path.stem().string(), CPUTexture{
                                                            .image = std::move(image),
                                                            .size =
                                                                {
                                                                    .width = static_cast<uint32_t>(texWidth),
                                                                    .height = static_cast<uint32_t>(texHeight),
                                                                    .depth = 1,
                                                                },
                                                        });
    return true;
}

}    // namespace pivot::graphics
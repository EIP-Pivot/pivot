#include "pivot/graphics/AssetStorage/Loaders.hxx"

#include "pivot/pivot.hxx"

#include <stb_image.h>

namespace pivot::graphics::asset::loaders
{

std::optional<asset::CPUStorage> loadPngTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    asset::CPUStorage storage;
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        logger.err("Asset Storage") << "stbi_load() returned NULL";
        return std::nullopt;
    }

    std::vector<std::byte> image(imageSize);
    std::memcpy(image.data(), pixels, imageSize);
    stbi_image_free(pixels);

    storage.textureStaging.insert(path.stem().string(), {
                                                            .image = std::move(image),
                                                            .size =
                                                                {
                                                                    .width = static_cast<uint32_t>(texWidth),
                                                                    .height = static_cast<uint32_t>(texHeight),
                                                                    .depth = 1,
                                                                },
                                                        });
    return storage;
}

}    // namespace pivot::graphics::loaders

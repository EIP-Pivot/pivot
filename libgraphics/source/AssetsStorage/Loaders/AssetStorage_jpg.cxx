#include "pivot/graphics/AssetStorage/Loaders.hxx"

#include "pivot/pivot.hxx"

namespace pivot::graphics::asset::loaders
{

std::optional<asset::CPUStorage> loadJpgTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    return loadPngTexture(path);
}

}    // namespace pivot::graphics::asset::loaders

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics::loaders
{

std::optional<asset::CPUStorage> loadJpgTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    return loadPngTexture(path);
}

}    // namespace pivot::graphics::loaders

#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DebugMacros.hxx"

namespace pivot::graphics::loaders
{

std::optional<AssetStorage::CPUStorage> loadJpgTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    return loadPngTexture(path);
}

}    // namespace pivot::graphics::loaders

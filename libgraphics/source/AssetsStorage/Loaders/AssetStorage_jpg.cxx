#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DebugMacros.hxx"

namespace pivot::graphics::loaders
{

bool loadJpgTexture(const std::filesystem::path &path, AssetStorage::CPUStorage &storage)
{
    DEBUG_FUNCTION
    return loadPngTexture(path, storage);
}

}    // namespace pivot::graphics::loaders

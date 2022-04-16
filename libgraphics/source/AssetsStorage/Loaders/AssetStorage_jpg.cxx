#include "pivot/graphics/AssetStorage.hxx"
#include "pivot/graphics/DebugMacros.hxx"

namespace pivot::graphics
{

bool AssetStorage::loadJpgTexture(const std::filesystem::path &path)
{
    DEBUG_FUNCTION
    return loadPngTexture(path);
}

}    // namespace pivot::graphics

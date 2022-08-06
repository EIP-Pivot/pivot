#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include "pivot/graphics/AssetStorage/CPUStorage.hxx"

namespace pivot::graphics::asset::loaders
{

/// @brief The function signature of an asset handler
using AssetHandler = std::function<std::optional<asset::CPUStorage>(const std::filesystem::path &)>;

LOGIC_ERROR(AssetHandler);

/// Load a .obj file
std::optional<asset::CPUStorage> loadObjModel(const std::filesystem::path &path);
/// Load a .gltf file
std::optional<asset::CPUStorage> loadGltfModel(const std::filesystem::path &path);

/// Load a .png file
std::optional<asset::CPUStorage> loadPngTexture(const std::filesystem::path &path);
/// Load a .jpg file
std::optional<asset::CPUStorage> loadJpgTexture(const std::filesystem::path &path);
/// Load a .ktx file
std::optional<asset::CPUStorage> loadKtxImage(const std::filesystem::path &path);

/// List of supported texture extensions
const std::unordered_map<std::string, loaders::AssetHandler> supportedTexture = {
    {".png", &loadPngTexture},
    {".jpg", &loadJpgTexture},
    {".ktx", &loadKtxImage},
};

/// List of supported object extensions
const std::unordered_map<std::string, loaders::AssetHandler> supportedObject = {
    {".obj", &loadObjModel},
    {".gltf", &loadGltfModel},
};

}    // namespace pivot::graphics::asset::loaders

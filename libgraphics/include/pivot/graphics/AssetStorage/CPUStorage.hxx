#pragma once

#include "pivot/containers/IndexedStorage.hxx"
#include "pivot/exception.hxx"
#include "pivot/graphics/AssetStorage/CPUMaterial.hxx"
#include "pivot/graphics/AssetStorage/CPUTexture.hxx"
#include "pivot/graphics/AssetStorage/Model.hxx"
#include "pivot/graphics/AssetStorage/Prefab.hxx"
#include "pivot/graphics/types/Vertex.hxx"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace pivot::graphics::asset
{

/// Represent the loaded assets before being uploaded to the GPU
class CPUStorage
{
public:
    /// CPUStorage error type
    LOGIC_ERROR(CPUStorage);

    /// Merge two CPUStorage together
    void merge(const CPUStorage &other);

    /// Operator += overload
    CPUStorage &operator+=(const CPUStorage &other);
    /// Equality operator
    bool operator==(const CPUStorage &) const = default;

public:
    /// Return a CPUStorage with default asset
    static CPUStorage default_assets();

public:
    /// store the Models
    std::unordered_map<std::string, Model> modelStorage;
    /// store the Prefab
    std::unordered_map<std::string, Prefab> prefabStorage;
    /// Store the Vertex
    std::vector<Vertex> vertexStagingBuffer;
    /// Store the index of the buffer
    std::vector<std::uint32_t> indexStagingBuffer;
    /// Store the textures
    IndexedStorage<std::string, CPUTexture> textureStaging;
    /// Store the Materials
    IndexedStorage<std::string, CPUMaterial> materialStaging;
    /// Store the path of the model currently loaded
    std::unordered_map<std::string, std::filesystem::path> modelPaths;
    /// Store the path of the texture currently loaded
    std::unordered_map<std::string, std::filesystem::path> texturePaths;
};

}    // namespace pivot::graphics::asset

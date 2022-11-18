#pragma once

#include <cstdint>

#include <optional>
#include <string>

namespace pivot::graphics::asset
{

/// @brief Represent a mesh in the buffers
struct Mesh {
    /// Starting offset of the mesh in the vertex buffer
    std::uint32_t vertexOffset;
    /// Number of vertex forming the mesh.
    std::uint32_t vertexSize;
    /// Starting offset of the mesh in the indice buffer
    std::uint32_t indicesOffset;
    /// Number of indice forming the mesh.
    std::uint32_t indicesSize;
    /// Equality operator
    constexpr bool operator==(const Mesh &other) const = default;
};

/// @brief A mesh with a default texture and a default material
struct Model {
    /// Model mesh
    Mesh mesh;
    /// Default material id
    std::optional<std::string> default_material;
    /// Equality operator
    bool operator==(const Model &) const = default;
};

}    // namespace pivot::graphics::asset

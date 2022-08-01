#pragma once

#include <cstdint>
#include <glm/mat4x4.hpp>
#include <optional>
#include <string>
#include <vector>

#include <pivot/containers/Node.hxx>

namespace pivot::graphics::asset
{

/// Represent a Primitive
struct Primitive {
    /// Starting offset of the mesh in the vertex buffer
    std::uint32_t vertexOffset;
    /// Number of vertex forming the mesh.
    std::uint32_t vertexSize;
    /// Starting offset of the mesh in the indice buffer
    std::uint32_t indicesOffset;
    /// Number of indice forming the mesh.
    std::uint32_t indicesSize;
    /// Default material id
    std::optional<std::string> default_material;
    /// Name of the primitive
    std::string name;
    /// Equality operator
    bool operator==(const Primitive &) const = default;
};

/// @brief A mesh with a default texture and a default material
struct Model {
    /// Name of the model
    std::string name;
    /// List of Primitive
    std::vector<Primitive> primitives;
    /// Local transformation matrix
    glm::mat4 localMatrix = glm::mat4(1.0f);
    /// Equality operator
    bool operator==(const Model &) const = default;
};

/// Model in a generic mode
using ModelNode = Node<Model>;
/// Pointer to a model node
using ModelPtr = ModelNode::NodePtr;

}    // namespace pivot::graphics::asset

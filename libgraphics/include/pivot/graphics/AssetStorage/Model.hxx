#pragma once

#include <cstdint>
#include <glm/mat4x4.hpp>
#include <optional>
#include <string>
#include <vector>

#include <pivot/containers/Node.hxx>

namespace pivot::graphics::asset
{

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
    std::string name;
    bool operator==(const Primitive &) const = default;
};

/// @brief A mesh with a default texture and a default material
struct Model {
    std::string name;
    std::vector<Primitive> primitives;
    glm::mat4 localMatrix = glm::mat4(1.0f);
    /// Equality operator
    bool operator==(const Model &) const = default;
};

using ModelNode = Node<Model>;
using ModelPtr = ModelNode::NodePtr;

}    // namespace pivot::graphics::asset

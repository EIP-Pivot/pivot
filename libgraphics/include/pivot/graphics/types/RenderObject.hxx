#pragma once

#include "pivot/graphics/types/Transform.hxx"

#include <optional>
#include <string>

/// @class RenderObject
///
/// @brief Structure representing a 3D object
struct RenderObject {
    /// Id of the mesh to use. The id is the stem of the file
    std::string meshID;
    /// The name of the material to use on the object
    std::string materialIndex;
    /// The object transform
    Transform transform;

    /// Default comparison operator
    auto operator<=>(const RenderObject &) const = default;
};

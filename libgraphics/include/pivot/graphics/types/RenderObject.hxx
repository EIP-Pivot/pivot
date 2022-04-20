#pragma once

#include "pivot/graphics/types/Transform.hxx"

#include <optional>
#include <string>

namespace pivot::graphics
{
/// @class RenderObject
///
/// @brief Structure representing a 3D object
struct RenderObject {
    /// Id of the mesh to use. The id is the stem of the file
    std::string meshID = "cube";
    /// Id of the pipeline to use
    std::string pipelineID = "";
    /// The name of the material to use on the object
    std::string materialIndex = "white";
    /// The object transform
    Transform transform;

    /// Default comparison operator
    auto operator<=>(const RenderObject &) const = default;
};
}    // namespace pivot::graphics

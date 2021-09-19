#pragma once

#include "pivot/graphics/types/UniformBufferObject.hxx"
#include <string>

/// @class RenderObject
///
/// @brief Structure representing a 3D object
struct RenderObject {
    /// Id of the mesh to use. The id is the stem of the file
    std::string meshID;
    /// The informations about the object
    /// @see ObjectInformation
    ObjectInformation objectInformation;
};

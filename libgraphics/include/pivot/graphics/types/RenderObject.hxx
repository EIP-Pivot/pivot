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
    /// Id of the pipeline to use
    std::string pipelineID;

    /// @class ObjectInformation
    ///
    /// @brief Hold the information of the 3D object, on the CPU-side
    struct ObjectInformation {
        /// The object transform
        Transform transform;
        /// The name of the texture to apply on the object
        std::optional<std::string> textureIndex;
        /// The name of the material to use on the object
        std::optional<std::string> materialIndex;
    }
    /// The informations about the object
    /// @see ObjectInformation
    objectInformation;
};

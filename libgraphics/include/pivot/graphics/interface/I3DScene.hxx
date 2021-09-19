#pragma once

#include <string>
#include <vector>

#include "pivot/graphics/types/RenderObject.hxx"

/// @class I3DScene
///
/// A interface representing the 3D scene to be rendered by the engine
class I3DScene
{
public:
    /// Get the information about the object to be rendered
    ///
    /// @return a vector containing the all the information about the 3D objects to render
    virtual std::vector<RenderObject> getSceneInformations() const = 0;
};
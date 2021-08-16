#pragma once

#include <string>
#include <vector>

#include "pivot/graphics/types/RenderObject.hxx"

class I3DScene
{
public:
    virtual std::vector<RenderObject> getSceneInformations() const = 0;
};
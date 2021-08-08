#pragma once

#include <string>
#include <vector>

#include "pivot/graphics/types/UniformBufferObject.hxx"

class I3DScene
{
public:
    virtual ~I3DScene();
    virtual std::vector<ObjectInformation> getSceneInformations() const = 0;
};
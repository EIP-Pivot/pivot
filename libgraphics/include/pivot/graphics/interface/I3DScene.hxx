#pragma once

#include <string>
#include <unordered_map>

#include "pivot/graphics/types/UniformBufferObject.hxx"

class I3DScene
{
public:
    virtual ~I3DScene();
    virtual std::unordered_map<std::string, ObjectInformations> getSceneInformations() const = 0;
};
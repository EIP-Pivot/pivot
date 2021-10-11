#pragma once
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/Mesh.hxx"

#include <string>
#include <unordered_map>

using ImageStorage = std::unordered_map<std::string, AllocatedImage>;
using MeshStorage = std::unordered_map<std::string, GPUMesh>;
using MeshBoundingBoxStorage = std::unordered_map<std::string, MeshBoundingBox>;
using MaterialStorage = std::unordered_map<std::string, gpuObject::Material>;

/// @namespace gpuObject
/// @brief Containing all the classes that will be used in a GLSL shader
///
/// They are contained into a separate namespace because they will be used inside the shaders, so the memory layout is
/// really important. As such, they need to be separated to avoid misuse
namespace gpuObject
{
}

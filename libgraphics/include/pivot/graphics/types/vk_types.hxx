#pragma once
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/Mesh.hxx"

#include <string>
#include <unordered_map>

using ImageStorage = std::unordered_map<std::string, AllocatedImage>;
using MeshStorage = std::unordered_map<std::string, GPUMesh>;
using MaterialStorage = std::unordered_map<std::string, gpuObject::Material>;
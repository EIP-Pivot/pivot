#pragma once

#if !defined(CULLING_DEBUG) && !defined(NDEBUG)
#define CULLING_DEBUG
#endif

#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/Material.hxx"
#include "pivot/graphics/types/Mesh.hxx"

#include <string>
#include <unordered_map>

template <typename T>
using OptionalRef = std::optional<std::reference_wrapper<T>>;

using ImageStorage = std::unordered_map<std::string, AllocatedImage>;
using MeshStorage = std::unordered_map<std::string, GPUMesh>;
using MeshBoundingBoxStorage = std::unordered_map<std::string, MeshBoundingBox>;
using MaterialStorage = std::unordered_map<std::string, gpuObject::Material>;
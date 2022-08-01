#pragma once

#include "pivot/graphics/AssetStorage/CPUTexture.hxx"
#include "pivot/graphics/types/Vertex.hxx"

namespace pivot::graphics::asset
{

/// name of the fallback texture if missing
const std::string missing_texture_name = "internal/missing_texture";
/// data of the default texture
const CPUTexture default_texture_data = {
    .image =
        {
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0xff),

            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),

            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),
            std::byte(0xff),

            std::byte(0x00),
            std::byte(0x00),
            std::byte(0x00),
            std::byte(0xff),
        },
    .size = {2, 2, 1},
};

/// name of the default material if missing
const std::string missing_material_name = "internal/missing_material";

/// name of the default quad mesh
const std::string quad_mesh_id = "internal/quad_mesh";

/// Vertices of the default mesh
const std::vector<Vertex> quad_mesh_vertices = {
    {
        .pos = {-0.5f, -0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {-0.5f, -0.5f},
        .color = {1.0f, 0.0f, 0.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
    {
        .pos = {0.5f, -0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {0.5f, -0.5f},
        .color = {0.0f, 1.0f, 0.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
    {
        .pos = {0.5f, 0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {0.5f, 0.5f},
        .color = {0.0f, 0.0f, 1.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
    {
        .pos = {-0.5f, 0.5f, 0.f},
        .normal = {0.f, 0.f, 0.f},
        .texCoord = {-0.5f, 0.5f},
        .color = {1.0f, 1.0f, 1.0f},
        .tangent = {0.0f, 0.0f, 0.0f, 0.0f},
    },
};

/// Indeices of the default mesh
const std::vector<std::uint32_t> quad_mesh_indices = {0, 1, 2, 2, 3, 0};

}    // namespace pivot::graphics::asset

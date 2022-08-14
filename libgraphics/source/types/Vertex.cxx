#include "pivot/graphics/types/Vertex.hxx"

namespace pivot::graphics
{

bool Vertex::operator==(const Vertex &other) const noexcept
{
    return pos == other.pos && color == other.color && normal == other.normal && texCoord == other.texCoord;
}

vk::VertexInputBindingDescription Vertex::getBindingDescription() noexcept
{
    return {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };
}

vk::VertexInputAttributeDescription Vertex::inputAttributeDescription(uint32_t binding, uint32_t location,
                                                                      VertexComponentFlagBits component)
{
    switch (component) {
        case VertexComponentFlagBits::Position:
            return {
                location,
                binding,
                vk::Format::eR32G32B32Sfloat,
                offsetof(Vertex, pos),
            };
        case VertexComponentFlagBits::Normal:
            return {
                location,
                binding,
                vk::Format::eR32G32B32Sfloat,
                offsetof(Vertex, normal),
            };
        case VertexComponentFlagBits::UV:
            return {
                location,
                binding,
                vk::Format::eR32G32Sfloat,
                offsetof(Vertex, texCoord),
            };
        case VertexComponentFlagBits::Color:
            return {
                location,
                binding,
                vk::Format::eR32G32B32Sfloat,
                offsetof(Vertex, color),
            };
        case VertexComponentFlagBits::Tangent:
            return {
                location,
                binding,
                vk::Format::eR32G32B32A32Sfloat,
                offsetof(Vertex, tangent),
            };
    }
    return {};
}

std::vector<vk::VertexInputAttributeDescription>
Vertex::getInputAttributeDescriptions(uint32_t binding, const VertexComponentFlags components)
{
    std::vector<vk::VertexInputAttributeDescription> result;
    uint32_t location = 0;

    if (components & VertexComponentFlagBits::Position) {
        result.push_back(Vertex::inputAttributeDescription(binding, location, VertexComponentFlagBits::Position));
        location++;
    }
    if (components & VertexComponentFlagBits::Normal) {
        result.push_back(Vertex::inputAttributeDescription(binding, location, VertexComponentFlagBits::Normal));
        location++;
    }
    if (components & VertexComponentFlagBits::UV) {
        result.push_back(Vertex::inputAttributeDescription(binding, location, VertexComponentFlagBits::UV));
        location++;
    }
    if (components & VertexComponentFlagBits::Color) {
        result.push_back(Vertex::inputAttributeDescription(binding, location, VertexComponentFlagBits::Color));
        location++;
    }
    if (components & VertexComponentFlagBits::Tangent) {
        result.push_back(Vertex::inputAttributeDescription(binding, location, VertexComponentFlagBits::Tangent));
        location++;
    }
    return result;
}

}    // namespace pivot::graphics

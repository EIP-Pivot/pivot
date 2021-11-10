#include "pivot/graphics/types/Vertex.hxx"

namespace pivot::graphics
{

bool Vertex::operator==(const Vertex &other) const
{
    return pos == other.pos && color == other.color && normal == other.normal && texCoord == other.texCoord;
}
bool Vertex::operator!=(const Vertex &other) const { return !((*this) == other); }

vk::VertexInputBindingDescription Vertex::getBindingDescription() noexcept
{
    vk::VertexInputBindingDescription bindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = vk::VertexInputRate::eVertex,
    };

    return bindingDescription;
}

std::vector<vk::VertexInputAttributeDescription> Vertex::getAttributeDescriptons() noexcept
{
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(4);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[2].offset = offsetof(Vertex, color);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[3].offset = offsetof(Vertex, texCoord);
    return attributeDescriptions;
}

}    // namespace pivot::graphics
#include "pivot/graphics/TransientRessourceSystem/RenderPassRessource.hxx"

namespace pivot::graphics::trs
{

RenderPassRessources::RenderPassRessources() {}

RenderPassRessources::~RenderPassRessources() {}

AllocatedImage RenderPassRessources::getTexture(const Ticket &ticket) const
{
    auto iter = textureStorage.find(ticket);
    if (iter == textureStorage.end())
        throw RenderPassRessourcesError("Ticket does not correspond to a texture, or is not part of this render pass.");
    return iter->second.texture;
}

vk::RenderPass RenderPassRessources::getRenderPass(const Ticket &ticket) const
{
    auto iter = renderPassStorage.find(ticket);
    if (iter == renderPassStorage.end())
        throw RenderPassRessourcesError(
            "Ticket does not correspond to a vk::RenderPass, or is not part of this render pass.");
    return iter->second;
}

}    // namespace pivot::graphics::trs

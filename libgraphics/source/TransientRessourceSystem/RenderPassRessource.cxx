#include "pivot/graphics/TransientRessourceSystem/RenderPassRessource.hxx"

namespace pivot::graphics::trs
{

RenderPassRessources::RenderPassRessources() {}

RenderPassRessources::~RenderPassRessources() {}

AllocatedImage RenderPassRessources::getTexture(const Ticket &ticket) const
{
    auto iter = textureStorage.find(ticket);
    if (iter == textureStorage.end())
        throw RenderPassRessourcesError("Ticket not found in this render pass requested texture.");
    return iter->second.texture;
}

}    // namespace pivot::graphics::trs

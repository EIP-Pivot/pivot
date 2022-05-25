#include "pivot/graphics/TransientRessourceSystem/RenderPassBuilder.hxx"

namespace pivot::graphics::trs
{

RenderPassBuilder::RenderPassBuilder() {}

RenderPassBuilder::~RenderPassBuilder() {}

Ticket RenderPassBuilder::read(const Ticket &ticket, ReadFlags) { return ticket.newVersion(); }

Ticket RenderPassBuilder::write(const Ticket &ticket, WriteFlags) { return ticket.newVersion(); }

Ticket RenderPassBuilder::create(const TextureDescription &desc)
{
    auto ret = Ticket::newTicket();
    masterTextureStorage[ret] = desc;
    return ret;
}

Ticket RenderPassBuilder::useRenderTarget(const Ticket &ticket, vk::RenderPass pass)
{
    renderPassStorage[ticket] = pass;
    return ticket.newVersion();
}

}    // namespace pivot::graphics::trs

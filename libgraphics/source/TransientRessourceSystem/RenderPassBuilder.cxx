#include "pivot/graphics/TransientRessourceSystem/RenderPassBuilder.hxx"

namespace pivot::graphics::trs
{

RenderPassBuilder::RenderPassBuilder() {}

RenderPassBuilder::~RenderPassBuilder() {}

Ticket RenderPassBuilder::read(const Ticket &ticket, ReadFlags)
{
    readTicket.push_back(ticket);
    return ticket;
}

Ticket RenderPassBuilder::write(const Ticket &ticket, WriteFlags)
{
    writeTicketConsumed.push_back(ticket);
    auto new_ticket = ticket.newVersion();
    writeTicketProduced.push_back(new_ticket);
    return new_ticket;
}

Ticket RenderPassBuilder::create(const TextureDescription &desc)
{
    auto ret = Ticket::newTicket();
    textureCreation[ret] = desc;
    return ret;
}

Ticket RenderPassBuilder::useRenderTarget(const Ticket &ticket, vk::RenderPass pass)
{
    auto t = write(ticket, {});
    renderPasses.emplace_back(t, pass);
    return t;
}

}    // namespace pivot::graphics::trs

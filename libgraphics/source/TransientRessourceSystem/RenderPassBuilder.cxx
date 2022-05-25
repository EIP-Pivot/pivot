#include "pivot/graphics/TransientRessourceSystem/RenderPassBuilder.hxx"

namespace pivot::graphics::trs
{

RenderPassBuilder::RenderPassBuilder(VulkanAllocator &allocator): allocator(allocator) {}

RenderPassBuilder::~RenderPassBuilder() {}

// Ticket RenderPassBuilder::useRenderTarget(const Ticket &ticket) { return ticket.newVersion(); }

Ticket RenderPassBuilder::read(const Ticket &ticket, ReadFlags flag) { return ticket.newVersion(); }

Ticket RenderPassBuilder::write(const Ticket &ticket, WriteFlags flag) { return ticket.newVersion(); }

Ticket RenderPassBuilder::create(const TextureDescription &desc) { return Ticket::newTicket(); }

}    // namespace pivot::graphics::trs

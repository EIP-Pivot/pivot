#pragma once

#include "pivot/exception.hxx"
#include "pivot/graphics/TransientRessourceSystem/RenderPassBuilder.hxx"
#include "pivot/graphics/TransientRessourceSystem/Ticket.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"

#include <any>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics::trs
{

class RenderPassRessources
{
public:
    RUNTIME_ERROR(RenderPassRessources)

public:
    RenderPassRessources();
    ~RenderPassRessources();

    AllocatedImage getTexture(const Ticket &ticket) const;
    template <BufferValid T>
    AllocatedBuffer<T> getBuffer(const Ticket &ticket) const
    {
        auto iter = bufferStorage.find(ticket);
        if (iter == bufferStorage.end())
            throw RenderPassRessourcesError("Ticket not found in this render pass requested buffer");

        const std::type_index expected = typeid(T);
        if (iter->second.type != expected) {
            logger.err("RenderPassRessource") << "Error while getting buffer. Expected T = " << iter->second.type.name()
                                              << " but got T = " << expected.name();
            throw RenderPassRessourcesError("Requested type not correct !");
        }
#warning unimplemented
        return {};
    }

    vk::Framebuffer getRenderTarget(const Ticket &ticket) const;

private:
    std::unordered_map<Ticket, RenderPassBuilder::TextureDescription> textureStorage;
    std::unordered_map<Ticket, RenderPassBuilder::VoidBufferDescription> bufferStorage;
    std::unordered_map<Ticket, vk::Framebuffer> framebufferStorage;
};

}    // namespace pivot::graphics::trs

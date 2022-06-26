#pragma once

#include <any>
#include <map>
#include <typeindex>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/TransientRessourceSystem/Ticket.hxx"
#include "pivot/graphics/VulkanAllocator.hxx"
#include "pivot/graphics/VulkanRenderPass.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"
#include "pivot/utility/flags.hxx"

namespace pivot::graphics::trs
{

class RenderPassBuilder
{
public:
    enum class InitialState : FlagsType { Clear, DontCare };

    enum class ReadFlagBits : FlagsType {};
    using ReadFlags = Flags<ReadFlagBits>;

    enum class WriteFlagBits : FlagsType {};
    using WriteFlags = Flags<WriteFlagBits>;

    struct TextureDescription {
        std::size_t width;
        std::size_t height;
        vk::Format format;
        InitialState initialState = InitialState::DontCare;
        vma::MemoryUsage memUsage = vma::MemoryUsage::eAuto;
        std::string debug_name = "";
        bool operator==(const TextureDescription &) const = default;
    };

    template <BufferValid T>
    struct BufferDescription {
        std::size_t size;
        vk::BufferUsageFlags usage;
        InitialState initialState = InitialState::DontCare;
        vma::MemoryUsage memUsage = vma::MemoryUsage::eAuto;
        std::string debug_name = "";
        bool operator==(const BufferDescription &) const = default;
    };

    struct VoidBufferDescription {
        template <BufferValid T>
        VoidBufferDescription(const BufferDescription<T> &des): desc(des), type(typeid(T))
        {
        }

        BufferDescription<void> desc;
        std::type_index type;
    };

public:
    RenderPassBuilder();
    ~RenderPassBuilder();

    // Ticket useRenderTarget(const Ticket &ticket);
    Ticket read(const Ticket &ticket, ReadFlags flag = {});
    Ticket write(const Ticket &ticket, WriteFlags flag = {});
    Ticket create(const TextureDescription &desc);
    Ticket useRenderTarget(const Ticket &ticket, vk::RenderPass renderPass);

    template <BufferValid T>
    Ticket create(const BufferDescription<T> &desc)
    {
        auto ret = Ticket::newTicket();
        bufferCreation.emplace(ret, desc);
        return ret;
    }

private:
    std::map<Ticket, TextureDescription> textureCreation;
    std::map<Ticket, VoidBufferDescription> bufferCreation;
    std::vector<Ticket> readTicket;
    std::vector<Ticket> writeTicketConsumed;
    std::vector<Ticket> writeTicketProduced;
    std::vector<std::pair<Ticket, vk::RenderPass>> renderPasses;

    friend class FrameGraph;
};

}    // namespace pivot::graphics::trs

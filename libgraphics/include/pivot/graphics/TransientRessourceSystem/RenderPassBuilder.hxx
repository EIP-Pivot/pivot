#pragma once

#include <any>
#include <map>
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
    };

    template <BufferValid T>
    struct BufferDescription {
        std::size_t size;
        vk::BufferUsageFlags usage;
        InitialState initialState = InitialState::DontCare;
        vma::MemoryUsage memUsage = vma::MemoryUsage::eAuto;
        std::string debug_name = "";
    };

    struct Texture {
        RenderPassBuilder::TextureDescription description;
        AllocatedImage texture;
    };

    template <BufferValid T>
    struct Buffer {
        RenderPassBuilder::BufferDescription<T> description;
        AllocatedBuffer<T> buffer;
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
        masterBufferStorage[ret] = desc;
        return ret;
    }

private:
    std::map<Ticket, TextureDescription> masterTextureStorage;
    std::map<Ticket, std::any> masterBufferStorage;
    std::unordered_map<Ticket, vk::RenderPass> renderPassStorage;
};

}    // namespace pivot::graphics::trs

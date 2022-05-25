#pragma once

#include <any>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/PivotFlags.hxx"
#include "pivot/graphics/TransientRessourceSystem/Ticket.hxx"
#include "pivot/graphics/VulkanAllocator.hxx"
#include "pivot/graphics/types/AllocatedBuffer.hxx"
#include "pivot/graphics/types/AllocatedImage.hxx"

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
        InitialState initialState;
        std::string debug_name = "";
    };

    template <BufferValid T>
    struct BufferDescription {
        std::size_t size;
        vk::BufferUsageFlags usage;
        vma::MemoryUsage memUsage;
        InitialState initialState;
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
    RenderPassBuilder(VulkanAllocator &allocator);
    ~RenderPassBuilder();

    // Ticket useRenderTarget(const Ticket &ticket);
    Ticket read(const Ticket &ticket, ReadFlags flag = {});
    Ticket write(const Ticket &ticket, WriteFlags flag = {});
    Ticket create(const TextureDescription &desc);
    template <BufferValid T>
    Ticket create(const BufferDescription<T> &desc)
    {
        auto ret = Ticket::newTicket();
        auto buffer = allocator.createBuffer<T>(desc.size, desc.usage, desc.memUsage);
        masterBufferStorage[ret] = buffer;
        return ret;
    }

private:
    VulkanAllocator &allocator;
    std::unordered_map<Ticket, Texture> masterTextureStorage;
    std::unordered_map<Ticket, std::any> masterBufferStorage;
};

}    // namespace pivot::graphics::trs

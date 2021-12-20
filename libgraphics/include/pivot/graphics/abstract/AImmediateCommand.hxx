#pragma once

#include "pivot/graphics/types/common.hxx"

namespace pivot::graphics::abstract
{

/// @class AImmediateCommand
/// Used to easly add immediateCommand functionality to an herited class
class AImmediateCommand
{
public:
    /// Default ctor
    AImmediateCommand();
    /// Default dtor
    ~AImmediateCommand();

    /// Initialize the immediate context, using a device an a queue
    void init(vk::Device &, const uint32_t queueFamilyIndex);
    /// Destroy the immediate context and releasing the reference to the device
    void destroy();

    /// Perform an command on the GPU immediately and wait for it to complete
    void immediateCommand(std::function<void(vk::CommandBuffer &)> cmd);

private:
    void createImmediateContext(const uint32_t queueFamilyIndex);

private:
    OptionalRef<vk::Device> device_ref;

    vk::Fence immediateFence = VK_NULL_HANDLE;
    vk::CommandPool immediateCommandPool = VK_NULL_HANDLE;
    vk::Queue immediateQueue = VK_NULL_HANDLE;
};

}    // namespace pivot::graphics::abstract
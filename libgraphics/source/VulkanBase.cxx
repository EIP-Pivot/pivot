#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/DebugMacros.hxx"

#include <Logger.hpp>

namespace pivot::graphics
{

VulkanBase::VulkanBase(): window("Vulkan", 800, 600)
{
    DEBUG_FUNCTION;
    if (bEnableValidationLayers && !checkValidationLayerSupport()) {
        logger->warn("Vulkan Instance") << "Validation layers requested, but not available!";
        LOGGER_ENDL;
        bEnableValidationLayers = false;
    }
}

VulkanBase::~VulkanBase() { DEBUG_FUNCTION; }

void VulkanBase::init()
{
    DEBUG_FUNCTION;
    createInstance();
    createDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();

    swapchain.create(window.getSize(), physical_device, device, surface);

    createCommandPool();
    createCommandBuffers();

    createSyncStructure();
}

void VulkanBase::destroy()
{
    DEBUG_FUNCTION;
    baseDeletionQueue.flush();
}

}    // namespace pivot::graphics
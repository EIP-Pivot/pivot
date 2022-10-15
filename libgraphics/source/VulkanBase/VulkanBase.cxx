#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/pivot.hxx"

#include <optional>

namespace pivot::graphics
{
VulkanBase::VulkanBase(const bool bForceValidation): VulkanLoader(), VulkanImmediateCommand()
{
    DEBUG_FUNCTION();
    bEnableValidationLayers |= bForceValidation;
}

VulkanBase::~VulkanBase() { DEBUG_FUNCTION(); }

void VulkanBase::init(Window &window, const std::vector<const char *> &instanceExtensions,
                      const std::vector<const char *> &deviceExtensions,
                      const std::vector<const char *> &validationLayers)
{
    DEBUG_FUNCTION();
    window_ref = window;
    createInstance(instanceExtensions, validationLayers);
    createDebugMessenger();
    createSurface();
    selectPhysicalDevice(deviceExtensions);
    createLogicalDevice(deviceExtensions);
    createAllocator();
    VulkanImmediateCommand::init(device, physical_device, queueIndices);
}

void VulkanBase::destroy()
{
    DEBUG_FUNCTION();

    VulkanImmediateCommand::destroy();
    baseDeletionQueue.flush();
}

}    // namespace pivot::graphics

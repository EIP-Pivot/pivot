#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/pivot.hxx"

#include <optional>

namespace pivot::graphics
{
VulkanBase::VulkanBase(const std::string &windowName, const bool bForceValidation)
    : VulkanLoader(), abstract::AImmediateCommand(), window(windowName, 800, 600)
{
    bEnableValidationLayers |= bForceValidation;
}

VulkanBase::~VulkanBase() {}

void VulkanBase::init(const std::vector<const char *> &instanceExtensions,
                      const std::vector<const char *> &deviceExtensions,
                      const std::vector<const char *> &validationLayers)
{
    DEBUG_FUNCTION
    createInstance(instanceExtensions, validationLayers);
    createDebugMessenger();
    createSurface();
    selectPhysicalDevice(deviceExtensions);
    createLogicalDevice(deviceExtensions);
    createAllocator();
    abstract::AImmediateCommand::init(device, physical_device, queueIndices.transferFamily.value());
}

void VulkanBase::destroy()
{
    DEBUG_FUNCTION;
    abstract::AImmediateCommand::destroy();
    baseDeletionQueue.flush();
}

}    // namespace pivot::graphics

#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

class IWindow
{
public:
    virtual ~IWindow() = 0;
    virtual void createSurface(const VkInstance &, VkSurfaceKHR *) = 0;
    virtual VkExtent2D getSize() = 0;

    virtual std::vector<const char *> getRequiredExtensions() = 0;
};
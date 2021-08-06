#pragma once

#include <stdexcept>
#include <vulkan/vulkan.h>

struct VulkanException : public std::runtime_error {
    VulkanException(const std::string message): std::runtime_error(message) {}
    VulkanException(const VkResult er): std::runtime_error(errorString(er)) {}

    static std::string errorString(VkResult errorCode);
};

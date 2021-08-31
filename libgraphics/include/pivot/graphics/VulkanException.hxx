#pragma once

#include <stdexcept>
#include <vulkan/vulkan.hpp>

struct VulkanException : public std::runtime_error {
    VulkanException(const std::string &message): std::runtime_error(message){};
    VulkanException(const vk::Result &er): std::runtime_error(vk::to_string(er)){};
    VulkanException(const VkResult &er): VulkanException(vk::Result(er)){};
};

struct OutOfDateSwapchainError : public VulkanException {
    OutOfDateSwapchainError(const std::string &message = "OutOfDateSwapchainError"): VulkanException(message) {}
};

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
    const char *what() const noexcept override
    {
        return "The Swapchain is not up to date, most likely due to window resize. It should be recreated asap";
    }
};

struct TooManyObjectInSceneError : public VulkanException {
    TooManyObjectInSceneError(const std::string &message = "TooManyObjectInSceneError"): VulkanException(message) {}
    const char *what() const noexcept override
    {
        return "The scene provided contain too many object. Try reducing the amount of drawn object";
    }
};
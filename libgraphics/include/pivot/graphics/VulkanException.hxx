#pragma once

#include <stdexcept>
#include <vulkan/vulkan.hpp>

/// @class VulkanException
/// @brief A custom exception throw when a error occurred in the graphical engine
///
/// @see OutOfDateSwapchainError
/// @see TooManyObjectInSceneError
struct VulkanException : public std::runtime_error {
    VulkanException(const std::string &message): std::runtime_error(message){};
    /// Constructor converting a vk::Result into a string and use it as error message
    VulkanException(const vk::Result &er): VulkanException(vk::to_string(er)){};
};

/// @class OutOfDateSwapchainError
/// @brief VulkanException specialization for handeling outdated swapchain
///
/// This exception is not fatal, and can usually be fixed by recreating the Swapchain to the correct size or format
struct OutOfDateSwapchainError : public VulkanException {
    OutOfDateSwapchainError(const std::string &message = "OutOfDateSwapchainError"): VulkanException(message) {}
    const char *what() const noexcept override
    {
        return "The Swapchain is not up to date, most likely due to window resize. It should be recreated asap";
    }
};

/// @class TooManyObjectInSceneError
/// @brief VulkanException specialization throw when the scene is bigger than what the engine can handle
///
/// This exception is not fatal, and can usually be fixed by reducing the number of 3D object in the scene
///
/// @see I3DScene
struct TooManyObjectInSceneError : public VulkanException {
    TooManyObjectInSceneError(const std::string &message = "TooManyObjectInSceneError"): VulkanException(message) {}
    const char *what() const noexcept override
    {
        return "The scene provided contain too many object. Try reducing the amount of drawn object";
    }
};
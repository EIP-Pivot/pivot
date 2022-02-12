#pragma once

#include <stdexcept>
#include <vulkan/vulkan.hpp>

namespace pivot::graphics
{

/// @class VulkanException
/// @brief A custom exception throw when a error occurred in the graphical engine
///
/// @see OutOfDateSwapchainError
/// @see TooManyObjectInSceneError
struct VulkanException : public std::runtime_error {
    using std::runtime_error::runtime_error;
    /// Constructor converting a vk::Result into a string and use it as error message
    VulkanException(const vk::Result &er): VulkanException(vk::to_string(er)){};
};

/// @class OutOfDateSwapchainError
/// @brief VulkanException specialization for handeling outdated swapchain
///
/// This exception is not fatal, and can usually be fixed by recreating the Swapchain to the correct size or format
struct OutOfDateSwapchainError : public VulkanException {
    /// Constructor
    OutOfDateSwapchainError(const std::string &message = "OutOfDateSwapchainError"): VulkanException(message) {}

    /// Print an coherent error message
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
    /// Constructor
    TooManyObjectInSceneError(const std::string &message = "TooManyObjectInSceneError"): VulkanException(message) {}

    /// Print an coherent error message
    const char *what() const noexcept override
    {
        return "The scene provided contain too many object. Try reducing the amount of drawn object";
    }
};

}    // namespace pivot::graphics

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

}    // namespace pivot::graphics

#pragma once

#include <cstddef>
#include <span>
#include <stdint.h>
#include <vulkan/vulkan.hpp>

namespace vk_init
{
vk::DebugUtilsMessengerCreateInfoEXT
populateDebugUtilsMessengerCreateInfoEXT(PFN_vkDebugUtilsMessengerCallbackEXT callback);
vk::DeviceQueueCreateInfo populateDeviceQueueCreateInfo(const uint32_t, const uint32_t, const float &);
vk::ImageViewCreateInfo populateVkImageViewCreateInfo(vk::Image &image, vk::Format format, uint32_t mipLevel = 1);
vk::ShaderModuleCreateInfo populateVkShaderModuleCreateInfo(const std::span<std::byte> &code);

vk::PipelineShaderStageCreateInfo populateVkPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits stage,
                                                                          vk::ShaderModule &module,
                                                                          const char *entryPoint = "main");
vk::PipelineVertexInputStateCreateInfo
populateVkPipelineVertexInputStateCreateInfo(const std::span<const vk::VertexInputBindingDescription> &binding,
                                             const std::span<const vk::VertexInputAttributeDescription> &attribute);
vk::PipelineInputAssemblyStateCreateInfo populateVkPipelineInputAssemblyCreateInfo(vk::PrimitiveTopology,
                                                                                   vk::Bool32 = VK_FALSE);
vk::PipelineRasterizationStateCreateInfo populateVkPipelineRasterizationStateCreateInfo(vk::PolygonMode);
vk::PipelineMultisampleStateCreateInfo
populateVkPipelineMultisampleStateCreateInfo(vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1);
vk::PipelineColorBlendAttachmentState populateVkPipelineColorBlendAttachmentState();

vk::PipelineLayoutCreateInfo
populateVkPipelineLayoutCreateInfo(const std::span<const vk::DescriptorSetLayout> &setLayout,
                                   const std::span<const vk::PushConstantRange> &pushLayout);

vk::PipelineDepthStencilStateCreateInfo populateVkPipelineDepthStencilStateCreateInfo();
vk::PushConstantRange populateVkPushConstantRange(vk::ShaderStageFlags stage, uint32_t size, uint32_t offset = 0);
}    // namespace vk_init

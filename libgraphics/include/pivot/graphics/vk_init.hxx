#pragma once

#include <cstddef>
#include <stdint.h>
#include <vector>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

namespace vk_init
{
VkInstanceCreateInfo populateVkInstanceCreateInfo(VkApplicationInfo &appCreateInfo,
                                                  const std::vector<const char *> &vExtentions,
                                                  const std::vector<const char *> &vLayers);
VkDebugUtilsMessengerCreateInfoEXT populateDebugUtilsMessengerCreateInfoEXT(
    VKAPI_ATTR VkBool32 VKAPI_CALL (*)(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                                       const VkDebugUtilsMessengerCallbackDataEXT *, void *));
VkDeviceQueueCreateInfo populateDeviceQueueCreateInfo(const uint32_t, const uint32_t, const float &);
VkImageViewCreateInfo populateVkImageViewCreateInfo(VkImage &image, VkFormat format, uint32_t mipLevel = 1);
VkShaderModuleCreateInfo populateVkShaderModuleCreateInfo(const std::vector<std::byte> &code);

VkPipelineShaderStageCreateInfo populateVkPipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                                                        VkShaderModule &module,
                                                                        const char *entryPoint = "main");
VkPipelineVertexInputStateCreateInfo
populateVkPipelineVertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription> &binding,
                                             const std::vector<VkVertexInputAttributeDescription> &attribute);
VkPipelineInputAssemblyStateCreateInfo populateVkPipelineInputAssemblyCreateInfo(VkPrimitiveTopology,
                                                                                 VkBool32 = VK_FALSE);
VkPipelineRasterizationStateCreateInfo populateVkPipelineRasterizationStateCreateInfo(VkPolygonMode);
VkPipelineMultisampleStateCreateInfo
populateVkPipelineMultisampleStateCreateInfo(VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT);
VkPipelineColorBlendAttachmentState populateVkPipelineColorBlendAttachmentState();

VkPipelineLayoutCreateInfo populateVkPipelineLayoutCreateInfo(const std::vector<VkDescriptorSetLayout> &setLayout,
                                                              const std::vector<VkPushConstantRange> &pushLayout);

VkPipelineDepthStencilStateCreateInfo populateVkPipelineDepthStencilStateCreateInfo();
VkPushConstantRange populateVkPushConstantRange(VkShaderStageFlags stage, uint32_t size, uint32_t offset = 0);

namespace empty
{
    VkPipelineLayoutCreateInfo populateVkPipelineLayoutCreateInfo();
}
}    // namespace vk_init

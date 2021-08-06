#include "pivot/graphics/VulkanLoader.hxx"

#include <Logger.hpp>
#include <ostream>
#include <string>
#include <vector>

#include "pivot/graphics/VulkanException.hxx"
#include "pivot/graphics/vk_utils.hxx"

#define VULKAN_LOAD_TEST(NAME) \
    if (NAME == nullptr) missingRequiredExtensions.push_back(#NAME);

#define VULKAN_LOAD(NAME) NAME = (PFN_##NAME)vkGetInstanceProcAddr(instance, #NAME);
#define VULKAN_LOAD_REQUIRED(NAME) VULKAN_LOAD(NAME) VULKAN_LOAD_TEST(NAME)

VulkanLoader::VulkanLoader() {}
VulkanLoader::~VulkanLoader()
{
    if (instance != VK_NULL_HANDLE) { vkDestroyInstance(instance, nullptr); }
}

void VulkanLoader::createInstance(const VkInstanceCreateInfo &createInfo)
{
    std::vector<std::string> missingRequiredExtensions;

    VK_TRY(vkCreateInstance(&createInfo, nullptr, &instance));

    VULKAN_LOAD(vkCreateDebugUtilsMessengerEXT);
    VULKAN_LOAD(vkDestroyDebugUtilsMessengerEXT);

    VULKAN_LOAD_REQUIRED(vkDestroyInstance);

    VULKAN_LOAD_REQUIRED(vkEnumeratePhysicalDevices);
    VULKAN_LOAD_REQUIRED(vkGetPhysicalDeviceProperties);
    VULKAN_LOAD_REQUIRED(vkGetPhysicalDeviceFormatProperties);

    VULKAN_LOAD_REQUIRED(vkDeviceWaitIdle);
    VULKAN_LOAD_REQUIRED(vkCreateDevice);
    VULKAN_LOAD_REQUIRED(vkDestroyDevice);

    VULKAN_LOAD_REQUIRED(vkDestroySurfaceKHR);
    VULKAN_LOAD_REQUIRED(vkGetDeviceQueue);

    VULKAN_LOAD_REQUIRED(vkCreateRenderPass);
    VULKAN_LOAD_REQUIRED(vkDestroyRenderPass);

    VULKAN_LOAD_REQUIRED(vkCreatePipelineLayout);
    VULKAN_LOAD_REQUIRED(vkDestroyPipelineLayout);
    VULKAN_LOAD_REQUIRED(vkCreateGraphicsPipelines);
    VULKAN_LOAD_REQUIRED(vkDestroyPipeline);

    VULKAN_LOAD_REQUIRED(vkCreateShaderModule);
    VULKAN_LOAD_REQUIRED(vkDestroyShaderModule);

    VULKAN_LOAD_REQUIRED(vkCreateFramebuffer);
    VULKAN_LOAD_REQUIRED(vkDestroyFramebuffer);

    VULKAN_LOAD_REQUIRED(vkCreateCommandPool);
    VULKAN_LOAD_REQUIRED(vkDestroyCommandPool);

    VULKAN_LOAD_REQUIRED(vkAllocateCommandBuffers);
    VULKAN_LOAD_REQUIRED(vkFreeCommandBuffers);

    VULKAN_LOAD_REQUIRED(vkCreateFence);
    VULKAN_LOAD_REQUIRED(vkCreateSemaphore);
    VULKAN_LOAD_REQUIRED(vkDestroyFence);
    VULKAN_LOAD_REQUIRED(vkDestroySemaphore);

    VULKAN_LOAD_REQUIRED(vkCreateDescriptorPool);
    VULKAN_LOAD_REQUIRED(vkDestroyDescriptorPool);
    VULKAN_LOAD_REQUIRED(vkCreateDescriptorSetLayout);
    VULKAN_LOAD_REQUIRED(vkDestroyDescriptorSetLayout);

    VULKAN_LOAD_REQUIRED(vkAllocateDescriptorSets);
    VULKAN_LOAD_REQUIRED(vkUpdateDescriptorSets);

    VULKAN_LOAD_REQUIRED(vkCreateImageView);
    VULKAN_LOAD_REQUIRED(vkDestroyImageView);

    VULKAN_LOAD_REQUIRED(vkCreateSampler);
    VULKAN_LOAD_REQUIRED(vkDestroySampler);

    VULKAN_LOAD_REQUIRED(vkBeginCommandBuffer);
    VULKAN_LOAD_REQUIRED(vkEndCommandBuffer);
    VULKAN_LOAD_REQUIRED(vkQueueSubmit);
    VULKAN_LOAD_REQUIRED(vkWaitForFences);
    VULKAN_LOAD_REQUIRED(vkResetFences);
    VULKAN_LOAD_REQUIRED(vkResetCommandPool);

    VULKAN_LOAD_REQUIRED(vkCmdCopyBuffer);
    VULKAN_LOAD_REQUIRED(vkCmdCopyBufferToImage);
    VULKAN_LOAD_REQUIRED(vkCmdPipelineBarrier);
    VULKAN_LOAD_REQUIRED(vkCmdBlitImage);
    VULKAN_LOAD_REQUIRED(vkCmdBindPipeline);
    VULKAN_LOAD_REQUIRED(vkCmdBindDescriptorSets);
    VULKAN_LOAD_REQUIRED(vkCmdPushConstants);
    VULKAN_LOAD_REQUIRED(vkCmdBeginRenderPass);
    VULKAN_LOAD_REQUIRED(vkCmdBindVertexBuffers);
    VULKAN_LOAD_REQUIRED(vkCmdBindIndexBuffer);
    VULKAN_LOAD_REQUIRED(vkCmdDrawIndexed);
    VULKAN_LOAD_REQUIRED(vkCmdEndRenderPass);
    VULKAN_LOAD_REQUIRED(vkQueuePresentKHR);

    for (const auto &i: missingRequiredExtensions) {
        logger->err("VulkanLoader") << "missing symbol  " << i;
        LOGGER_ENDL;
    }
    if (!missingRequiredExtensions.empty())
        throw VulkanException("Missing symbols : " + std::to_string(missingRequiredExtensions.size()));
}

#undef VULKAN_LOAD
#undef VULKAN_LOAD_TEST
#undef VULKAN_LOAD_REQUIRED

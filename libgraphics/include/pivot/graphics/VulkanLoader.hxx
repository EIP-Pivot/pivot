#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#define VULKAN_PROTOTYPE(NAME) PFN_##NAME NAME = nullptr

class VulkanLoader
{
public:
    VulkanLoader();
    ~VulkanLoader();
    void createInstance(const VkInstanceCreateInfo &createInfo);

public:
    VULKAN_PROTOTYPE(vkCreateDebugUtilsMessengerEXT);
    VULKAN_PROTOTYPE(vkDestroyDebugUtilsMessengerEXT);

    VULKAN_PROTOTYPE(vkDestroyInstance);

    VULKAN_PROTOTYPE(vkEnumeratePhysicalDevices);
    VULKAN_PROTOTYPE(vkGetPhysicalDeviceProperties);
    VULKAN_PROTOTYPE(vkGetPhysicalDeviceFormatProperties);
    VULKAN_PROTOTYPE(vkDeviceWaitIdle);
    VULKAN_PROTOTYPE(vkCreateDevice);
    VULKAN_PROTOTYPE(vkDestroyDevice);

    VULKAN_PROTOTYPE(vkDestroySurfaceKHR);
    VULKAN_PROTOTYPE(vkGetDeviceQueue);

    VULKAN_PROTOTYPE(vkCreateRenderPass);
    VULKAN_PROTOTYPE(vkDestroyRenderPass);

    VULKAN_PROTOTYPE(vkCreatePipelineLayout);
    VULKAN_PROTOTYPE(vkDestroyPipelineLayout);
    VULKAN_PROTOTYPE(vkCreateGraphicsPipelines);
    VULKAN_PROTOTYPE(vkDestroyPipeline);

    VULKAN_PROTOTYPE(vkCreateShaderModule);
    VULKAN_PROTOTYPE(vkDestroyShaderModule);

    VULKAN_PROTOTYPE(vkCreateFramebuffer);
    VULKAN_PROTOTYPE(vkDestroyFramebuffer);

    VULKAN_PROTOTYPE(vkCreateCommandPool);
    VULKAN_PROTOTYPE(vkDestroyCommandPool);

    VULKAN_PROTOTYPE(vkAllocateCommandBuffers);
    VULKAN_PROTOTYPE(vkFreeCommandBuffers);

    VULKAN_PROTOTYPE(vkCreateFence);
    VULKAN_PROTOTYPE(vkCreateSemaphore);
    VULKAN_PROTOTYPE(vkDestroyFence);
    VULKAN_PROTOTYPE(vkDestroySemaphore);

    VULKAN_PROTOTYPE(vkCreateDescriptorPool);
    VULKAN_PROTOTYPE(vkDestroyDescriptorPool);
    VULKAN_PROTOTYPE(vkCreateDescriptorSetLayout);
    VULKAN_PROTOTYPE(vkDestroyDescriptorSetLayout);

    VULKAN_PROTOTYPE(vkAllocateDescriptorSets);
    VULKAN_PROTOTYPE(vkUpdateDescriptorSets);

    VULKAN_PROTOTYPE(vkCreateImageView);
    VULKAN_PROTOTYPE(vkDestroyImageView);

    VULKAN_PROTOTYPE(vkCreateSampler);
    VULKAN_PROTOTYPE(vkDestroySampler);

    VULKAN_PROTOTYPE(vkBeginCommandBuffer);
    VULKAN_PROTOTYPE(vkEndCommandBuffer);
    VULKAN_PROTOTYPE(vkQueueSubmit);
    VULKAN_PROTOTYPE(vkWaitForFences);
    VULKAN_PROTOTYPE(vkResetFences);
    VULKAN_PROTOTYPE(vkResetCommandPool);

    VULKAN_PROTOTYPE(vkCmdCopyBuffer);
    VULKAN_PROTOTYPE(vkCmdCopyBufferToImage);
    VULKAN_PROTOTYPE(vkCmdPipelineBarrier);
    VULKAN_PROTOTYPE(vkCmdBlitImage);
    VULKAN_PROTOTYPE(vkCmdBindPipeline);
    VULKAN_PROTOTYPE(vkCmdBindDescriptorSets);
    VULKAN_PROTOTYPE(vkCmdPushConstants);
    VULKAN_PROTOTYPE(vkCmdBeginRenderPass);
    VULKAN_PROTOTYPE(vkCmdBindVertexBuffers);
    VULKAN_PROTOTYPE(vkCmdBindIndexBuffer);
    VULKAN_PROTOTYPE(vkCmdDrawIndexed);
    VULKAN_PROTOTYPE(vkCmdEndRenderPass);
    VULKAN_PROTOTYPE(vkQueuePresentKHR);

protected:
    VkInstance instance;
};

#undef VULKAN_LOAD

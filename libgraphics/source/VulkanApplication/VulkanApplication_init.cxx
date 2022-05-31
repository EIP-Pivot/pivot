#include "pivot/graphics/VulkanApplication.hxx"

#include "pivot/graphics/DebugMacros.hxx"
#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"
#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"
#include "pivot/graphics/QueueFamilyIndices.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/graphics/vk_init.hxx"
#include "pivot/graphics/vk_utils.hxx"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <numeric>

namespace pivot::graphics
{

void VulkanApplication::createRenderPass()
{
    DEBUG_FUNCTION
    vk::AttachmentDescription colorAttachmentResolve{
        .format = swapchain.getSwapchainFormat(),
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eDontCare,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };
    const auto depthFormat =
        vk_utils::findSupportedFormat(physical_device,
                                      {
                                          vk::Format::eD32Sfloat,
                                          vk::Format::eD32SfloatS8Uint,
                                          vk::Format::eD24UnormS8Uint,
                                      },
                                      vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

    renderPass.addAttachement(VulkanRenderPass::Color, swapchain.getSwapchainFormat(), maxMsaaSample,
                              vk::ImageLayout::eColorAttachmentOptimal);
    renderPass.addAttachement(VulkanRenderPass::Depth, depthFormat, maxMsaaSample,
                              vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal);
    renderPass.addAttachement(VulkanRenderPass::Resolve, colorAttachmentResolve);
    renderPass.build(device);

    vk_debug::setObjectName(device, renderPass.getRenderPass(), "Main Render Pass");
    swapchainDeletionQueue.push([&] { renderPass.destroy(device); });
}

void VulkanApplication::createFramebuffers()
{
    DEBUG_FUNCTION
    swapChainFramebuffers.resize(swapchain.nbOfImage());
    for (size_t i = 0; i < swapchain.nbOfImage(); i++) {
        std::array<vk::ImageView, 3> attachments = {colorImage.imageView, depthResources.imageView,
                                                    swapchain.getSwapchainImageView(i)};

        vk::FramebufferCreateInfo framebufferInfo{
            .renderPass = renderPass.getRenderPass(),
            .width = swapchain.getSwapchainExtent().width,
            .height = swapchain.getSwapchainExtent().height,
            .layers = 1,
        };
        framebufferInfo.setAttachments(attachments);

        swapChainFramebuffers.at(i) = device.createFramebuffer(framebufferInfo);
        vk_debug::setObjectName(device, swapChainFramebuffers.at(i), "FrameBuffer nb " + std::to_string(i));
    }
    swapchainDeletionQueue.push([&] {
        for (auto &framebuffer: swapChainFramebuffers) { device.destroy(framebuffer); }
    });
}

void VulkanApplication::createCommandPool()
{
    DEBUG_FUNCTION
    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queueIndices.graphicsFamily.value(),
    };
    commandPool = device.createCommandPool(poolInfo);
    vk_debug::setObjectName(device, commandPool, "Main Command Pool");
    mainDeletionQueue.push([&] { device.destroy(commandPool); });
}

void VulkanApplication::createDepthResources()
{
    DEBUG_FUNCTION
    vk::Format depthFormat = pivot::graphics::vk_utils::findSupportedFormat(
        physical_device, {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = depthFormat,
        .extent = swapchain.getSwapchainExtent3D(),
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = maxMsaaSample,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };
    vma::AllocationCreateInfo allocInfo;
    allocInfo.setUsage(vma::MemoryUsage::eGpuOnly);
    allocInfo.setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);
    depthResources = allocator.createImage(imageInfo, allocInfo);

    auto createInfo = vk_init::populateVkImageViewCreateInfo(depthResources.image, depthFormat);
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthResources.createImageView(device, createInfo);
    transitionLayout(depthResources, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    vk_debug::setObjectName(device, depthResources.image, "Depth Image");
    vk_debug::setObjectName(device, depthResources.imageView, "Depth Image view");
    swapchainDeletionQueue.push([&] {
        device.destroyImageView(depthResources.imageView);
        allocator.destroyImage(depthResources);
    });
}

void VulkanApplication::createColorResources()
{
    DEBUG_FUNCTION
    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = swapchain.getSwapchainFormat(),
        .extent = swapchain.getSwapchainExtent3D(),
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = maxMsaaSample,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };
    vma::AllocationCreateInfo allocInfo{};
    allocInfo.setUsage(vma::MemoryUsage::eGpuOnly);
    allocInfo.setFlags(vma::AllocationCreateFlagBits::eDedicatedMemory);

    colorImage = allocator.createImage(imageInfo, allocInfo);
    colorImage.createImageView(device);
    vk_debug::setObjectName(device, colorImage.image, "Color Image");
    vk_debug::setObjectName(device, colorImage.imageView, "Color Image view");
    swapchainDeletionQueue.push([&] {
        device.destroyImageView(colorImage.imageView);
        allocator.destroyImage(colorImage);
    });
}

}    // namespace pivot::graphics

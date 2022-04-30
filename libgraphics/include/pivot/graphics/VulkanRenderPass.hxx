#pragma once

#include <span>
#include <vector>

#include "pivot/graphics/VulkanBase.hxx"

namespace pivot::graphics
{

/// Wrapper for constructing a Vulkan Render Pass
class VulkanRenderPass
{
public:
    /// Exception of VulkanRenderPass
    LOGIC_ERROR(RenderPass);

    /// Stage of VulkanRenderPass Attachment
    enum Attachment {
        Color,
        Depth,
        Resolve,
    };

public:
    VulkanRenderPass();
    ~VulkanRenderPass();

    /// @brief add an Attachement to the Render Pass
    ///
    /// Short-hand for :
    /// @code{.cpp}
    /// vk::AttachmentDescription{
    ///     .format = format,
    ///     .samples = sample,
    ///     .loadOp = vk::AttachmentLoadOp::eClear,
    ///     .storeOp = vk::AttachmentStoreOp::eDontCare,
    ///     .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
    ///     .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
    ///     .initialLayout = vk::ImageLayout::eUndefined,
    ///     .finalLayout = finalLayout,
    /// }
    /// @endcode
    unsigned addAttachement(const Attachment &att, const vk::Format &format, const vk::SampleCountFlagBits &sample,
                            const vk::ImageLayout &finalLayout);
    /// add an Attachment to the Render Pass
    unsigned addAttachement(const Attachment &att, const vk::AttachmentDescription &description);

    /// Create the Render Pass with the provided attachement
    void build(vk::Device &device);
    /// Destroy the Render Pass
    void destroy(vk::Device &device);

    /// Return the Vulkan Render Pass
    vk::RenderPass &getRenderPass() noexcept { return renderPass; }

private:
    void createRenderPass(vk::Device &device);

private:
    vk::RenderPass renderPass;
    std::vector<std::pair<Attachment, vk::AttachmentDescription>> renderPassAttachment;
};

}    // namespace pivot::graphics

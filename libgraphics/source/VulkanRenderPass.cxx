#include "pivot/graphics/VulkanRenderPass.hxx"
#include "pivot/graphics/vk_debug.hxx"
#include "pivot/pivot.hxx"

namespace pivot::graphics
{

VulkanRenderPass::VulkanRenderPass() {}

VulkanRenderPass::~VulkanRenderPass() {}

unsigned VulkanRenderPass::addAttachement(const Attachment &att, const vk::Format &format,
                                          const vk::SampleCountFlagBits &sample, const vk::ImageLayout &finalLayout)
{
    DEBUG_FUNCTION();
    return addAttachement(att, vk::AttachmentDescription{
                                   .format = format,
                                   .samples = sample,
                                   .loadOp = vk::AttachmentLoadOp::eClear,
                                   .storeOp = vk::AttachmentStoreOp::eDontCare,
                                   .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                   .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                                   .initialLayout = vk::ImageLayout::eUndefined,
                                   .finalLayout = finalLayout,
                               });
}

unsigned VulkanRenderPass::addAttachement(const Attachment &att, const vk::AttachmentDescription &description)
{
    DEBUG_FUNCTION();
    renderPassAttachment.push_back(std::pair(att, description));
    return renderPassAttachment.size();
}

void VulkanRenderPass::build(vk::Device &device)
{
    DEBUG_FUNCTION();
    createRenderPass(device);
}

void VulkanRenderPass::destroy(vk::Device &device)
{
    DEBUG_FUNCTION();
    renderPassAttachment.clear();
    device.destroyRenderPass(renderPass);
}

void VulkanRenderPass::createRenderPass(vk::Device &device)
{
    DEBUG_FUNCTION();
    std::vector<vk::AttachmentDescription> attachment;
    std::unordered_map<Attachment, vk::AttachmentReference> attachement_refs;

    for (const auto &[pos, attach]: renderPassAttachment) {
        if (attachement_refs.contains(pos)) throw RenderPassError("Multiple render pass stage");
        if (pos == Attachment::Resolve && attach.finalLayout != vk::ImageLayout::ePresentSrcKHR)
            throw RenderPassError("Resolve attachment is not in the right final layout");

        attachement_refs.emplace(pos, vk::AttachmentReference{
                                          .attachment = static_cast<uint32_t>(attachment.size()),
                                          .layout = vk::ImageLayout::eUndefined,
                                      });
        attachment.push_back(attach);
    }

    vk::SubpassDescription subpass{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
    };
    for (auto &[att, ref]: attachement_refs) {
        switch (att) {
            case Attachment::Color: {
                ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
                subpass.pColorAttachments = &ref;
            } break;
            case Attachment::Depth: {
                ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                subpass.pDepthStencilAttachment = &ref;
            } break;
            case Attachment::Resolve: {
                ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
                subpass.pResolveAttachments = &ref;
            } break;
            default: throw RenderPassError("Attachement value error");
        }
    }

    vk::SubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .dstStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .srcAccessMask = vk::AccessFlagBits::eNoneKHR,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
    };

    vk::RenderPassCreateInfo renderPassInfo{
        .attachmentCount = static_cast<uint32_t>(attachment.size()),
        .pAttachments = attachment.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };
    renderPass = device.createRenderPass(renderPassInfo);
    vk_debug::setObjectName(device, renderPass, "Main renderPass");
}

}    // namespace pivot::graphics

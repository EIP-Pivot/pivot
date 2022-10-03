#include "pivot/graphics/Renderer/ImGuiRenderer.hxx"

#include "pivot/graphics/vk_debug.hxx"
#include "pivot/pivot.hxx"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

namespace pivot::graphics
{

ImGuiRenderer::ImGuiRenderer(StorageUtils &utils): IGraphicsRenderer(utils) {}
ImGuiRenderer::~ImGuiRenderer() {}

bool ImGuiRenderer::onInit(const vk::Extent2D &, VulkanBase &base_ref, const ResolverDispatcher &, vk::RenderPass &pass)
{
    DEBUG_FUNCTION();

    IMGUI_CHECKVERSION();
    ImGui_ImplVulkan_LoadFunctions(
        [](const char *function_name, void *user) {
            auto loader = reinterpret_cast<vk::DynamicLoader *>(user);
            return loader->getProcAddress<PFN_vkVoidFunction>(function_name);
        },
        &base_ref.loader);
    createDescriptorPool(base_ref.device);
    createImGuiContext(base_ref, pass);
    return true;
}

void ImGuiRenderer::onStop(VulkanBase &base_ref)
{
    DEBUG_FUNCTION();

    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    if (pool) base_ref.device.destroyDescriptorPool(pool);
}

bool ImGuiRenderer::onDraw(const RenderingContext &, const CameraData &, ResolverDispatcher &, vk::CommandBuffer &cmd)
{
    PROFILE_FUNCTION();

    vk_debug::beginRegion(cmd, "Imgui Commands", {1.f, 0.f, 0.f, 1.f});
    if (auto imguiData = ImGui::GetDrawData(); imguiData != nullptr) {
        ImGui_ImplVulkan_RenderDrawData(imguiData, cmd);
    }
    vk_debug::endRegion(cmd);
    return true;
}

void ImGuiRenderer::createDescriptorPool(vk::Device &device)
{
    DEBUG_FUNCTION();

    const vk::DescriptorPoolSize pool_sizes[]{
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000},
    };

    const vk::DescriptorPoolCreateInfo pool_info{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000,
        .poolSizeCount = std::size(pool_sizes),
        .pPoolSizes = pool_sizes,
    };

    pool = device.createDescriptorPool(pool_info);
    vk_debug::setObjectName(device, pool, "ImGui Descriptor Pool");
}

void ImGuiRenderer::createImGuiContext(VulkanBase &base_ref, vk::RenderPass &pass)
{
    DEBUG_FUNCTION();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGui_ImplGlfw_InitForVulkan(base_ref.window.getWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = base_ref.instance;
    init_info.PhysicalDevice = base_ref.physical_device;
    init_info.Device = base_ref.device;
    init_info.QueueFamily = base_ref.queueIndices.graphicsFamily.value();
    init_info.Queue = base_ref.graphicsQueue;
    init_info.DescriptorPool = pool;
    init_info.MinImageCount = PIVOT_MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = PIVOT_MAX_FRAMES_IN_FLIGHT;
    init_info.MSAASamples = static_cast<VkSampleCountFlagBits>(base_ref.maxMsaaSample);
    init_info.CheckVkResultFn = vk_utils::vk_try;

    ImGui_ImplVulkan_Init(&init_info, pass);

    base_ref.immediateCommand([&](vk::CommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(cmd); },
                              vk::QueueFlagBits::eGraphics);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

}    // namespace pivot::graphics

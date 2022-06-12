#pragma once

#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/PipelineBuilders/ComputePipelineBuilder.hxx"
#include "pivot/graphics/PipelineBuilders/GraphicsPipelineBuilder.hxx"
#include "pivot/graphics/VulkanBase.hxx"

namespace pivot::graphics
{
/// @class PipelineStorage
/// @brief Store vulkan pipeline, ready to be used in render
class PipelineStorage
{
public:
    /// @brief Constructor
    PipelineStorage(VulkanBase &base);
    PipelineStorage(const PipelineStorage &) = delete;
    /// @brief Destructor
    ~PipelineStorage();

    /// @brief Initalize the PipelineCache
    void init();
    /// @brief Destroy all the stored pipeline
    void destroy();

    /// @brief Store a new pipeline as given name
    void newGraphicsPipeline(const std::string &name, const GraphicsPipelineBuilder &builder);
    /// @copydoc newGraphicsPipeline
    void newGraphicsPipeline(const std::string &name, vk::Pipeline pipeline);
    /// @copydoc newGraphicsPipeline
    void newComputePipeline(const std::string &name, const ComputePipelineBuilder &builder);
    /// @copydoc newGraphicsPipeline
    void newComputePipeline(const std::string &name, vk::Pipeline pipeline);

    /// @brief Destroy the pipeline
    void removePipeline(const std::string &name);
    /// @brief Recover a Graphics pipeline with id
    vk::Pipeline &getGraphics(const std::string &id)
    {
        if (id.empty() || bForceDefault) return getDefault();
        auto iter = graphicsStorage.find(id);
        if (iter == graphicsStorage.end()) {
            logger.warn("Pipeline Storage") << "Pipeline not found, using default one :" << getDefaultName();
            return getDefault();
        }
        return iter->second;
    }
    /// @brief Recovery a Compute pipeline with id
    vk::Pipeline &getCompute(const std::string &id) { return computeStorage.at(id); }

    /// @brief get all available pipeline id
    auto getNames() const
    {
        return this->graphicsStorage | std::views::transform([](const auto &pipeline) { return pipeline.first; });
    }

    /// @brief Get the default graphics pipeline
    vk::Pipeline &getDefault() { return graphicsStorage.at(getDefaultName()); }

    /// @brief Get the default pipeline name
    constexpr const std::string &getDefaultName() const noexcept { return defaultPipeline; }

    /// @brief Set the name of the default pipeline
    void setDefault(const std::string &id, bool bForce = false) noexcept
    {
        defaultPipeline = id;
        bForceDefault = bForce;
    }

    /// Return the pipeline cache
    vk::PipelineCache getCache() const noexcept { return pipelineCache; }

private:
    VulkanBase &base_ref;
    vk::PipelineCache pipelineCache;
    std::unordered_map<std::string, vk::Pipeline> graphicsStorage;
    std::unordered_map<std::string, vk::Pipeline> computeStorage;

    std::string defaultPipeline;
    bool bForceDefault = false;
};

}    // namespace pivot::graphics

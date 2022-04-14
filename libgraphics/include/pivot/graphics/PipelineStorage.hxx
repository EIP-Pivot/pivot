#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "pivot/graphics/VulkanBase.hxx"
#include "pivot/graphics/interface/IPipelineBuilder.hxx"

namespace pivot::graphics
{
/// @class PipelineStorage
/// @brief Store vulkan pipeline, ready to be used in render
class PipelineStorage
{
public:
    /// @brief Constructor
    PipelineStorage(VulkanBase &base);
    /// @brief Destructor
    ~PipelineStorage();

    /// @brief Initalize the PipelineCache
    void init();
    /// @brief Destroy all the stored pipeline
    void destroy();

    /// @brief Store a new pipeline as given name
    void newPipeline(const std::string &name, const internal::IPipelineBuilder &builder);
    /// @brief Destroy the pipeline
    void removePipeline(const std::string &name);
    /// @brief Recover the pipeline at id
    vk::Pipeline &get(const std::string &id)
    {
        if (id.empty() || bForceDefault) return getDefault();
        return storage.at(id);
    }

    /// @brief get all available pipeline id
    std::vector<std::string> getNames() const;

    /// @brief Get the default pipeline
    vk::Pipeline &getDefault() { return get(defaultPipeline); }

    /// @brief Get the default pipeline name
    const std::string &getDefaultName() const noexcept { return defaultPipeline; }

    /// @brief Set the name of the default pipeline
    void setDefault(const std::string &id, bool bForce = false)
    {
        defaultPipeline = id;
        bForceDefault = bForce;
    }

private:
    VulkanBase &base_ref;
    vk::PipelineCache pipelineCache;
    std::unordered_map<std::string, vk::Pipeline> storage;

    std::string defaultPipeline;
    bool bForceDefault = false;
};

}    // namespace pivot::graphics

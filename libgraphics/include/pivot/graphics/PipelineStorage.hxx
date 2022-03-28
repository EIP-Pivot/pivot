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
    inline vk::Pipeline &get(const std::string &id)
    {
        if (id.empty()) return getDefault();
        return storage.at(id);
    }

    /// @brief Get the default pipeline
    inline vk::Pipeline &getDefault() { return get(defaultPipeline); }
    /// @brief Set the name of the default pipeline
    inline void setDefault(const std::string &id) { defaultPipeline = id; }

private:
    VulkanBase &base_ref;
    vk::PipelineCache pipelineCache;
    std::string defaultPipeline;
    std::unordered_map<std::string, vk::Pipeline> storage;
};

}    // namespace pivot::graphics
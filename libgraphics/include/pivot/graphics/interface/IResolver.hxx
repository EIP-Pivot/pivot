#pragma once

#include <vulkan/vulkan.hpp>

#include "pivot/graphics/AssetStorage/AssetStorage.hxx"
#include "pivot/graphics/DescriptorAllocator/DescriptorBuilder.hxx"
#include "pivot/graphics/VulkanBase.hxx"

#include "pivot/graphics/types/Light.hxx"
#include "pivot/graphics/types/RenderObject.hxx"
#include "pivot/graphics/types/UniformBufferObject.hxx"

namespace pivot::graphics
{

template <typename T>
/// Wrapper for the T/bool pair returned from the ECS
struct Object {
    /// The array of component
    std::reference_wrapper<const std::vector<T>> objects;
    /// Indicate which entities have the component
    std::reference_wrapper<const std::vector<bool>> exist;
};

/// Informations needed to draw a scene. Including all objects and al lights
struct DrawSceneInformation {
    /// @cond
    Object<RenderObject> renderObjects;
    Object<PointLight> pointLight;
    Object<DirectionalLight> directionalLight;
    Object<SpotLight> spotLight;
    Object<Transform> transform;
    ///@endcond
};

/// Hold both a layout and the actual set
struct DescriptorPair {
    /// @brief Hold the descriptor set layout.
    ///
    /// If the descriptor size is equal to zero, set will be nullptr.
    vk::DescriptorSetLayout layout;
    /// @brief Hold the descriptor set layout.
    ///
    /// Can be nullptr if the descriptor is empty.
    vk::DescriptorSet set;
};

class IResolver
{
public:
    virtual bool initialize(VulkanBase &base, const AssetStorage &assetStorage, DescriptorBuilder &builder) = 0;
    virtual bool destroy(VulkanBase &base) = 0;

    virtual DescriptorPair getManagedDescriptorSet() const = 0;

    virtual bool prepareForDraw(const DrawSceneInformation &information) = 0;
};

}    // namespace pivot::graphics

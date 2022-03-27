#pragma once

#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/Event/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/Systems/index.hxx>

#include <pivot/graphics/VulkanApplication.hxx>

namespace pivot
{
class Engine
{
public:
    Engine();

protected:
    pivot::ecs::component::Index m_component_index;
    pivot::ecs::event::Index m_event_index;
    pivot::ecs::systems::Index m_system_index;
    pivot::ecs::SceneManager m_scene_manager;
    pivot::graphics::VulkanApplication m_vulkan_application;
};
}    // namespace pivot

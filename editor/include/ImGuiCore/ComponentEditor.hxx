#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include <ImGuiCore/ValueInput.hxx>
#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/ecs/Core/Component/index.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/ecs/Core/types.hxx>
#include <pivot/graphics/types/UniformBufferObject.hxx>

class ComponentEditor
{
public:
    ComponentEditor(const pivot::ecs::component::Index &index, pivot::ecs::CurrentScene scene)
        : m_index(index), m_scene(scene), m_value_input(scene)
    {
    }

    void create(Entity entity);
    void create();

private:
    void addComponent(const pivot::ecs::component::Description &description);
    void displayComponent();
    void displayName();
    void deleteComponent(pivot::ecs::component::ComponentRef ref);
    void createPopUp();

private:
    Entity currentEntity;
    const pivot::ecs::component::Index &m_index;
    pivot::ecs::CurrentScene m_scene;
    ValueInput m_value_input;
};

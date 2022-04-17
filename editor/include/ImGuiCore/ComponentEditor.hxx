#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

#include <pivot/ecs/Core/types.hxx>

#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/graphics/types/UniformBufferObject.hxx>

#include <pivot/ecs/Core/Component/index.hxx>

#include <iostream>

#include <pivot/ecs/Core/SceneManager.hxx>

class ComponentEditor
{
public:
    ComponentEditor(const pivot::ecs::component::Index &index, pivot::ecs::CurrentScene scene)
        : m_index(index), m_scene(scene)
    {
    }

    void create(Entity entity);
    void create();

private:
    void addComponent(const pivot::ecs::component::Description &description);
    void displayComponent();
    void createPopUp();

private:
    Entity currentEntity;
    glm::vec3 matrixTranslation{0.f};
    glm::vec3 matrixRotation{0.f};
    glm::vec3 matrixScale{1.f};
    std::array<std::string, 8> textures = {"rouge", "vert", "bleu", "cyan", "orange", "jaune", "blanc", "violet"};
    std::array<std::string, 2> models = {"cube", "plane"};

    const pivot::ecs::component::Index &m_index;
    pivot::ecs::CurrentScene m_scene;
};

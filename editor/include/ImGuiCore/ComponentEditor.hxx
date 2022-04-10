#pragma once

#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

#include <pivot/ecs/Core/types.hxx>

#include <pivot/components/RenderObject.hxx>
#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>
#include <pivot/graphics/types/UniformBufferObject.hxx>

#include <pivot/ecs/Core/Component/index.hxx>

#include <iostream>

#include <pivot/ecs/Core/SceneManager.hxx>

using ObjectVector = std::vector<std::reference_wrapper<const RenderObject>>;

// TODO: Remove using namespace
using namespace pivot::ecs::component;

class ComponentEditor
{
public:
    ComponentEditor(const Index &index, pivot::ecs::SceneManager &sceneManager)
        : m_index(index), m_sceneManager(sceneManager)
    {
    }

    void create(Entity entity);
    void create();

    void setVectorObject(LevelId scene);
    std::unordered_map<LevelId, ObjectVector> &getVectorObject();
    ObjectVector getObject();

private:
    void addComponent(const Description &description);
    void displayComponent();
    void createPopUp();

private:
    Entity currentEntity;
    glm::vec3 matrixTranslation{0.f};
    glm::vec3 matrixRotation{0.f};
    glm::vec3 matrixScale{1.f};
    std::unordered_map<LevelId, ObjectVector> sceneObject;
    std::array<std::string, 8> textures = {"rouge", "vert", "bleu", "cyan", "orange", "jaune", "blanc", "violet"};
    std::array<std::string, 2> models = {"cube", "plane"};

    const Index &m_index;
    pivot::ecs::SceneManager &m_sceneManager;
};

#include "pivot/ecs/Core/Scene.hxx"
#include <pivot/ecs/Core/Component/index.hxx>
#include "pivot/ecs/Components/Gravity.hxx"
#include "pivot/ecs/Components/RigidBody.hxx"
#include "pivot/graphics/types/RenderObject.hxx"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace pivot::ecs;

Scene::Scene(std::string sceneName)
    : name(sceneName),

      mComponentManager(std::make_unique<component::Manager>()),
      mEntityManager(std::make_unique<EntityManager>()),
      mEventManager(std::make_unique<EventManager>()),
      mSystemManager(std::make_unique<SystemManager>()),
      mCurrentCamera(0)
{
    auto &global_index = component::GlobalIndex::getSingleton();
    for (auto &[name, description]: global_index) {
        auto componentId = mComponentManager->RegisterComponent(description);
        if (name == "Tag") { mTagId = componentId; }
    }
}

std::string Scene::getName() { return name; }

Entity Scene::CreateEntity()
{
    Entity newEntity = mEntityManager->CreateEntity();
    mComponentManager->AddComponent(newEntity, std::make_any<Tag>("Entity " + std::to_string(newEntity)), mTagId);
    return newEntity;
}

Entity Scene::CreateEntity(std::string newName)
{
    Entity newEntity = mEntityManager->CreateEntity();
    mComponentManager->AddComponent(newEntity, std::make_any<Tag>(newName), mTagId);
    return newEntity;
}

std::unordered_map<Entity, Signature> Scene::getEntities() { return mEntityManager->getEntities(); }

void Scene::DestroyEntity(Entity entity)
{
    mEntityManager->DestroyEntity(entity);
    mComponentManager->EntityDestroyed(entity);
    mSystemManager->EntityDestroyed(entity);
}

Signature Scene::getSignature(Entity entity) { return mEntityManager->GetSignature(entity); }

std::string Scene::getEntityName(Entity entity)
{
    return std::any_cast<Tag>(mComponentManager->GetComponent(entity, mTagId).value()).name;
}

uint32_t Scene::getLivingEntityCount() { return mEntityManager->getLivingEntityCount(); }

// std::unordered_map<const char *, ComponentType> Scene::getComponentsTypes()
// {
//     return mComponentManager->getComponentsTypes();
// }

void Scene::Update(float dt)
{
    for (std::shared_ptr<System> system: mSystems) { system->Update(dt); }
}

void Scene::AddEventListener(EventId eventId, std::function<void(Event &)> const &listener)
{
    mEventManager->AddListener(eventId, listener);
}

void Scene::SendEvent(Event &event) { mEventManager->SendEvent(event); }

void Scene::SendEvent(EventId eventId) { mEventManager->SendEvent(eventId); }

void Scene::setCamera(std::uint16_t camera) { mCurrentCamera = camera; }

void Scene::addCamera(Entity camera) { mCamera.push_back(camera); }

void Scene::switchCamera() { mCurrentCamera = (mCurrentCamera + 1) % mCamera.size(); }

Camera &Scene::getCamera()
{
    if (mCamera.size() == 0) throw EcsException("No camera set");
    throw std::logic_error("Unimplemented");
    // return mComponentManager->GetComponent<Camera>(mCamera[mCurrentCamera]);
}

std::vector<Entity> &Scene::getCameras() { return mCamera; }

pivot::ecs::component::Manager &Scene::getComponentManager() { return *this->mComponentManager.get(); }

/// Get the component manager (const)
const pivot::ecs::component::Manager &Scene::getComponentManager() const { return *this->mComponentManager.get(); }

void Scene::save()
{
    if (!std::filesystem::exists("scene")) { std::filesystem::create_directory("scene"); }
    std::ofstream out("scene/" + name + ".json");
    nlohmann::json j;
    j["name"] = name;
    for (auto const &[entity, _]: getEntities()) {
        j["entity"][entity]["tag"] = GetComponent<Tag>(entity).name;
        if (hasComponent<RigidBody>(entity)) {
            j["entity"][entity]["RigidBody"] = {{"acceleration",
                                                 {{"x", GetComponent<RigidBody>(entity).acceleration.x},
                                                  {"y", GetComponent<RigidBody>(entity).acceleration.y},
                                                  {"z", GetComponent<RigidBody>(entity).acceleration.z}}},
                                                {"velocity",
                                                 {{"x", GetComponent<RigidBody>(entity).velocity.x},
                                                  {"y", GetComponent<RigidBody>(entity).velocity.y},
                                                  {"z", GetComponent<RigidBody>(entity).velocity.z}}}};
        }
        if (hasComponent<Gravity>(entity)) {
            j["entity"][entity]["Gravity"] = {{"force",
                                               {{"x", GetComponent<Gravity>(entity).force.x},
                                                {"y", GetComponent<Gravity>(entity).force.y},
                                                {"z", GetComponent<Gravity>(entity).force.z}}}};
        }
        if (hasComponent<RenderObject>(entity)) {
            j["entity"][entity]["RenderObject"] = {
                {"meshID", GetComponent<RenderObject>(entity).meshID},
                {"objectInformation",
                 {{"textureIndex", GetComponent<RenderObject>(entity).objectInformation.textureIndex},
                  {"materialIndex", GetComponent<RenderObject>(entity).objectInformation.materialIndex}}}};
        }
    }

    out << std::setw(4) << j << std::endl;
    out.close();
}

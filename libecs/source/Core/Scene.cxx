#include "pivot/ecs/Core/Scene.hxx"
#include <pivot/ecs/Core/Component/index.hxx>

#include <nlohmann/json.hpp>

using namespace pivot::ecs;

Scene::Scene(std::string sceneName)
    : name(sceneName),
      mSystemManager(mComponentManager, mEntityManager),
      mEventManager(mSystemManager),
      mCurrentCamera(0)
{
    auto &global_index = component::GlobalIndex::getSingleton();
    for (auto &[name, description]: global_index) {
        auto componentId = mComponentManager.RegisterComponent(description);
        if (name == "Tag") { mTagId = componentId; }
    }
}

std::string Scene::getName() { return name; }

Entity Scene::CreateEntity()
{
    Entity newEntity = mEntityManager.CreateEntity();
    mComponentManager.AddComponent(newEntity, "Entity " + std::to_string(newEntity), mTagId);
    return newEntity;
}

Entity Scene::CreateEntity(std::string newName)
{
    Entity newEntity = mEntityManager.CreateEntity();
    mComponentManager.AddComponent(newEntity, data::Value{data::Record{{"name", newName}}}, mTagId);
    return newEntity;
}

std::unordered_map<Entity, Signature> Scene::getEntities() { return mEntityManager.getEntities(); }

void Scene::DestroyEntity(Entity entity)
{
    mEntityManager.DestroyEntity(entity);
    mComponentManager.EntityDestroyed(entity);
}

Signature Scene::getSignature(Entity entity) { return mEntityManager.GetSignature(entity); }

std::string Scene::getEntityName(Entity entity)
{
    return std::get<std::string>(
        std::get<data::Record>(mComponentManager.GetComponent(entity, mTagId).value()).at("name"));
}

uint32_t Scene::getLivingEntityCount() { return mEntityManager.getLivingEntityCount(); }

void Scene::setCamera(std::uint16_t camera) { mCurrentCamera = camera; }

void Scene::addCamera(Entity camera) { mCamera.push_back(camera); }

void Scene::switchCamera() { mCurrentCamera = (mCurrentCamera + 1) % mCamera.size(); }

Camera &Scene::getCamera()
{
    if (mCamera.size() == 0) throw EcsException("No camera set");
    throw std::logic_error("Unimplemented");
    // return mComponentManager.GetComponent<Camera>(mCamera[mCurrentCamera]);
}

std::vector<Entity> &Scene::getCameras() { return mCamera; }

pivot::ecs::component::Manager &Scene::getComponentManager() { return mComponentManager; }

/// Get the component manager (const)
const pivot::ecs::component::Manager &Scene::getComponentManager() const { return mComponentManager; }

pivot::ecs::systems::Manager &Scene::getSystemManager() { return mSystemManager; }

const pivot::ecs::systems::Manager &Scene::getSystemManager() const { return mSystemManager; }

pivot::ecs::event::Manager &Scene::getEventManager() { return mEventManager; }

const pivot::ecs::event::Manager &Scene::getEventManager() const { return mEventManager; }

void Scene::saveScene()
{
    if (!std::filesystem::exists("scene")) { std::filesystem::create_directory("scene"); }
    std::ofstream out("scenes/" + name + ".json");
    nlohmann::json j;
    j["name"] = name;
    // for (auto const &[entity, _]: getEntities()) {
    //     j["entity"][entity]["tag"] = GetComponent<Tag>(entity).name;
    //     if (hasComponent<RigidBody>(entity)) {
    //         j["entity"][entity]["RigidBody"] = {{"acceleration",
    //                                              {{"x", GetComponent<RigidBody>(entity).acceleration.x},
    //                                               {"y", GetComponent<RigidBody>(entity).acceleration.y},
    //                                               {"z", GetComponent<RigidBody>(entity).acceleration.z}}},
    //                                             {"velocity",
    //                                              {{"x", GetComponent<RigidBody>(entity).velocity.x},
    //                                               {"y", GetComponent<RigidBody>(entity).velocity.y},
    //                                               {"z", GetComponent<RigidBody>(entity).velocity.z}}}};
    //     }
    //     if (hasComponent<Gravity>(entity)) {
    //         j["entity"][entity]["Gravity"] = {{"force",
    //                                            {{"x", GetComponent<Gravity>(entity).force.x},
    //                                             {"y", GetComponent<Gravity>(entity).force.y},
    //                                             {"z", GetComponent<Gravity>(entity).force.z}}}};
    //     }
    //     if (hasComponent<RenderObject>(entity)) {
    //         j["entity"][entity]["RenderObject"] = {
    //             {"meshID", GetComponent<RenderObject>(entity).meshID},
    //             {"objectInformation",
    //              {{"textureIndex", GetComponent<RenderObject>(entity).objectInformation.textureIndex},
    //               {"materialIndex", GetComponent<RenderObject>(entity).objectInformation.materialIndex}}}};
    //     }
    // }

    out << std::setw(4) << j << std::endl;
    out.close();
}
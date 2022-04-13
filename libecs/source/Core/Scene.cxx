#include "pivot/ecs/Core/Scene.hxx"
#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs;

Scene::Scene(std::string sceneName)
    : name(sceneName),
      mSystemManager(mComponentManager, mEntityManager),
      mEventManager(mSystemManager),
      mCurrentCamera(0)
{
    mTagId = mComponentManager.RegisterComponent(Tag::description);
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

EntityManager &Scene::getEntityManager() { return mEntityManager; }

const EntityManager &Scene::getEntityManager() const { return mEntityManager; }

Scene Scene::load(const nlohmann::json &obj, const pivot::ecs::component::Index &cIndex,
                  const pivot::ecs::systems::Index &sIndex)
{
    Scene scene(obj["name"].dump());
    auto &componentManager = scene.getComponentManager();
    auto &entityManager = scene.getEntityManager();
    auto &systemManager = scene.getSystemManager();

    for (auto entities: obj["components"]) {
        auto entity = entityManager.CreateEntity();
        for (auto component = entities.begin(); component != entities.end(); ++component) {
            if (!componentManager.GetComponentId(component.key())) {
                auto description = cIndex.getDescription(component.key());
                if (!description.has_value()) throw std::runtime_error("Unknown Component " + component.key());
                componentManager.RegisterComponent(description.value());
            }
            auto componentId = componentManager.GetComponentId(component.key());
            auto componentValue = component.value().get<pivot::ecs::data::Value>();
            componentManager.AddComponent(entity, componentValue, componentId.value());
        }
    }
    for (auto systems: obj["systems"]) {
        auto description = sIndex.getDescription(systems.get<std::string>());
        if (!description.has_value()) throw std::runtime_error("Unknown System " + systems.get<std::string>());
        systemManager.useSystem(description.value());
    }
    return scene;
}
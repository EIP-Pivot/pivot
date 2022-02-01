#include "pivot/ecs/Core/Scene.hxx"
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs;

Scene::Scene(std::string sceneName)
    : name(sceneName),
      mComponentManager(std::make_unique<component::Manager>()),
      mEntityManager(std::make_unique<EntityManager>()),
      mSystemManager(std::make_unique<systems::Manager>(mComponentManager, mEntityManager)),
      mEventManager(std::make_unique<event::Manager>(mSystemManager)),
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
    mComponentManager->AddComponent(newEntity, "Entity " + std::to_string(newEntity), mTagId);
    return newEntity;
}

Entity Scene::CreateEntity(std::string newName)
{
    Entity newEntity = mEntityManager->CreateEntity();
    mComponentManager->AddComponent(newEntity, data::Value{data::Record{{"name", newName}}}, mTagId);
    return newEntity;
}

std::unordered_map<Entity, Signature> Scene::getEntities() { return mEntityManager->getEntities(); }

void Scene::DestroyEntity(Entity entity)
{
    mEntityManager->DestroyEntity(entity);
    mComponentManager->EntityDestroyed(entity);
}

Signature Scene::getSignature(Entity entity) { return mEntityManager->GetSignature(entity); }

std::string Scene::getEntityName(Entity entity)
{
    return std::get<std::string>(
        std::get<data::Record>(mComponentManager->GetComponent(entity, mTagId).value()).at("name"));
}

uint32_t Scene::getLivingEntityCount() { return mEntityManager->getLivingEntityCount(); }

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

pivot::ecs::systems::Manager &Scene::getSystemManager() { return *this->mSystemManager.get(); }

const pivot::ecs::systems::Manager &Scene::getSystemManager() const { return *this->mSystemManager.get(); }

pivot::ecs::event::Manager &Scene::getEventManager() { return *this->mEventManager.get(); }

const pivot::ecs::event::Manager &Scene::getEventManager() const { return *this->mEventManager.get(); }

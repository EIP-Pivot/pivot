#include "pivot/ecs/Core/Scene.hxx"
#include <pivot/ecs/Core/Component/index.hxx>

using namespace pivot::ecs;

Scene::Scene(std::string sceneName)
    : name(sceneName),

      mComponentManager(std::make_unique<component::Manager>()),
      mEntityManager(std::make_unique<EntityManager>()),
      mEventManager(std::make_unique<EventManager>()),
      mSystemManager(std::make_unique<SystemManager>()),
      mCurrentCamera(0)
{
    auto tag = pivot::ecs::component::GlobalIndex::getSingleton().getComponentNameByType<Tag>().value();
    auto description = pivot::ecs::component::GlobalIndex::getSingleton().getDescription(tag).value();
    mTagId = mComponentManager->RegisterComponent(description);
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

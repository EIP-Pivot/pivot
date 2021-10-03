#include "pivot/ecs/Core/Scene.hxx"

void Scene::Init()
{
    mComponentManager = std::make_unique<ComponentManager>();
    mEntityManager = std::make_unique<EntityManager>();
    mEventManager = std::make_unique<EventManager>();
    mSystemManager = std::make_unique<SystemManager>();
}

Entity Scene::CreateEntity()
{
    return mEntityManager->CreateEntity();
}

void Scene::DestroyEntity(Entity entity)
{
    mEntityManager->DestroyEntity(entity);
    mComponentManager->EntityDestroyed(entity);
    mSystemManager->EntityDestroyed(entity);
}

void Scene::Update(float dt)
{
    for (std::shared_ptr<System> system: mSystems) {
        system->Update(dt);
    }
}

void Scene::AddEventListener(EventId eventId, std::function<void(Event &)> const &listener)
{
    mEventManager->AddListener(eventId, listener);
}

void Scene::SendEvent(Event &event)
{
    mEventManager->SendEvent(event);
}

void Scene::SendEvent(EventId eventId)
{
    mEventManager->SendEvent(eventId);
}

std::vector<RenderObject> Scene::getSceneInformations() const
{
    return obj;
}
#include "pivot/ecs/Core/Scene.hxx"

void Scene::Init()
{
    mComponentManager = std::make_unique<ComponentManager>();
    mEntityManager = std::make_unique<EntityManager>();
    mEventManager = std::make_unique<EventManager>();
    mSystemManager = std::make_unique<SystemManager>();
    mComponentManager->RegisterComponent<Tag>();
    mCurrentCamera = 0;
}

Entity Scene::CreateEntity()
{
    Entity newEntity = mEntityManager->CreateEntity();
    mComponentManager->AddComponent<Tag>(newEntity, {
        .name = "Default",
    });
    return newEntity;
}

void Scene::DestroyEntity(Entity entity)
{
    mEntityManager->DestroyEntity(entity);
    mComponentManager->EntityDestroyed(entity);
    mSystemManager->EntityDestroyed(entity);
}

Signature Scene::getSignature(Entity entity)
{
    return mEntityManager->GetSignature(entity);
}

uint32_t Scene::getLivingEntityCount()
{
    return mEntityManager->getLivingEntityCount();
}

std::unordered_map<const char *, ComponentType> Scene::getComponentsTypes()
{
    return mComponentManager->getComponentsTypes();
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


void Scene::setCamera(std::uint16_t camera)
{
    mCurrentCamera = camera;
}

void Scene::addCamera(Entity camera)
{
    mCamera.push_back(camera);
}

void Scene::switchCamera()
{
    mCurrentCamera = (mCurrentCamera + 1) % mCamera.size();
}

Camera &Scene::getCamera()
{
    if (mCamera.size() == 0)
        throw EcsException("No camera set");
    return mComponentManager->GetComponent<Camera>(mCamera[mCurrentCamera]);
}

std::vector<Entity> &Scene::getCameras()
{
    return mCamera;
}

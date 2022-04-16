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

const std::string &Scene::getName() const { return name; }

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

std::unordered_map<Entity, Signature> Scene::getEntities() const { return mEntityManager.getEntities(); }

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

void Scene::save(const std::filesystem::path &path) const
{
    // serialize scene
    nlohmann::json output;
    std::set<std::string> scriptUsed;
    output["name"] = name;
    for (auto &[entity, _]: mEntityManager.getEntities()) {
        for (pivot::ecs::component::ComponentRef ref: mComponentManager.GetAllComponents(entity)) {
            auto &provenance = ref.description().provenance;
            if (provenance.isExternalRessource()) scriptUsed.insert(provenance.getExternalRessource());
            output["components"][entity][ref.description().name] = nlohmann::json(ref.get());
        }
    }
    std::vector<std::string> systems;
    for (auto &[systemName, _]: mSystemManager) { systems.push_back(systemName); }
    output["systems"] = systems;
    output["scripts"] = scriptUsed;
    // write in file
    std::ofstream out(path);
    out << std::setw(4) << output << std::endl;
    out.close();
}

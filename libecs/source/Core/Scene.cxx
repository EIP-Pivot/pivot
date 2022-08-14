#include <set>

#include "pivot/ecs/Core/Scene.hxx"
#include <pivot/ecs/Components/Tag.hxx>
#include <pivot/ecs/Components/TagArray.hxx>
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

void Scene::switchCamera()
{
    if (mCamera.size() > 0) mCurrentCamera = (mCurrentCamera + 1) % mCamera.size();
}

pivot::builtins::Camera &Scene::getCamera()
{
    if (mCamera.size() == 0) throw EcsException("No camera set");
    throw std::logic_error("Unimplemented");
    // return mComponentManager.GetComponent<Camera>(mCamera[mCurrentCamera]);
}

std::vector<Entity> &Scene::getCameras() { return mCamera; }

pivot::ecs::component::Manager &Scene::getComponentManager() { return mComponentManager; }

/// Get the component manager (const)
const pivot::ecs::component::Manager &Scene::getComponentManager() const { return mComponentManager; }

const pivot::ecs::systems::Manager &Scene::getSystemManager() const { return mSystemManager; }

pivot::ecs::event::Manager &Scene::getEventManager() { return mEventManager; }

const pivot::ecs::event::Manager &Scene::getEventManager() const { return mEventManager; }

EntityManager &Scene::getEntityManager() { return mEntityManager; }

const EntityManager &Scene::getEntityManager() const { return mEntityManager; }

std::unique_ptr<Scene> Scene::load(const nlohmann::json &obj, const pivot::ecs::component::Index &cIndex,
                                   const pivot::ecs::systems::Index &sIndex)
{
    auto scene = std::make_unique<Scene>(obj["name"].get<std::string>());
    auto &componentManager = scene->getComponentManager();
    auto &entityManager = scene->getEntityManager();
    auto &systemManager = scene->mSystemManager;

    for (auto entities: obj["components"]) {
        auto entity = entityManager.CreateEntity();
        for (auto &component: entities.items()) {
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

namespace
{
void extract_assets(const data::Value &value, std::set<std::string> &assets,
                    std::optional<Scene::AssetTranslator> &assetTranslator)
{
    value.visit_data([&](const auto &data) {
        using type = std::decay_t<decltype(data)>;
        if constexpr (std::is_same_v<type, data::Asset>) {
            if (assetTranslator) {
                auto assetPath = assetTranslator.value()(data.name);
                if (assetPath.has_value()) { assets.insert(assetPath.value()); }
            } else {
                assets.insert(data.name);
            }
        }
    });
}
}    // namespace

void Scene::save(const std::filesystem::path &path, std::optional<AssetTranslator> assetTranslator,
                 std::optional<ScriptTranslator> scriptTranslator) const
{
    // serialize scene
    nlohmann::json output;
    std::set<std::string> scriptUsed;
    std::set<std::string> assets;

    auto addScript = [&scriptUsed, &scriptTranslator](const std::string &name) {
        if (scriptTranslator) {
            auto script = scriptTranslator.value()(name);
            if (script.has_value()) scriptUsed.insert(script.value());
        } else {
            scriptUsed.insert(name);
        }
    };

    output["name"] = name;
    for (auto &[entity, _]: mEntityManager.getEntities()) {
        for (pivot::ecs::component::ComponentRef ref: mComponentManager.GetAllComponents(entity)) {
            auto &provenance = ref.description().provenance;
            if (provenance.isExternalRessource()) addScript(provenance.getExternalRessource());
            output["components"][entity][ref.description().name] = nlohmann::json(ref.get());
            extract_assets(ref.get(), assets, assetTranslator);
        }
    }
    std::vector<std::string> systems;
    for (auto &[systemName, systemDescription]: mSystemManager) {
        if (systemDescription.provenance.isExternalRessource())
            addScript(systemDescription.provenance.getExternalRessource());
        systems.push_back(systemName);
    }
    output["systems"] = systems;
    output["scripts"] = scriptUsed;
    output["assets"] = assets;
    // write in file
    std::ofstream out(path);
    out << std::setw(4) << output << std::endl;
    out.close();
}

void Scene::registerSystem(const systems::Description &description, pivot::OptionalRef<const component::Index> cIndex)
{
    if (cIndex.has_value()) {
        for (auto &component: description.systemComponents) {
            if (!mComponentManager.GetComponentId(component).has_value()) {
                auto cDesc = cIndex->get().getDescription(component);
                if (cDesc.has_value()) { mComponentManager.RegisterComponent(*cDesc); }
            }
        }
    }
    mSystemManager.useSystem(description);
}

std::optional<Entity> Scene::getEntityID(const std::string &name)
{
    auto array = dynamic_cast<const component::TagArray &>(mComponentManager.GetComponentArray(mTagId));
    return array.getEntityID(name);
}

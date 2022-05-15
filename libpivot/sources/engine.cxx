#include <pivot/engine.hxx>

#include <pivot/internal/FrameLimiter.hxx>
#include <pivot/internal/camera.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>

#include <pivot/builtins/events/tick.hxx>
#include <pivot/builtins/systems/PhysicSystem.hxx>

using namespace pivot;
using namespace pivot::ecs;

namespace pivot
{
Engine::Engine()
    : m_scripting_engine(m_system_index, m_component_index), m_camera(builtins::Camera(glm::vec3(0, 200, 500)))
{
    m_component_index.registerComponent(builtins::components::Gravity::description);
    m_component_index.registerComponent(builtins::components::RigidBody::description);
    m_component_index.registerComponent(builtins::components::RenderObject::description);
    m_event_index.registerEvent(builtins::events::tick);
    m_system_index.registerSystem(builtins::systems::physicSystem);

    m_vulkan_application.addRenderer<pivot::graphics::CullingRenderer>();
    m_vulkan_application.addRenderer<pivot::graphics::GraphicsRenderer>();
    m_vulkan_application.addRenderer<pivot::graphics::ImGuiRenderer>();
    m_vulkan_application.init();
}

void Engine::run()
{

    float dt = 0.0f;
    FrameLimiter<60> fpsLimiter;
    // FIXME: Move to constructor when default mesh is added
    m_vulkan_application.init();
    while (!m_vulkan_application.window.shouldClose()) {
        auto startTime = std::chrono::high_resolution_clock::now();
        m_vulkan_application.window.pollEvent();

        this->onTick(dt);

        auto aspectRatio = m_vulkan_application.getAspectRatio();

        auto data = m_current_scene_render_object.value().get().getData();
        // FIXME: Send data and existence vector directly to the graphic library
        std::vector<std::reference_wrapper<const pivot::graphics::RenderObject>> objects;
        objects.reserve(objects.size());
        for (const auto &ro: data) { objects.push_back(ro); }

        m_vulkan_application.draw(objects, pivot::internals::getGPUCameraData(m_camera, Engine::fov, aspectRatio));

        if (!m_paused) {
            m_scene_manager.getCurrentScene().getEventManager().sendEvent(
                {pivot::builtins::events::tick, {}, data::Value(dt)});
        }

        fpsLimiter.sleep();
        auto stopTime = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<float>(stopTime - startTime).count();
    }
}

void Engine::changeCurrentScene(ecs::SceneManager::SceneId sceneId)
{
    m_scene_manager.setCurrentSceneId(sceneId);

    using RenderObject = pivot::builtins::components::RenderObject;
    auto &cm = m_scene_manager.getCurrentScene().getComponentManager();
    if (auto renderobject_id = cm.GetComponentId(RenderObject::description.name)) {
        auto &array = cm.GetComponentArray(*renderobject_id);
        m_current_scene_render_object =
            dynamic_cast<pivot::ecs::component::DenseTypedComponentArray<RenderObject> &>(array);
    } else {
        m_current_scene_render_object = std::nullopt;
    }
}

namespace
{
    void postSceneRegister(Scene &scene)
    {
        auto &cm = scene.getComponentManager();
        if (cm.GetComponentId(builtins::components::RenderObject::description.name).has_value()) { return; }
        cm.RegisterComponent(builtins::components::RenderObject::description);
    }
}    // namespace

ecs::SceneManager::SceneId Engine::registerScene()
{
    auto id = m_scene_manager.registerScene();
    postSceneRegister(m_scene_manager.getSceneById(id));
    return id;
}

ecs::SceneManager::SceneId Engine::registerScene(std::string name)
{

    auto id = m_scene_manager.registerScene(name);
    postSceneRegister(m_scene_manager.getSceneById(id));
    return id;
}

ecs::SceneManager::SceneId Engine::registerScene(std::unique_ptr<ecs::Scene> scene)
{
    auto id = m_scene_manager.registerScene(std::move(scene));
    postSceneRegister(m_scene_manager.getSceneById(id));
    return id;
}

void Engine::saveScene(ecs::SceneManager::SceneId id, const std::filesystem::path &path)
{
    auto assetTranslator =
        std::make_optional(std::function([this, &path](const std::string &asset) -> std::optional<std::string> {
            auto &assetStorage = m_vulkan_application.assetStorage;
            auto texturePath = assetStorage.getTexturePath(asset);
            auto modelPath = assetStorage.getModelPath(asset);
            if (!texturePath.has_value() && !modelPath.has_value()) return std::nullopt;
            std::filesystem::path assetPath = texturePath.value_or(modelPath.value());
            return assetPath.lexically_relative(path.parent_path()).string();
        }));

    auto scriptTranslator =
        std::make_optional(std::function([&path](const std::string &script) -> std::optional<std::string> {
            std::filesystem::path scriptPath = script;
            return scriptPath.lexically_relative(path.parent_path()).string();
        }));

    m_scene_manager.getSceneById(id).save(path, assetTranslator, scriptTranslator);
}

ecs::SceneManager::SceneId Engine::loadScene(const std::filesystem::path &path)
{

    std::ifstream scene_file{path};
    if (!scene_file.is_open()) {
        logger.err() << "Could not open scene file: " << std::strerror(errno);
        return 1;
    }
    auto scene_json = nlohmann::json::parse(scene_file);
    auto scene = Scene::load(scene_json, m_component_index, m_system_index);
    auto scene_base_path = path.parent_path();
    for (auto &asset: scene_json["assets"]) {
        auto assetPath = scene_base_path / asset.get<std::string>();
        m_vulkan_application.assetStorage.addAsset(assetPath);
    }
    for (auto &script: scene_json["scripts"]) {
        auto scriptPath = scene_base_path / script.get<std::string>();
        m_scripting_engine.loadFile(scriptPath.string(), false, true);
    }
    m_vulkan_application.buildAssetStorage(pivot::graphics::AssetStorage::BuildFlagBits::eClear);
    m_vulkan_application.buildAssetStorage();
    return this->registerScene(std::move(scene));
}

void Engine::loadScript(const std::filesystem::path &path) { m_scripting_engine.loadFile(path.string(), false, true); }

void Engine::loadAsset(const std::filesystem::path &path)
{
    m_vulkan_application.assetStorage.addAsset(path);
    m_vulkan_application.buildAssetStorage(pivot::graphics::AssetStorage::BuildFlagBits::eReloadOldAssets);
}
}    // namespace pivot

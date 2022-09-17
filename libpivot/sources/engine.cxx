#include <imgui.h>
#include <magic_enum.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

// Must be after imgui
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <pivot/engine.hxx>

#include <pivot/internal/FrameLimiter.hxx>
#include <pivot/internal/camera.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>

#include <pivot/builtins/events/collision.hxx>
#include <pivot/builtins/events/editor_tick.hxx>
#include <pivot/builtins/events/key_press.hxx>
#include <pivot/builtins/events/tick.hxx>

#include <pivot/builtins/systems/CollisionSystem.hxx>
#include <pivot/builtins/systems/CollisionTestSystem.hxx>
#include <pivot/builtins/systems/DrawTextSystem.hxx>
#include <pivot/builtins/systems/PhysicSystem.hxx>
#include <pivot/builtins/systems/TestTickSystem.hxx>

#include <pivot/builtins/components/Collidable.hxx>
#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/builtins/components/Transform.hxx>

#include <pivot/builtins/components/Light.hxx>
#include <pivot/builtins/components/Text.hxx>
#include <pivot/builtins/components/Transform2D.hxx>

#include <pivot/graphics/Resolver/AssetResolver.hxx>
#include <pivot/graphics/Resolver/DrawCallResolver.hxx>
#include <pivot/graphics/Resolver/LightDataResolver.hxx>

using namespace pivot;
using namespace pivot::ecs;

namespace pivot
{
Engine::Engine()
    : m_scripting_engine(
          m_system_index, m_component_index,
          pivot::ecs::script::interpreter::builtins::BuiltinContext{std::bind_front(&Engine::isKeyPressed, this)}),
      m_camera(builtins::Camera(glm::vec3(0, 5, 0)))
{
    DEBUG_FUNCTION();
    m_component_index.registerComponent(builtins::components::Gravity::description);
    m_component_index.registerComponent(builtins::components::RigidBody::description);
    m_component_index.registerComponent(builtins::components::RenderObject::description);
    m_component_index.registerComponent(builtins::components::Transform::description);
    m_component_index.registerComponent(builtins::components::PointLight::description);
    m_component_index.registerComponent(builtins::components::DirectionalLight::description);
    m_component_index.registerComponent(builtins::components::SpotLight::description);
    m_component_index.registerComponent(builtins::components::Collidable::description);
    m_component_index.registerComponent(builtins::components::Text::description);
    m_component_index.registerComponent(builtins::components::Transform2D::description);

    m_event_index.registerEvent(builtins::events::tick);
    m_event_index.registerEvent(builtins::events::editor_tick);
    m_event_index.registerEvent(builtins::events::keyPress);
    m_event_index.registerEvent(builtins::events::collision);
    m_system_index.registerSystem(builtins::systems::physicSystem);
    m_system_index.registerSystem(builtins::systems::makeCollisionSystem(m_vulkan_application.assetStorage));
    m_system_index.registerSystem(builtins::systems::collisionTestSystem);
    m_system_index.registerSystem(builtins::systems::testTickSystem);
    m_system_index.registerSystem(builtins::systems::drawTextSystem);

    auto current_entry = boost::dll::program_location().parent_path();
    auto find_asset_folder = [](const boost::filesystem::path &entry) -> std::optional<std::filesystem::path> {
        for(auto& directory_entry : boost::make_iterator_range(boost::filesystem::directory_iterator(entry), {})) {
            if (!boost::filesystem::is_directory(directory_entry))
                continue;
            if (directory_entry.path().filename() == "assets")
                return std::filesystem::path(directory_entry.path().string());
        }
        return std::nullopt;
    };
    for (int i = 0; i < PIVOT_ASSET_SEARCH_DEPTH; i++) {
        auto path = find_asset_folder(current_entry);
        if (path.has_value()) {
            m_asset_directory = path.value();
            break;
        }
        current_entry = current_entry.parent_path();
    }

    m_vulkan_application.addRenderer<pivot::graphics::CullingRenderer>();
    m_vulkan_application.addRenderer<pivot::graphics::GraphicsRenderer>();
    m_vulkan_application.addRenderer<pivot::graphics::ImGuiRenderer>();

    m_vulkan_application.addResolver<pivot::graphics::DrawCallResolver>(0);
    m_vulkan_application.addResolver<pivot::graphics::LightDataResolver>(1);
    m_vulkan_application.addResolver<pivot::graphics::AssetResolver>(2);

    m_vulkan_application.init(m_asset_directory);

    m_vulkan_application.window.addGlobalKeyPressCallback(std::bind_front(&Engine::onKeyPressed, this));
}

void Engine::run()
{
    DEBUG_FUNCTION();
    float dt = 0.0f;
    FrameLimiter<60> fpsLimiter;
    while (!m_vulkan_application.window.shouldClose()) {
        auto startTime = std::chrono::high_resolution_clock::now();
        m_vulkan_application.window.pollEvent();

        this->onFrameStart();

        m_scene_manager.getCurrentScene().getEventManager().sendEvent(
            {pivot::builtins::events::editor_tick, {}, data::Value(dt)});

        this->onTick(dt);

        float aspectRatio =
            (renderArea.has_value())
                ? (static_cast<float>(renderArea->extent.width) / static_cast<float>(renderArea->extent.height))
                : (m_vulkan_application.getAspectRatio());

        this->onFrameEnd();

        if (m_current_scene_draw_command) {
            auto result = m_vulkan_application.draw(
                m_current_scene_draw_command.value(),
                pivot::internals::getGPUCameraData(m_camera, Engine::fov, aspectRatio), renderArea);
            if (result == pivot::graphics::VulkanApplication::DrawResult::Error) {
                std::terminate();
            } else if (result == pivot::graphics::VulkanApplication::DrawResult::FrameSkipped) {
                this->onReset();
            }
        }

        if (!m_paused) {
            m_scene_manager.getCurrentScene().getEventManager().sendEvent(
                {pivot::builtins::events::tick, {}, data::Value(dt)});
        }

        fpsLimiter.sleep();
        auto stopTime = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<float>(stopTime - startTime).count();
    }
}

template <typename T>
using Array = pivot::ecs::component::DenseTypedComponentArray<T>;

void Engine::changeCurrentScene(ecs::SceneManager::SceneId sceneId)
{
    DEBUG_FUNCTION();
    m_scene_manager.setCurrentSceneId(sceneId);

    using namespace pivot::builtins::components;

    auto &cm = m_scene_manager.getCurrentScene().getComponentManager();
    auto renderobject_id = cm.GetComponentId(RenderObject::description.name);
    auto transform_id = cm.GetComponentId(Transform::description.name);
    auto pointlight_id = cm.GetComponentId(PointLight::description.name);
    auto directional_id = cm.GetComponentId(DirectionalLight::description.name);
    auto spotlight_id = cm.GetComponentId(SpotLight::description.name);
    if (renderobject_id && transform_id && pointlight_id && directional_id && spotlight_id) {
        auto &ro_array = dynamic_cast<Array<pivot::graphics::RenderObject> &>(cm.GetComponentArray(*renderobject_id));
        auto &transform_array = dynamic_cast<Array<pivot::graphics::Transform> &>(cm.GetComponentArray(*transform_id));
        auto &point_array = dynamic_cast<Array<pivot::graphics::PointLight> &>(cm.GetComponentArray(*pointlight_id));
        auto &directional_array =
            dynamic_cast<Array<pivot::graphics::DirectionalLight> &>(cm.GetComponentArray(*directional_id));
        auto &spotlight_array = dynamic_cast<Array<pivot::graphics::SpotLight> &>(cm.GetComponentArray(*spotlight_id));

        m_current_scene_draw_command = {
            .renderObjects =
                {
                    .objects = ro_array.getComponents(),
                    .exist = ro_array.getExistence(),
                },
            .pointLight =
                {
                    .objects = point_array.getComponents(),
                    .exist = point_array.getExistence(),
                },
            .directionalLight =
                {
                    .objects = directional_array.getComponents(),
                    .exist = directional_array.getExistence(),
                },
            .spotLight =
                {
                    .objects = spotlight_array.getComponents(),
                    .exist = spotlight_array.getExistence(),
                },
            .transform =
                {
                    .objects = transform_array.getComponents(),
                    .exist = transform_array.getExistence(),
                },
        };
    } else {
        m_current_scene_draw_command = std::nullopt;
    }
}

namespace
{
    void postSceneRegister(Scene &scene)
    {
        DEBUG_FUNCTION();
        auto &cm = scene.getComponentManager();
        if (!cm.GetComponentId(builtins::components::RenderObject::description.name).has_value()) {
            cm.RegisterComponent(builtins::components::RenderObject::description);
        }
        if (!cm.GetComponentId(builtins::components::Transform::description.name).has_value()) {
            cm.RegisterComponent(builtins::components::Transform::description);
        }
        if (!cm.GetComponentId(builtins::components::PointLight::description.name).has_value()) {
            cm.RegisterComponent(builtins::components::PointLight::description);
        }
        if (!cm.GetComponentId(builtins::components::DirectionalLight::description.name).has_value()) {
            cm.RegisterComponent(builtins::components::DirectionalLight::description);
        }
        if (!cm.GetComponentId(builtins::components::SpotLight::description.name).has_value()) {
            cm.RegisterComponent(builtins::components::SpotLight::description);
        }
    }
}    // namespace

ecs::SceneManager::SceneId Engine::registerScene()
{
    DEBUG_FUNCTION();
    auto id = m_scene_manager.registerScene();
    postSceneRegister(m_scene_manager.getSceneById(id));
    return id;
}

ecs::SceneManager::SceneId Engine::registerScene(std::string name)
{
    DEBUG_FUNCTION();
    auto id = m_scene_manager.registerScene(name);
    postSceneRegister(m_scene_manager.getSceneById(id));
    return id;
}

ecs::SceneManager::SceneId Engine::registerScene(std::unique_ptr<ecs::Scene> scene)
{
    DEBUG_FUNCTION();
    auto id = m_scene_manager.registerScene(std::move(scene));
    postSceneRegister(m_scene_manager.getSceneById(id));
    return id;
}

void Engine::saveScene(ecs::SceneManager::SceneId id, const std::filesystem::path &path)
{
    DEBUG_FUNCTION();
    auto assetTranslator =
        std::make_optional(std::function([this, &path](const std::string &asset) -> std::optional<std::string> {
            auto &assetStorage = m_vulkan_application.assetStorage;
            auto texturePath = assetStorage.getTexturePath(asset);
            auto modelPath = assetStorage.getModelPath(asset);
            if (!texturePath.has_value() && !modelPath.has_value()) return std::nullopt;
            std::filesystem::path assetPath = texturePath.value_or(modelPath.value());
            return std::filesystem::relative(std::filesystem::absolute(assetPath), path.parent_path()).string();
        }));

    auto scriptTranslator =
        std::make_optional(std::function([&path](const std::string &script) -> std::optional<std::string> {
            std::filesystem::path scriptPath = script;
            return std::filesystem::relative(std::filesystem::absolute(scriptPath), path.parent_path()).string();
        }));

    m_scene_manager.getSceneById(id).save(path, assetTranslator, scriptTranslator);
}

ecs::SceneManager::SceneId Engine::loadScene(const std::filesystem::path &path)
{
    DEBUG_FUNCTION();
    logger.info("Scene Manager") << "Loading scene at " << path;
    std::ifstream scene_file{path};
    if (!scene_file.is_open()) {
        logger.err() << "Could not open scene file: " << std::strerror(errno);
        return 1;
    }
    auto scene_json = nlohmann::json::parse(scene_file);
    auto scene_base_path = path.parent_path();
    for (auto &script: scene_json["scripts"]) {
        auto scriptPath = scene_base_path / script.get<std::string>();
        m_scripting_engine.loadFile(scriptPath.string(), false, true);
    }
    m_vulkan_application.assetStorage.setAssetDirectory(scene_base_path);
    for (auto &asset: scene_json["assets"]) loadAsset(asset.get<std::string>(), false);
    m_vulkan_application.buildAssetStorage(graphics::AssetStorage::BuildFlagBits::eReloadOldAssets);
    auto scene = Scene::load(scene_json, m_component_index, m_system_index);
    return this->registerScene(std::move(scene));
}

void Engine::loadScript(const std::filesystem::path &path)
{
    DEBUG_FUNCTION();
    m_scripting_engine.loadFile(path.string(), false, true);
}

void Engine::loadAsset(const std::filesystem::path &path, bool reload)
{
    DEBUG_FUNCTION();
    m_vulkan_application.assetStorage.addAsset(path);
    if (reload) m_vulkan_application.buildAssetStorage(pivot::graphics::AssetStorage::BuildFlagBits::eReloadOldAssets);
}

bool Engine::isKeyPressed(const std::string &key) const
{
    auto key_cast = magic_enum::enum_cast<pivot::graphics::Window::Key>(key);
    if (key_cast.has_value()) {
        return this->m_vulkan_application.window.isKeyPressed(key_cast.value());
    } else {
        return false;
    }
}

void Engine::onKeyPressed(graphics::Window &, const graphics::Window::Key key, const graphics::Window::Modifier)
{
    if (!m_paused) {
        m_scene_manager.getCurrentScene().getEventManager().sendEvent(
            {pivot::builtins::events::keyPress, {}, data::Value(std::string(magic_enum::enum_name(key)))});
    }
}

}    // namespace pivot

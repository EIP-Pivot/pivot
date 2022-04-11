#include <pivot/engine.hxx>

#include <pivot/internal/FrameLimiter.hxx>
#include <pivot/internal/camera.hxx>

#include <pivot/ecs/Components/Gravity.hxx>
#include <pivot/ecs/Components/RigidBody.hxx>

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>

#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/builtins/events/tick.hxx>
#include <pivot/builtins/systems/PhysicSystem.hxx>

using namespace pivot;
using namespace pivot::ecs;

namespace pivot
{
Engine::Engine(): m_camera(Camera(glm::vec3(0, 200, 500)))
{
    m_component_index.registerComponent(Gravity::description);
    m_component_index.registerComponent(RigidBody::description);
    m_component_index.registerComponent(builtins::components::RenderObject::description);
    m_event_index.registerEvent(builtins::events::tick);
    m_system_index.registerSystem(builtins::systems::physicSystem);
}

void Engine::changeCurrentScene(ecs::SceneManager::SceneId sceneId) { m_scene_manager.setCurrentSceneId(sceneId); }

void Engine::run()
{

    float dt = 0.0f;
    m_vulkan_application.init();
    FrameLimiter<60> fpsLimiter;
    while (!m_vulkan_application.window.shouldClose()) {
        auto startTime = std::chrono::high_resolution_clock::now();
        m_vulkan_application.window.pollEvent();

        this->onTick(dt);

        auto aspectRatio = m_vulkan_application.getAspectRatio();
        float fov = 80;

        using RenderObject = pivot::builtins::components::RenderObject;
        auto &cm = m_scene_manager.getCurrentScene().getComponentManager();
        auto renderobject_id = cm.GetComponentId("RenderObject").value();
        auto &array = cm.GetComponentArray(renderobject_id);
        pivot::ecs::component::DenseTypedComponentArray<RenderObject> &dense_array =
            dynamic_cast<pivot::ecs::component::DenseTypedComponentArray<RenderObject> &>(array);

        auto data = dense_array.getData();
        std::vector<std::reference_wrapper<const pivot::graphics::RenderObject>> objects;
        for (const auto &ro: data) { objects.push_back(ro); }

        m_vulkan_application.draw(
            objects, pivot::internals::getGPUCameraData(m_camera, fov, aspectRatio)
#ifdef CULLING_DEBUG
                         ,
            std::make_optional(pivot::internals::getGPUCameraData(m_culling_camera, fov, aspectRatio))
#endif
        );

        if (!m_paused) {
            m_scene_manager.getCurrentScene().getEventManager().sendEvent(
                {pivot::builtins::events::tick, {}, data::Value(dt)});
        }

        fpsLimiter.sleep();
        auto stopTime = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<float>(stopTime - startTime).count();
    }
}
}    // namespace pivot

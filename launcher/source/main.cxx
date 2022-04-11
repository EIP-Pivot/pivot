#include <Logger.hpp>

#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/engine.hxx>

using RenderObject = pivot::builtins::components::RenderObject;

class Launcher : public pivot::Engine
{
public:
    void init()
    {
        m_vulkan_application.assetStorage.loadModels("../editor/assets/cube.obj");
        m_vulkan_application.assetStorage.loadTextures("../editor/assets/violet.png");

        auto id = registerScene("Test launcher");
        changeCurrentScene(id);
        auto scene = getCurrentScene();

        Entity cube = scene->CreateEntity("cube");
        auto &cm = scene->getComponentManager();

        auto render_object_id = cm.GetComponentId(RenderObject::description.name);

        RenderObject cube_render_object{"cube", "", "white", Transform()};

        cm.AddComponent(cube,
                        pivot::ecs::component::helpers::Helpers<RenderObject>::createValueFromType(cube_render_object),
                        render_object_id.value());
    }
};

int main(int argc, char *argv[])
{
    logger.start();
    Launcher launcher;
    launcher.init();
    launcher.run();
    return 0;
}

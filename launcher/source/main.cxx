#include <Logger.hpp>

#include <pivot/builtins/components/RenderObject.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/engine.hxx>

using RenderObject = pivot::builtins::components::RenderObject;
using Transform = pivot::graphics::Transform;

class Launcher : public pivot::Engine
{
public:
    void init()
    {
        m_vulkan_application.assetStorage.loadModels("cube.obj");
        m_vulkan_application.assetStorage.loadTextures("violet.png");
    }
};

int main(int argc, char *argv[])
{
    logger.start();

    if (argc != 2) {
        logger.err() << "Missing scene path";
        return 1;
    }

    std::filesystem::path scene_path{argv[1]};
    Launcher launcher;
    launcher.init();
    auto sceneId = launcher.loadScene(scene_path);
    launcher.changeCurrentScene(sceneId);
    launcher.run();
    return 0;
}

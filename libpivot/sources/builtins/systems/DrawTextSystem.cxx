#include <cpplogger/Logger.hpp>
#include <imgui.h>

#include <pivot/builtins/events/editor_tick.hxx>
#include <pivot/builtins/systems/DrawTextSystem.hxx>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>

#include <pivot/builtins/components/Text.hxx>
#include <pivot/builtins/components/Transform2D.hxx>

#include <pivot/pivot.hxx>

using namespace pivot::ecs;
using namespace pivot::builtins::components;

namespace
{
void drawText(Entity entity, const Text &text, const Transform2D &transform)
{
    DEBUG_FUNCTION
    auto color = text.color.rgba;

    ImGui::SetNextWindowPos(ImVec2(transform.position.x, transform.position.y), 0, ImVec2(0.5, 0.5));
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    auto window_title = "text " + std::to_string(entity);
    ImGui::Begin(window_title.c_str(), NULL,
                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color[0], color[1], color[2], color[3]));
    ImGui::Text("%s", text.content.c_str());
    ImGui::PopStyleColor();
    ImGui::End();
}

std::vector<event::Event> drawTextSystemImpl(const systems::Description &, component::ArrayCombination &cmb,
                                             [[maybe_unused]] const event::EventWithComponent &event)
{
    DEBUG_FUNCTION
    auto textArray = dynamic_cast<component::DenseTypedComponentArray<Text> &>(cmb.arrays()[0].get());
    auto transformArray = dynamic_cast<component::DenseTypedComponentArray<Transform2D> &>(cmb.arrays()[1].get());

    auto maxEntity = std::min({textArray.maxEntity(), transformArray.maxEntity()});
    for (std::size_t entity = 0; entity < maxEntity; entity++) {
        if (!textArray.entityHasValue(entity) || !transformArray.entityHasValue(entity)) continue;

        auto &text = textArray.getData()[entity];
        auto &transform = transformArray.getData()[entity];

        drawText(entity, text, transform);
    }
    return {};
}
}    // namespace

namespace pivot::builtins::systems
{

const pivot::ecs::systems::Description drawTextSystem{
    .name = "Draw Text System",
    .entityName = "",
    .systemComponents =
        {
            "Text",
            "Transform2D",
        },
    .eventListener = events::editor_tick,
    .eventComponents = {},
    .provenance = pivot::ecs::Provenance::builtin(),
    .system = &drawTextSystemImpl,

};
}

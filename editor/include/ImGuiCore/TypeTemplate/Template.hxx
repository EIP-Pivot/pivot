#pragma once

#include <glm/gtc/type_ptr.hpp>

#include "ImGuiCore/CustomWidget.hxx"
#include "Windows/AssetWindow.hxx"
#include <pivot/ecs/Core/Component/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

void draw(glm::vec3 &value, const std::string &name) { CustomWidget::CustomVec3(name, value); }

void draw(glm::vec2 &value, const std::string &name) { ImGui::InputFloat2(name.c_str(), glm::value_ptr(value)); }

void draw(std::string &value, const std::string &name) { CustomWidget::CustomInputText(name, value); }

void draw(int &value, const std::string &name) { CustomWidget::CustomInputInt(name, value); }

void draw(bool &value, const std::string &name) { ImGui::Checkbox(name.c_str(), &value); }

void draw(double &value, const std::string &name) { CustomWidget::CustomInputDouble(name, value); }

void draw(pivot::ecs::data::Asset &asset, const std::string &name)
{
    CustomWidget::CustomInputText(name, asset.name);
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET")) {
            pivotAssertMsg(payload->Data, "Empty drag and drop payload");
            auto *assetWrapper = reinterpret_cast<pivot::editor::AssetWindow::wrapper *>(payload->Data);
            asset.name = std::string(assetWrapper->name);
        }
        ImGui::EndDragDropTarget();
    }
}

void draw(pivot::ecs::data::Color &color, const std::string &name)
{
    ImGui::ColorEdit4(name.c_str(), color.rgba.data());
}

void draw(pivot::ecs::data::Void &, const std::string &) { ImGui::Text("This component has no data"); }

void draw(pivot::ecs::data::Value &value, const std::string &name);
void draw(pivot::ecs::data::Record &values, const std::string &)
{
    for (auto &[name, value]: values) draw(value, name);
}

void draw(pivot::ecs::data::Value &value, const std::string &name)
{
    std::visit([&name](auto &&arg) { draw(arg, name); }, static_cast<pivot::ecs::data::Value::variant &>(value));
}

#pragma once
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

// Must be after imgui
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

#include <pivot/ecs/Core/Component/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>

#include "ImGuiCore/CustomWidget.hxx"
#include "Windows/AssetWindow.hxx"

class ValueInput
{
public:
    ValueInput(pivot::ecs::CurrentScene scene): m_scene(scene) {}

    void drawInput(glm::vec3 &value, const std::string &name) { CustomWidget::CustomVec3(name, value); }

    void drawInput(glm::vec2 &value, const std::string &name)
    {
        ImGui::InputFloat2(name.c_str(), glm::value_ptr(value));
    }

    void drawInput(std::string &value, const std::string &name) { ImGui::InputText(name.c_str(), &value); }

    void drawInput(int &value, const std::string &name) { ImGui::InputInt(name.c_str(), &value); }

    void drawInput(bool &value, const std::string &name) { ImGui::Checkbox(name.c_str(), &value); }

    void drawInput(double &value, const std::string &name) { ImGui::InputDouble(name.c_str(), &value); }

    void drawInput(pivot::ecs::data::Asset &asset, const std::string &name)
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

    void drawInput(pivot::ecs::data::Void &, const std::string &name)
    {
        ImGui::Text("%s: This component has no data", name.c_str());
    }
    void drawInput(pivot::EntityRef &value, const std::string &name)
    {
        std::string entityName = "";
        if (!value.is_empty()) { entityName = m_scene->getEntityName(value.ref); }

        ImGui::InputText(name.c_str(), &entityName);

        if (entityName.empty()) {
            value = pivot::EntityRef::empty();
        } else {
            auto id = m_scene->getEntityID(entityName);
            if (id.has_value()) { value.ref = id.value(); }
        }
        // logger.info() << "new ref " << value;
    }

    void drawInput(pivot::ecs::data::Color &color, const std::string &name)
    {
        ImGui::ColorEdit4(name.c_str(), color.rgba.data());
    }

    void drawInput(pivot::ecs::data::Record &values, const std::string &)
    {
        for (auto &[name, value]: values) drawInput(value, name);
    }

    void drawInput(pivot::ecs::data::ScriptEntity &entity, const std::string &)
    {
        for (auto &[name, value]: entity.components) drawInput(value, name);
    }

    void drawInput(pivot::ecs::data::List &list, const std::string &name)
    {
        for (size_t i = 0; i < list.items.size(); i++) drawInput(list.items.at(i), std::to_string(i));
    }

    void drawInput(pivot::ecs::data::Value &value, const std::string &name)
    {
        std::visit([&name, this](auto &&arg) { this->drawInput(arg, name); },
                   static_cast<pivot::ecs::data::Value::variant &>(value));
    }

private:
    pivot::ecs::CurrentScene m_scene;
};

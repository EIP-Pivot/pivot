#pragma once
#include <imgui.h>

// Must be after imgui
#include <ImGuizmo.h>
#include <misc/cpp/imgui_stdlib.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

#include <pivot/ecs/Core/Component/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>

class ValueInput
{
public:
    ValueInput(pivot::ecs::CurrentScene scene): m_scene(scene) {}

    void drawInput(glm::vec3 &value, const std::string &name)
    {
        ImGui::InputFloat3(name.c_str(), glm::value_ptr(value));
    }

    void drawInput(std::string &value, const std::string &name) { ImGui::InputText(name.c_str(), &value); }

    void drawInput(int &value, const std::string &name) { ImGui::InputInt(name.c_str(), &value); }

    void drawInput(bool &value, const std::string &name) { ImGui::Checkbox(name.c_str(), &value); }

    void drawInput(double &value, const std::string &name) { ImGui::InputDouble(name.c_str(), &value); }

    void drawInput(pivot::ecs::data::Asset &asset, const std::string &name)
    {
        ImGui::InputText(name.c_str(), &asset.name);
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

    void drawInput(pivot::ecs::data::Record &values, const std::string &)
    {
        for (auto &[name, value]: values) drawInput(value, name);
    }

    void drawInput(pivot::ecs::data::Value &value, const std::string &name)
    {
        std::visit([&name, this](auto &&arg) { this->drawInput(arg, name); },
                   static_cast<pivot::ecs::data::Value::variant &>(value));
    }

private:
    pivot::ecs::CurrentScene m_scene;
};

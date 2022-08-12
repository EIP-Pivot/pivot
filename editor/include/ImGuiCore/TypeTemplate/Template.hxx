#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <pivot/ecs/Core/Component/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

#include <ImGuizmo.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void draw(glm::vec3 &value, const std::string &name) { ImGui::InputFloat3(name.c_str(), glm::value_ptr(value)); }

void draw(std::string &value, const std::string &name) { ImGui::InputText(name.c_str(), &value); }

void draw(int &value, const std::string &name) { ImGui::InputInt(name.c_str(), &value); }

void draw(bool &value, const std::string &name) { ImGui::Checkbox(name.c_str(), &value); }

void draw(double &value, const std::string &name) { ImGui::InputDouble(name.c_str(), &value); }

void draw(pivot::ecs::data::Asset &asset, const std::string &name) { ImGui::InputText(name.c_str(), &asset.name); }

void draw(pivot::ecs::data::Void &, const std::string &name)
{
    ImGui::Text("%s: This component has no data", name.c_str());
}
void draw(pivot::EntityRef &value, const std::string &name)
{
    // FIXME: Handle entity reference input. Be careful about handling null
    // references. Empty input should store EntityRef::empty() in the reference.
    if (value.is_empty()) {
        ImGui::Text("%s: Empty entity ref", name.c_str());
    } else {
        ImGui::Text("%s: Entity ref %u", name.c_str(), value.ref);
    }
}

void draw(pivot::ecs::data::Value &value, const std::string &name);
void draw(pivot::ecs::data::Record &values, const std::string &)
{
    for (auto &[name, value]: values) draw(value, name);
}

void draw(pivot::ecs::data::Value &value, const std::string &name)
{
    std::visit([&name](auto &&arg) { draw(arg, name); }, static_cast<pivot::ecs::data::Value::variant &>(value));
}

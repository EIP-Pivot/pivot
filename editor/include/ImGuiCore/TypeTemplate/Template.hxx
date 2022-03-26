#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <pivot/ecs/Core/Component/description.hxx>
#include <pivot/ecs/Core/Data/value.hxx>

#include <ImGuizmo.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace pivot::ecs::component;
using namespace pivot::ecs::data;

void draw(Value &value, std::string name);
void draw(glm::vec3 &value, std::string name) { ImGui::InputFloat3(name.c_str(), glm::value_ptr(value)); }

void draw(std::string &value, std::string name) { ImGui::InputText(name.c_str(), &value); }

void draw(int &value, std::string name)
{
    // ImGui::InputText("Name", &value);
}

void draw(bool &value, std::string name)
{
    // ImGui::InputText("Name", &value);
}

void draw(double &value, std::string name)
{
    // ImGui::InputText("Name", &value);
}

void draw(Record &values, std::string name)
{
    for (auto &[name, value]: values) draw(value, name);
}

void draw(Value &value, std::string name)
{
    std::visit([&name](auto &&arg) { draw(arg, name); }, static_cast<Value::variant &>(value));
}

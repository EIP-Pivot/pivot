#pragma once

#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>

class CustomWidget
{
public:
    static void CustomVec3(const std::string &label, glm::vec3 &values, const glm::vec3 resetValues = glm::vec3{0.f},
                           float columnWidth = 100.f);
    static void ImageText(ImTextureID texture_id, const ImVec2 &size, const std::string &text);
};

#pragma once

#include <imgui.h>
#include <map>

class ImGuiTheme
{
public:
    ImGuiTheme();
    void setStyle();

    void setColors();
    void getColor(const std::string &label, ImVec4 &color);
    static void setDefaultFramePadding();
    static void unsetDefaultFramePadding();

private:
    std::map<std::string, ImVec4> colors;
    void setColorsStyle();
};

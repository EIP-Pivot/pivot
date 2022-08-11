#include "ImGuiCore/ImGuiTheme.hxx"

ImGuiTheme::ImGuiTheme()
{
    colors.insert({"Dark", ImVec4(0.00396481, 0.00396481, 0.00490195, 1.00f)});
    colors.insert({"Gray", ImVec4(0.0166282, 0.0166282, 0.0196078, 1.00f)});
    colors.insert({"FrameBg", ImVec4(0.064927, 0.0669828, 0.0686275, 1.00f)});
    colors.insert({"FrameHover", ImVec4(0.129412, 0.129412, 0.129412, 1.00f)});
    colors.insert({"FrameActive", ImVec4(0.0509804, 0.0509804, 0.0509804, 1.00f)});
    colors.insert({"Button", ImVec4(0.936275, 0.275375, 0, 1.00f)});
    colors.insert({"ButtonHover", ImVec4(0.127451, 0.127451, 0.127451, 1.00f)});
    colors.insert({"ButtonActive", ImVec4(0.0509804, 0.0509804, 0.0509804, 1.00f)});
    colors.insert({"Separator", ImVec4(0.129412, 0.129412, 0.129412, 1.00f)});
    colors.insert({"SeparatorHovered", ImVec4(0.937255, 0.27451, 0, 1.00f)});
    colors.insert({"SeparatorActive", ImVec4(0.937255, 0.27451, 0, 1.00f)});
    colors.insert({"Header", ImVec4(0.129412, 0.129412, 0.129412, 0.55f)});
    colors.insert({"HeaderHovered", ImVec4(0.129412, 0.129412, 0.129412, 0.80f)});
    colors.insert({"HeaderActive", ImVec4(0.129412, 0.129412, 0.129412, 1.00f)});
}

void ImGuiTheme::setStyle() { setColorsStyle(); }

void ImGuiTheme::setColorsStyle()
{
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    ImGui::GetStyle().TabRounding = 5.0f;
    ImGui::GetStyle().WindowBorderSize = 0.f;
    ImGui::GetStyle().ChildBorderSize = 0.f;
    ImGui::GetStyle().FramePadding = ImVec2(ImGui::GetStyle().FramePadding.x, 7.0f);

    auto &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = colors["Gray"];
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = colors["FrameBg"];
    style.Colors[ImGuiCol_FrameBgHovered] = colors["FrameHover"];
    style.Colors[ImGuiCol_FrameBgActive] = colors["FrameActive"];
    style.Colors[ImGuiCol_TitleBg] = colors["Dark"];
    style.Colors[ImGuiCol_TitleBgActive] = colors["Dark"];
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    style.Colors[ImGuiCol_MenuBarBg] = colors["Dark"];
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_Button] = colors["Button"];
    style.Colors[ImGuiCol_ButtonHovered] = colors["ButtonHover"];
    style.Colors[ImGuiCol_ButtonActive] = colors["ButtonActive"];
    style.Colors[ImGuiCol_Header] = colors["Header"];
    style.Colors[ImGuiCol_HeaderHovered] = colors["HeaderHovered"];
    style.Colors[ImGuiCol_HeaderActive] = colors["HeaderActive"];
    style.Colors[ImGuiCol_Separator] = colors["Separator"];
    style.Colors[ImGuiCol_SeparatorHovered] = colors["SeparatorHovered"];
    style.Colors[ImGuiCol_SeparatorActive] = colors["SeparatorActive"];
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_Tab] = colors["Dark"];
    style.Colors[ImGuiCol_TabHovered] = colors["Gray"];
    style.Colors[ImGuiCol_TabActive] = colors["Gray"];
    style.Colors[ImGuiCol_TabUnfocused] = colors["Dark"];
    style.Colors[ImGuiCol_TabUnfocusedActive] = colors["Gray"];
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGuiTheme::setColors()
{
    ImGui::Begin("Colors");
    std::cout << "------- Colors -------" << std::endl;
    for (auto &[key, value]: colors) { getColor(key, value); }
    std::cout << "----------------------" << std::endl;
    ImGui::End();
}

void ImGuiTheme::getColor(const std::string &label, ImVec4 &color)
{
    float col[4] = {color.x, color.y, color.z, color.w};
    ImGui::ColorEdit4(label.c_str(), col);
    std::cout << label << ": " << col[0] << ", " << col[1] << ", " << col[2] << ", " << col[3] << std::endl;
    color.x = col[0];
    color.y = col[1];
    color.z = col[2];
    color.w = col[3];
}

void ImGuiTheme::setDefaultFramePadding() { ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.f, 3.f)); }

void ImGuiTheme::unsetDefaultFramePadding() { ImGui::PopStyleVar(); }

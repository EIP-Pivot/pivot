#include "ImGuiCore/CustomWidget.hxx"

#include <pivot/pivot.hxx>

void CustomWidget::CustomVec3(const std::string &label, glm::vec3 &values, const glm::vec3 resetValues,
                              float columnWidth)
{
    PROFILE_FUNCTION();
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{1.0f, 3.0f});

    float lineHeight = (GImGui->Font->FontSize * boldFont->Scale) + GImGui->Style.FramePadding.y * 2.f;
    ImVec2 buttonSize = {lineHeight + 3.f, lineHeight};

    // X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize)) values.x = resetValues.x;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.x, 0.1f);
    ImGui::PopItemWidth();
    //    UI::Tooltip(std::to_string(values.x));
    ImGui::SameLine();
    ImGui::Text("");
    ImGui::SameLine();

    // Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize)) values.y = resetValues.y;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, 0.1f);
    ImGui::PopItemWidth();
    //    UI::Tooltip(std::to_string(values.y));
    ImGui::SameLine();
    ImGui::Text("");
    ImGui::SameLine();

    // Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize)) values.z = resetValues.z;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, 0.1f);
    ImGui::PopItemWidth();
    //    UI::Tooltip(std::to_string(values.z));

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();
}

void CustomWidget::CustomInputText(const std::string &label, std::string &value, float columnWidth)
{
    PROFILE_FUNCTION();
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushItemWidth(-1);
    ImGui::InputText("##text", &value);
    ImGui::PopItemWidth();

    ImGui::Columns(1);

    ImGui::PopID();
}

void CustomWidget::CustomInputInt(const std::string &label, int &value, float columnWidth)
{
    PROFILE_FUNCTION();
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushItemWidth(-1);
    ImGui::InputInt("##int", &value);
    ImGui::PopItemWidth();

    ImGui::Columns(1);

    ImGui::PopID();
}

void CustomWidget::CustomInputDouble(const std::string &label, double &value, float columnWidth)
{
    PROFILE_FUNCTION();
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    ImGui::PushItemWidth(-1);
    ImGui::InputDouble("##int", &value);
    ImGui::PopItemWidth();

    ImGui::Columns(1);

    ImGui::PopID();
}

void CustomWidget::ImageText(ImTextureID texture_id, const ImVec2 &size, const std::string &text)
{
    PROFILE_FUNCTION();
    ImGui::BeginGroup();
    ImGui::Image(texture_id, size);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((size.x - ImGui::CalcTextSize(text.c_str()).x) * 0.5f));
    ImGui::Text("%s", text.c_str());
    ImGui::EndGroup();
}

bool CustomWidget::ButtonCenteredOnLine(const std::string &label, float alignment)
{
    PROFILE_FUNCTION();
    ImGuiStyle &style = ImGui::GetStyle();

    float size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;

    float off = (avail - size) * alignment;
    if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    return ImGui::Button(label.c_str());
}

bool CustomWidget::RadioImageButton(const std::string &label, ImTextureID texture_id, const ImVec2 &size, bool active)
{
    PROFILE_FUNCTION();
    bool buttonValue;
    ImVec4 bgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    ImVec4 activeColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
    ImVec4 hoverColor(0.08f, 0.08f, 0.092f, 1.00f);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.f, 5.f));
    if (active) {
        ImGui::PushID(label.c_str());
        ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgColor);
        buttonValue = ImGui::ImageButton(label.c_str(), texture_id, size, ImVec2(0, 0), ImVec2(1, 1),
                                         ImVec4(0, 0, 0, 0), activeColor);
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, hoverColor);
        buttonValue = ImGui::ImageButton(label.c_str(), texture_id, size);
        ImGui::PopStyleColor(3);
    }
    ImGui::PopStyleVar();
    return buttonValue;
}

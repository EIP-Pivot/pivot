#include "ImGuiCore/CustomWidget.hxx"

void CustomWidget::CustomVec3(const std::string &label, glm::vec3 &values, const glm::vec3 resetValues,
                              float columnWidth)
{
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
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

void CustomWidget::ImageText(ImTextureID texture_id, const ImVec2 &size, const std::string &text)
{
    ImGui::BeginGroup();
    ImGui::Image(texture_id, size);
    ImGui::SetCursorPosX(((size.x - ImGui::CalcTextSize(text.c_str()).x) * 0.5f) + ImGui::GetStyle().ItemSpacing.x);
    ImGui::Text("%s", text.c_str());
    ImGui::EndGroup();
}

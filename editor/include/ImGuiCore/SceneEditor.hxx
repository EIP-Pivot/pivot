#pragma once

#include <imgui.h>

#include "ImGuiCore/AssetBrowser.hxx"
#include "ImGuiCore/ImGuiManager.hxx"

class SceneEditor
{
public:
    SceneEditor(ImGuiManager &imGuiManager): m_imGuiManager(imGuiManager){};
    void create();

public:
    ImGuiManager &m_imGuiManager;
    ImVec2 offset;
    ImVec2 size;
};

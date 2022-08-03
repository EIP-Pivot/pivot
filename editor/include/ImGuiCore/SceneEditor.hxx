#pragma once

#include <imgui.h>

#include "ImGuiCore/AssetBrowser.hxx"

class SceneEditor
{
public:
    void create();

public:
    ImVec2 offset;
    ImVec2 size;
};

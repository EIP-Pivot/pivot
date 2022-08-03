#pragma once

#include <imgui.h>

#include <pivot/ecs/Core/Data/value.hxx>
#include <pivot/ecs/Core/SceneManager.hxx>
#include <pivot/graphics/AssetStorage.hxx>

class AssetBrowser
{
public:
    struct wrapper {
        AssetBrowser &assetBrowser;
        std::string asset_name;
    };

    AssetBrowser(pivot::graphics::AssetStorage &assetStorage, pivot::ecs::CurrentScene scene)
        : m_assetStorage(assetStorage), m_scene(scene){};
    void create();
    void dropSource(const std::string &name);
    void createEntity(std::string name);

private:
    pivot::graphics::AssetStorage &m_assetStorage;
    pivot::ecs::CurrentScene m_scene;
};
#pragma once

#include <span>

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/array.hxx>
#include <pivot/ecs/Core/Component/description_helpers.hxx>
#include <pivot/ecs/Core/Component/error.hxx>
#include <pivot/ecs/Core/types.hxx>

#include <pivot/builtins/components/Camera.hxx>

namespace pivot::internals
{
class CameraArray : public pivot::ecs::component::DenseTypedComponentArray<builtins::components::Camera>
{
public:
    CameraArray(ecs::component::Description d): DenseTypedComponentArray<builtins::components::Camera>(d)
    {
        m_component_exist.push_back(true);
        m_components.push_back(builtins::components::Camera{});
    }

    std::optional<std::pair<Entity, std::reference_wrapper<const builtins::components::Camera>>>
    getCurrentCamera() const
    {
        if (m_current_camera.has_value() && *m_current_camera < m_components.size() &&
            m_component_exist.at(*m_current_camera)) {
            Entity current_camera = *m_current_camera;
            std::reference_wrapper<const builtins::components::Camera> camera = std::ref(m_components[current_camera]);
            return {{current_camera, camera}};
        }

        return std::nullopt;
    }
    std::optional<std::pair<Entity, std::reference_wrapper<builtins::components::Camera>>> getCurrentCamera()
    {
        if (m_current_camera.has_value() && *m_current_camera < m_components.size() &&
            m_component_exist.at(*m_current_camera)) {
            Entity current_camera = *m_current_camera;
            std::reference_wrapper<builtins::components::Camera> camera = std::ref(m_components[current_camera]);
            return {{current_camera, camera}};
        }

        return std::nullopt;
    }

    void setCurrentCamera(std::optional<Entity> camera)
    {
        if (camera.has_value()) {
            if (*camera < m_components.size() && m_component_exist.at(*camera)) { m_current_camera = camera; }
        } else {
            m_current_camera = std::nullopt;
        }
    }

private:
    std::optional<Entity> m_current_camera;
};
}    // namespace pivot::internals

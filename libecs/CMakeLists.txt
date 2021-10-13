project(pivot-ecs)

include(FetchContent)

add_library(${PROJECT_NAME} source/Systems/ControlSystem.cxx
                            source/Core/ComponentManager.cxx
                            source/Core/Scene.cxx
                            source/Core/EntityManager.cxx
                            source/Core/EventManager.cxx
                            source/Core/SystemManager.cxx
                            source/Core/SceneManager.cxx
)
target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC glm)
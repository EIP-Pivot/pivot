project(pivot-ecs)

include(FetchContent)

add_library(${PROJECT_NAME} source/PhysicsSystem.cxx
)
target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC glm)
project(editor)

add_executable(${PROJECT_NAME} source/main.cxx
                               source/ImGuiCore/ImGuiManager.cxx
                               source/ImGuiCore/Editor.cxx
                               source/ImGuiCore/EntityModule.cxx
                               source/ImGuiCore/ComponentEditor.cxx
                               source/ImGuiCore/SystemsEditor.cxx
)

target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_link_libraries(${PROJECT_NAME} PRIVATE pivot)

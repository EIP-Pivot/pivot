project(editor)

message(STATUS "Looking for lld")
find_program(LLD_FOUND lld)
if (LLD_FOUND)
    message(STATUS "Looking for lld - found")
    set(CMAKE_C_LINKER_FLAGS "-fuse-ld=lld")
    set(CMAKE_CXX_LINKER_FLAGS "-fuse-ld=lld")
    set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
else()
    message(STATUS "Looking for lld - not found")
endif()

add_executable(${PROJECT_NAME} source/main.cxx
                               source/Systems/PhysicsSystem.cxx
                               source/ImGuiCore/ImGuiManager.cxx
                               source/ImGuiCore/Editor.cxx
                               source/ImGuiCore/EntityModule.cxx
                               source/ImGuiCore/ComponentEditor.cxx
                               source/ImGuiCore/SystemsEditor.cxx
)

target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_link_libraries(${PROJECT_NAME} PRIVATE pivot)

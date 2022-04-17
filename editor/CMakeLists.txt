cmake_minimum_required(VERSION 3.20)
project(editor)

include(FetchContent)

FetchContent_Declare(
    nativefiledialog
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG 28ade5a5cc5d17cea8fe4034572cac8fd54eb53f
)
if (NOT nativefiledialog_POPULATED)
    message(STATUS "Populating nativefiledialog")
    FetchContent_Populate(nativefiledialog)
    add_subdirectory(${nativefiledialog_SOURCE_DIR} ${nativefiledialog_BINARY_DIR})
endif()

add_executable(${PROJECT_NAME} source/main.cxx
                               source/ImGuiCore/ImGuiManager.cxx
                               source/ImGuiCore/Editor.cxx
                               source/ImGuiCore/EntityModule.cxx
                               source/ImGuiCore/ComponentEditor.cxx
                               source/ImGuiCore/SystemsEditor.cxx
)


target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_link_libraries(${PROJECT_NAME} PRIVATE pivot nfd)

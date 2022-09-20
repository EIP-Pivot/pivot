cmake_minimum_required(VERSION 3.20)
project(editor)

include(FetchContent)

fetchcontent_declare(
    nativefiledialog
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG 28ade5a5cc5d17cea8fe4034572cac8fd54eb53f
)
fetchcontent_getproperties(nativefiledialog)
if(NOT nativefiledialog_POPULATED)
    message(STATUS "Populating nativefiledialog")
    fetchcontent_populate(nativefiledialog)
    add_subdirectory(${nativefiledialog_SOURCE_DIR} ${nativefiledialog_BINARY_DIR})
    target_compile_definitions(nfd PUBLIC NFD_THROWS_EXCEPTIONS)
endif()


fetchcontent_declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v2.6
)
fetchcontent_getproperties(argparse)
if(NOT argparse_POPULATED)
    message(STATUS "Populating argparse")
    fetchcontent_populate(argparse)
    add_subdirectory(${argparse_SOURCE_DIR} ${argparse_BINARY_DIR})
endif()

add_executable(
    ${PROJECT_NAME}
    source/main.cxx
    source/CmdLineArg.cxx
    source/ImGuiCore/ImGuiManager.cxx
    source/ImGuiCore/Editor.cxx
    source/ImGuiCore/EntityModule.cxx
    source/ImGuiCore/ComponentEditor.cxx
    source/ImGuiCore/SystemsEditor.cxx
    source/ImGuiCore/AssetBrowser.cxx
    source/ImGuiCore/MenuBar.cxx
    source/ImGuiCore/SceneEditor.cxx
    source/ImGuiCore/CustomWidget.cxx
    source/ImGuiCore/ImGuiTheme.cxx
)

target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE PIVOT_EDITOR_ICON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../editor/assets/Editor"
)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall
            -Wextra>
            $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /wd4267
            /wd4201
            /wd4244>
)

target_include_directories(${PROJECT_NAME} PRIVATE include/)

target_link_libraries(${PROJECT_NAME} PRIVATE pivot nfd argparse)

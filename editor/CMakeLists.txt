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
    add_subdirectory(${nativefiledialog_SOURCE_DIR} ${nativefiledialog_BINARY_DIR} EXCLUDE_FROM_ALL)
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
    add_subdirectory(${argparse_SOURCE_DIR} ${argparse_BINARY_DIR} EXCLUDE_FROM_ALL)
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

target_pivot_compile_option(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE pivot nfd argparse)

install(TARGETS ${PROJECT_NAME}
        DESTINATION .
        OPTIONAL)

install(DIRECTORY ${CMAKE_BINARY_DIR}/shaders/ DESTINATION ./shaders)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiConfig/imgui.ini DESTINATION .)

set(CPACK_GENERATOR ZIP)

set(CPACK_PACKAGE_NAME "PivotEditor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PivotEditor")

set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")


include(CPack)

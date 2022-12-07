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

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "pivot-editor" ) 

target_pivot_compile_option(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE pivot nfd argparse)

install(TARGETS ${PROJECT_NAME})

install(DIRECTORY ${CMAKE_BINARY_DIR}/shaders DESTINATION ${CMAKE_INSTALL_DATADIR}/pivot)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_INSTALL_DATADIR}/pivot)

install(FILES ${CMAKE_SOURCE_DIR}/assets/PivotEngine.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiConfig/imgui.ini DESTINATION .)

install(FILES ${CMAKE_SOURCE_DIR}/eu.epitech.eip.pivot.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

set(CPACK_GENERATOR ZIP)

set(CPACK_PACKAGE_NAME "PivotEditor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PivotEditor")

set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")


include(CPack)

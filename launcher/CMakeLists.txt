project(launcher)

add_executable(${PROJECT_NAME} source/main.cxx)

target_link_libraries(${PROJECT_NAME} PRIVATE pivot)

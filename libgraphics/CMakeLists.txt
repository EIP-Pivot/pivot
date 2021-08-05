project(pivot-graphics)

add_library(${PROJECT_NAME} source/lib.cxx)

target_include_directories(${PROJECT_NAME} PUBLIC include/)

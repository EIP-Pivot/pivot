project(pivot-script)

add_library(
    ${PROJECT_NAME} STATIC
    sources/Builtins.cxx
    sources/Engine.cxx
    sources/Interpreter.cxx
    sources/Parser.cxx
    sources/Stack.cxx
)

target_include_directories(${PROJECT_NAME} PUBLIC include/)

target_link_libraries(${PROJECT_NAME} PUBLIC pivot-graphics pivot-ecs)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM pivot-common)

build_tests(${PROJECT_NAME} tests/test_parser.cxx tests/test_interpreter.cxx tests/test_builtins.cxx)

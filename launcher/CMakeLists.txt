project(launcher)

add_executable(${PROJECT_NAME} source/main.cxx)
target_compile_options(
    ${PROJECT_NAME}
    PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall -Wextra>
            $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /wd4267
            /wd4201
            /wd4244>
)

target_link_libraries(${PROJECT_NAME} PRIVATE pivot)

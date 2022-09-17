function(target_pivot_compile_option TARGET)
    target_compile_options(
        ${TARGET}
        PUBLIC $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
               -Wall
               -Wextra>
               $<$<CXX_COMPILER_ID:MSVC>:
               /W4
               /wd4267
               /wd4201
               /wd4244>
    )
    target_compile_features(${TARGET} PRIVATE cxx_std_20)
    target_include_directories(${TARGET} PUBLIC include/)
endfunction()
# ######################################################################################################################
# SHADER COMPILATION #
# ######################################################################################################################
# add_shader(${TARGET} shader.(vert|frag|comp))
function(add_shader TARGET SHADER)
    set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
    set(current-output-path ${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv)

    # Add a custom command to compile GLSL to SPIR-V.
    get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
    file(MAKE_DIRECTORY ${current-output-dir})

    set(GLSLC_EXTRA_ARGS
        ""
        CACHE INTERNAL "" FORCE
    )
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        set(GLSLC_EXTRA_ARGS "-O")
    elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
        set(GLSLC_EXTRA_ARGS "-g")
    else()
        set(GLSLC_EXTRA_ARGS "-O0")
    endif()
    add_custom_command(
        OUTPUT ${current-output-path}
        COMMAND
            ${Vulkan_GLSLC_EXECUTABLE} -fnan-clamp -x glsl --target-env=vulkan1.1
            $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-g> $<$<CONFIG:Release>:-O> $<$<CONFIG:MinSizeRel>:-Os> -o
            ${current-output-path} ${current-shader-path}
        DEPENDS ${current-shader-path}
        IMPLICIT_DEPENDS CXX ${current-shader-path}
        COMMAND_EXPAND_LISTS VERBATIM
    )

    # Make sure our build depends on this output.
    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)

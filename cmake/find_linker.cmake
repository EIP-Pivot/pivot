# Find the best linker available (skip on window)
function(find_linker)
    if(MSVC)
        return()
    endif()

    set(SELECTED_LINKER "ld" CACHE STRING "Used linker")

    message(STATUS "Looking for mold")
    find_program(MOLD_FOUND mold)
    if(MOLD_FOUND)
        message(STATUS "Looking for mold - found")
    else()
        message(STATUS "Looking for mold - not found")
    endif()
    message(STATUS "Looking for lld")
    find_program(LLD_FOUND lld)
    if(LLD_FOUND)
        message(STATUS "Looking for lld - found")
    else()
        message(STATUS "Looking for lld - not found")
    endif()

    if(MOLD_FOUND)
        if(CMAKE_CXX_COMPILER_ID EQUAL GNU AND CMAKE_CXX_COMPILER_VERSION LESS 12.1.0)
            if(LLD_FOUND)
                set(SELECTED_LINKER "lld")
            endif()
        else()
            set(SELECTED_LINKER "mold")
        endif()
    elseif(LLD_FOUND)
        set(SELECTED_LINKER "lld")
    endif()

    set(CMAKE_C_LINKER_FLAGS "${CMAKE_C_LINKER_FLAGS} -fuse-ld=${SELECTED_LINKER}")
    set(CMAKE_CXX_LINKER_FLAGS "${CMAKE_CXX_LINKER_FLAGS} -fuse-ld=${SELECTED_LINKER}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=${SELECTED_LINKER}")
    message(STATUS "Linker used - ${SELECTED_LINKER}")
endfunction(find_linker)
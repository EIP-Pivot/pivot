cmake_minimum_required(VERSION 3.20)
project(pivot)

option(BUILD_DOCUMENTATION "Build the HTML documentation" OFF)
if (BUILD_DOCUMENTATION)
    message(STATUS "Documentation building using Doxygen enabled")
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_WARN_LOGFILE ${CMAKE_BINARY_DIR}/doxygen-warnings.log)
        doxygen_add_docs(doc
            libgraphics/include
            libecs/include
            README.md)
    else()
        message(SEND_ERROR "Doxygen needs to be installed to build the documentation")
    endif()
endif()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(libecs/)
add_subdirectory(libgraphics/)


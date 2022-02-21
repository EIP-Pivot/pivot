# Unit tests building
function(build_tests TARGET)
  if (BUILD_TESTING)
    message(STATUS "Building unit tests for ${PROJECT_NAME}")
    set(TEST_NAME ${TARGET}_test)
    add_executable(${TEST_NAME} ${ARGN})
    target_include_directories(${TEST_NAME} PRIVATE tests/)
    target_link_libraries(${TEST_NAME} PRIVATE Catch2::Catch2WithMain ${TARGET})

      catch_discover_tests(${TEST_NAME})
  endif()
endfunction()

# Unit tests building
function(build_tests TARGET)
  if (BUILD_TESTING)
    message(STATUS "Building unit tests for ${PROJECT_NAME}")
    set(TEST_NAME ${TARGET}_test)
    add_executable(${TEST_NAME} ${ARGN})
    target_include_directories(${TEST_NAME} PRIVATE tests/)
    target_link_libraries(${TEST_NAME} Catch2 ${TARGET})

    add_test(NAME "${TARGET} tests"
      COMMAND ${TEST_NAME})
  endif()
endfunction()

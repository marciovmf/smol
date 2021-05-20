cmake_minimum_required(VERSION 3.9)
project("smol_tests" LANGUAGES CXX VERSION ${SMOL_VERSION})

function(ADD_TEST_EXECUTABLE target sourceList)
  list(APPEND sourceList smol_test.cpp smol_test.h)
  add_executable(${target} ${sourceList})
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
  TARGET_INCLUDE_DIRECTORIES(${target} INTERFACE "${SOURCE_PATH}/include")
  TARGET_INCLUDE_DIRECTORIES(${target} INTERFACE "${SOURCE_PATH}/include/smol")
  TARGET_LINK_LIBRARIES(${target} PRIVATE smol ${OPENGL_LIBRARY})
  ADD_TEST(NAME ${target}_tests COMMAND  test_infrastructure)
endfunction()

if(BUILD_TESTING)
  ADD_TEST_EXECUTABLE(test_infrastructure test_infrastructure_test.cpp)
  # Adding more tests is as simple as giving a name and the list of source files for the test
  #ADD_TEST_EXECUTABLE(foo foo_test.cpp)
  #ADD_TEST_EXECUTABLE(bar bar_test.cpp)
endif()

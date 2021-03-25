cmake_minimum_required(VERSION 3.9)
project("smol_tests" LANGUAGES CXX VERSION ${SMOL_VERSION})

function(ADD_TEST_EXECUTABLE target sourceList)
  list(APPEND sourceList smol_test.cpp smol_test.h)
  add_executable(${target} ${sourceList})
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
  #target_compile_options(${target} PUBLIC "/E")
endfunction()

if(BUILD_TESTING)

  ADD_TEST_EXECUTABLE(test_infrastructure test_infrastructure_test.cpp)
  TARGET_INCLUDE_DIRECTORIES(test_infrastructure INTERFACE "${SOURCE_PATH}/include")
  TARGET_INCLUDE_DIRECTORIES(test_infrastructure INTERFACE "${SOURCE_PATH}/include/smol")
  TARGET_LINK_LIBRARIES(test_infrastructure PRIVATE smol ${OPENGL_LIBRARY})
  ADD_TEST(NAME test_infrastructure_suite COMMAND  test_infrastructure)

endif()

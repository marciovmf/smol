cmake_minimum_required(VERSION 3.13)
set(PROJECT_NAME "objimport")
project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 14)

if (NOT SMOL_PROJECT_ROOT)
  message(FATAL_ERROR "SMOL_PROJECT_ROOT is not set.")
endif()

message(STATUS ${SMOL_PROJECT_ROOT})

# Adding some engine sources because we don't link against smol.lib
set(SOURCE_FILES src/objimport.cpp
  ../../smol_color.cpp
  ../../smol_vector2.cpp
  ../../smol_vector3.cpp
  )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SMOL_PROJECT_ROOT}/include" "${SOURCE_PATH}/include/smol")
target_compile_definitions(${PROJECT_NAME} PRIVATE SMOL_ENGINE_IMPLEMENTATION)
target_link_libraries(${PROJECT_NAME} PRIVATE smol)


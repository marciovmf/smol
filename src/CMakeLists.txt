cmake_minimum_required(VERSION 3.9)
include(CMakeDependentOption)
set(SMOL_VERSION_MAJOR 0)
set(SMOL_VERSION_MINOR 1)
set(SMOL_VERSION_PATCH 1)
set(SMOL_VERSION_TWEAK 0)
set(SMOL_VERSION ${SMOL_VERSION_MAJOR}.${SMOL_VERSION_MINOR}.${SMOL_VERSION_PATCH}.${SMOL_VERSION_TWEAK})
project("smol" LANGUAGES CXX VERSION ${SMOL_VERSION})
find_package(OpenGL REQUIRED)

# General compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
  add_compile_options(/W3 /WX)
endif()

set(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Directory output files single config generators
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/smolSDK/lib 
  CACHE PATH "Output directory for static libraries.")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/smolSDK/lib
  CACHE PATH "Output directory for shared libraries.")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/smolSDK/bin
  CACHE PATH "Output directory for executables and DLL's.")

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/smolSDK")

if(CMAKE_CONFIGURATION_TYPES)
  # Directory output files for multi config generators
  foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${OUTPUT_DIR}/bin" CACHE PATH "" FORCE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${OUTPUT_DIR}/lib" CACHE PATH "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${OUTPUT_DIR}/lib" CACHE PATH "" FORCE)
  endforeach()
else()
  if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
  endif()
  # Directory output files for single config generators
  string(TOUPPER ${CMAKE_BUILD_TYPE} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/smolSDK/bin" CACHE PATH "" FORCE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/smolSDK/lib" CACHE PATH "" FORCE)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/smolSDK/lib" CACHE PATH "" FORCE)
endif()

# Genereate correct smol_version.h
configure_file(${SOURCE_PATH}/include/smol/smol_version.h.in ${SOURCE_PATH}/include/smol/smol_version.h)

#
# smol.dll: Smol engine itself
#
string(TOLOWER ${CMAKE_SYSTEM_NAME} PLATFORM)
if(PLATFORM STREQUAL "windows")
  set(PLATFORM "win64")
endif()

list(APPEND srcs
  # Platform
  "${SOURCE_PATH}/${PLATFORM}/smol_platform_${PLATFORM}.cpp"
  ${SOURCE_PATH}/include/smol/smol_platform.h
  ${SOURCE_PATH}/include/smol/smol_gl.h
  ${SOURCE_PATH}/smol_gl.cpp
  # Core Engine
  ${SOURCE_PATH}/include/smol/smol_point.h
  ${SOURCE_PATH}/include/smol/smol_renderer_types.h
  ${SOURCE_PATH}/include/smol/smol_rect.h
  ${SOURCE_PATH}/smol_rect.cpp
  ${SOURCE_PATH}/include/smol/smol_handle_list.h
  ${SOURCE_PATH}/include/smol/smol_systems_root.h
  ${SOURCE_PATH}/smol_systems_root.cpp
  ${SOURCE_PATH}/include/smol/smol_keyboard.h
  ${SOURCE_PATH}/smol_keyboard.cpp
  ${SOURCE_PATH}/include/smol/smol_mouse.h
  ${SOURCE_PATH}/smol_mouse.cpp
  ${SOURCE_PATH}/include/smol/smol_engine.h
  ${SOURCE_PATH}/include/smol/smol_version.h
  ${SOURCE_PATH}/include/smol/smol.h
  ${SOURCE_PATH}/include/smol/smol_gl.h
  ${SOURCE_PATH}/include/smol/smol_vector2.h
  ${SOURCE_PATH}/smol_vector2.cpp
  ${SOURCE_PATH}/include/smol/smol_vector3.h
  ${SOURCE_PATH}/smol_vector3.cpp
  ${SOURCE_PATH}/include/smol/smol_vector4.h
  ${SOURCE_PATH}/smol_vector4.cpp
  ${SOURCE_PATH}/include/smol/smol_transform.h
  ${SOURCE_PATH}/smol_transform.cpp
  ${SOURCE_PATH}/include/smol/smol_color.h
  ${SOURCE_PATH}/smol_color.cpp
  ${SOURCE_PATH}/include/smol/smol_mat4.h
  ${SOURCE_PATH}/smol_mat4.cpp
  ${SOURCE_PATH}/include/smol/smol_resource_manager.h
  ${SOURCE_PATH}/smol_resource_manager.cpp
  ${SOURCE_PATH}/include/smol/smol_arena.h
  ${SOURCE_PATH}/smol_arena.cpp
  ${SOURCE_PATH}/include/smol/smol_log.h
  ${SOURCE_PATH}/smol_log.cpp
  ${SOURCE_PATH}/include/smol/smol_renderer.h
  ${SOURCE_PATH}/smol_renderer_gl.cpp
  ${SOURCE_PATH}/include/smol/smol_scene_nodes.h
  ${SOURCE_PATH}/smol_scene_nodes.cpp
  ${SOURCE_PATH}/include/smol/smol_scene.h
  ${SOURCE_PATH}/smol_scene.cpp
  ${SOURCE_PATH}/include/smol/smol_cfg_parser.h
  ${SOURCE_PATH}/smol_cfg_parser.cpp
  ${SOURCE_PATH}/include/smol/smol_mesh_data.h
  ${SOURCE_PATH}/smol_mesh_data.cpp
  "${SMOL_WINDOWS_RESOURCE_FILE}")

add_library(smol SHARED ${srcs})
#set(UNITY_BUILD ON)
#set_target_properties(smol PROPERTIES UNITY_BUILD_MODE GROUP)
#set_source_files_properties(${srcs} PROPERTIES UNITY_BUILD_MODE "smol_group_1")

set_target_properties(smol PROPERTIES VERSION ${SMOL_VERSION})
target_compile_definitions(smol PRIVATE SMOL_ENGINE_IMPLEMENTATION)
target_include_directories(smol PUBLIC 
  "${SOURCE_PATH}/include" 
  "${SOURCE_PATH}/include/smol")

target_link_libraries(smol PRIVATE ${OPENGL_LIBRARY})

if(SMOL_EXPORT_SDK)
add_custom_command(TARGET smol POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}/include
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_PATH}/include/smol/ ${OUTPUT_DIR}/include)
endif()

#
# build demo game
#
add_library(game SHARED "${SOURCE_PATH}/demo/game.cpp")
target_include_directories(game PUBLIC 
  "${SOURCE_PATH}/include" 
  "${SOURCE_PATH}/include/smol")
target_link_libraries(game PRIVATE smol)

#
# Copy assets
#
add_custom_target(assets ALL)
add_custom_command(TARGET assets POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Coppying assets..."
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)

#
# slauncher.exe: Editor / Game launcher
#
add_subdirectory("Launcher")


# Build tests
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory("tests")
endif()

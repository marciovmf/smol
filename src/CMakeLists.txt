cmake_minimum_required(VERSION 3.9)
include(CMakeDependentOption)
set(SMOL_VERSION_MAJOR 0)
set(SMOL_VERSION_MINOR 0)
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

if(CMAKE_CONFIGURATION_TYPES)
  # Directory output files for multi config generators
  foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/smolSDK/bin" CACHE PATH "" FORCE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/smolSDK/lib" CACHE PATH "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/smolSDK/lib" CACHE PATH "" FORCE)
  endforeach()
else()
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

add_library(smol SHARED 
  # Platform
  "${SOURCE_PATH}/${PLATFORM}/smol_platform_${PLATFORM}.cpp"
  ${SOURCE_PATH}/include/smol/smol_platform.h
  ${SOURCE_PATH}/include/smol/smol_gl.h
  ${SOURCE_PATH}/smol_gl.cpp
  # Core Engine
  ${SOURCE_PATH}/smol_keyboard.cpp
  ${SOURCE_PATH}/include/smol/smol_keyboard.h
  ${SOURCE_PATH}/include/smol/smol_engine.h
  ${SOURCE_PATH}/include/smol/smol_version.h
  ${SOURCE_PATH}/include/smol/smol.h
  ${SOURCE_PATH}/include/smol/smol_gl.h
  ${SOURCE_PATH}/include/smol/smol_vector2.h
  ${SOURCE_PATH}/smol_vector2.cpp
  ${SOURCE_PATH}/include/smol/smol_vector3.h
  ${SOURCE_PATH}/smol_vector3.cpp
  ${SOURCE_PATH}/include/smol/smol_mat4.h
  ${SOURCE_PATH}/smol_mat4.cpp
  ${SOURCE_PATH}/include/smol/smol_assetmanager.h
  ${SOURCE_PATH}/smol_assetmanager.cpp
  ${SOURCE_PATH}/include/smol/smol_log.h
  ${SOURCE_PATH}/smol_log.cpp
  ${SOURCE_PATH}/include/smol/smol_renderer.h
  ${SOURCE_PATH}/smol_renderer_gl.cpp
  "${SMOL_WINDOWS_RESOURCE_FILE}")

set_target_properties(smol PROPERTIES VERSION ${SMOL_VERSION})
target_compile_definitions(smol PRIVATE SMOL_ENGINE_IMPLEMENTATION)
target_include_directories(smol PUBLIC 
  "${SOURCE_PATH}/include" 
  "${SOURCE_PATH}/include/smol")

target_link_libraries(smol PRIVATE ${OPENGL_LIBRARY})

if(SMOL_EXPORT_SDK)
add_custom_command(TARGET smol POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/smolSDK/include
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_PATH}/include/smol/ ${CMAKE_BINARY_DIR}/smolSDK/include)
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
# slauncher.exe: Editor / Game launcher
#
add_subdirectory("Launcher")

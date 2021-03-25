cmake_minimum_required(VERSION 3.9)
include(CMakeDependentOption)
set(SMOL_VERSION_MAJOR 0)
set(SMOL_VERSION_MINOR 0)
set(SMOL_VERSION_PATCH 1)
set(SMOL_VERSION_TWEAK 0)
set(SMOL_VERSION ${SMOL_VERSION_MAJOR}.${SMOL_VERSION_MINOR}.${SMOL_VERSION_PATCH}.${SMOL_VERSION_TWEAK})
cmake_policy(SET CMP0048 NEW)
project("smol" LANGUAGES CXX VERSION ${SMOL_VERSION})
find_package(OpenGL REQUIRED)

# General compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Valid platforms
set(PLATFORM win64 CACHE STRING "Target platform" FORCE)
set_property(CACHE PLATFORM PROPERTY STRINGS win64)

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
# Add windows resource files only if we are building for windows
#
if("${PLATFORM}" STREQUAL win64)
  set(SMOL_WINDOWS_RESOURCE_FILE "${SOURCE_PATH}/win64/smol_resource_win64.rc")
endif()

#
# smol.dll: Smol engine itself
#
add_library(smol SHARED 
  # Platform
  "${SOURCE_PATH}/${PLATFORM}/smol_platform_${PLATFORM}.cpp"
  ${SOURCE_PATH}/include/smol/smol_platform.h
  ${SOURCE_PATH}/include/smol/smol_gl.h
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
  "${SMOL_WINDOWS_RESOURCE_FILE}")

set_target_properties(smol PROPERTIES VERSION ${SMOL_VERSION})
target_compile_options(smol PRIVATE -DSMOL_ENGINE_IMPLEMENTATION)
target_include_directories(smol PUBLIC 
  "${SOURCE_PATH}/include" 
  "${SOURCE_PATH}/include/smol")

target_link_libraries(smol PRIVATE ${OPENGL_LIBRARY})
add_custom_command(TARGET smol POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/smolSDK/include
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_PATH}/include/smol/ ${CMAKE_BINARY_DIR}/smolSDK/include)

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
add_executable(slauncher ${SOURCE_PATH}/smol_launcher.cpp "${SMOL_WINDOWS_RESOURCE_FILE}")
target_include_directories(slauncher INTERFACE "${SOURCE_PATH}/include")
target_include_directories(slauncher INTERFACE "${SOURCE_PATH}/include/smol")
target_link_libraries(slauncher PRIVATE smol ${OPENGL_LIBRARY})

# Make windows Debug builds to have a console subsystem
if(CMAKE_CONFIGURATION_TYPES)
  set_property(TARGET slauncher PROPERTY LINK_OPTIONS
    $<$<CXX_COMPILER_ID:MSVC>:$<IF:$<CONFIG:Release>,/SUBSYSTEM:WINDOWS,/SUBSYSTEM:CONSOLE>>)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release" )
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET slauncher PROPERTY LINK_OPTIONS /SUBSYSTEM:WINDOWS)
  endif()
endif()

# build tests

include(CTest)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()


install(DIRECTORY "${CMAKE_BINARY_DIR}/smolSDK" DESTINATION .)


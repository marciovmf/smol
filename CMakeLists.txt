cmake_minimum_required(VERSION 3.18)
project("smol" LANGUAGES CXX)

# General compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the output folder where your program will be created
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Valid platforms
set(PLATFORM win64 CACHE STRING "Target platform" FORCE)
set_property(CACHE PLATFORM PROPERTY STRINGS win64)

if (MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  add_compile_options(/W3 /WX)
endif()

set(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

#
# smol_platform.lib: OS dependent code implementation of the platform layer
#
add_library(smol_platform SHARED "${SOURCE_PATH}/${PLATFORM}/smol_platform_${PLATFORM}.cpp")
target_include_directories(smol_platform PUBLIC "${SOURCE_PATH}/include")
target_compile_options(smol_platform PRIVATE -DSMOL_PLATFORM_IMPLEMENTATION)

#
# smol.dll: Smol engine itself
#
add_library(smol SHARED 
  ${SOURCE_PATH}/smol_engine.cpp)
target_include_directories(smol PUBLIC "${SOURCE_PATH}/include")
target_compile_options(smol PRIVATE -DSMOL_ENGINE_IMPLEMENTATION)
target_link_libraries(smol PRIVATE smol_platform)

#
# slauncher.exe: Editor / Game launcher
#
add_executable(slauncher WIN32 "${SOURCE_PATH}/smol_launcher.cpp")
target_link_libraries(slauncher PRIVATE smol smol_platform)

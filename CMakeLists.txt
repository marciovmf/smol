cmake_minimum_required(VERSION 3.18)
project("smol" LANGUAGES CXX)

# General compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Valid platforms
set(PLATFORM win64 CACHE STRING "Target platform" FORCE)
set_property(CACHE PLATFORM PROPERTY STRINGS win64)

if (MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  add_compile_options(/W3 /WX)
endif()

set(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Directory output files single config generators
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/smolSDK/lib 
  CACHE PATH "Output directory for static libraries.")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/smolSDK/lib
  CACHE PATH "Output directory for shared libraries.")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/smolSDK/bin
  CACHE PATH "Output directory for executables and DLL's.")

# Directory output files for multi config generators
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/smolSDK/bin" CACHE PATH "" FORCE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/smolSDK/lib" CACHE PATH "" FORCE)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/smolSDK/lib" CACHE PATH "" FORCE)
endforeach()

#
# smol_platform.dll: OS dependent code implementation of the platform layer
#
add_library(smol_platform SHARED "${SOURCE_PATH}/${PLATFORM}/smol_platform_${PLATFORM}.cpp")
target_include_directories(smol_platform PUBLIC "${SOURCE_PATH}/include")
target_compile_options(smol_platform PRIVATE -DSMOL_PLATFORM_IMPLEMENTATION)

#
# smol.dll: Smol engine itself
#
add_library(smol SHARED ${SOURCE_PATH}/smol_engine.cpp)
target_include_directories(smol PUBLIC "${SOURCE_PATH}/include")
target_compile_options(smol PRIVATE -DSMOL_ENGINE_IMPLEMENTATION)
target_link_libraries(smol PRIVATE smol_platform)
add_custom_command(TARGET smol POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/smolSDK/include
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_PATH}/include/smol/ ${CMAKE_BINARY_DIR}/smolSDK/include)

#
# slauncher.exe: Editor / Game launcher
#
add_executable(slauncher WIN32 "${SOURCE_PATH}/smol_launcher.cpp")
target_link_libraries(slauncher PRIVATE smol smol_platform)

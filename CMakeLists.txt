cmake_minimum_required(VERSION 3.14)
#project("smol")
project("smol"LANGUAGES CXX)

# General compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the output folder where your program will be created
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# General build type options
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Build type" FORCE)

# By default, we are building a DEBUG build

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_MACRO)
add_definitions(-D ${BUILD_TYPE_MACRO}) 

if (MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  add_compile_options(/W3 /WX /INCREMENTAL)
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(WINDOWS_SUBSYSTEM_BUILD WIN32)
  endif()
endif()


set(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

#
# Platform dll
#
set(PLATFORM_PATH "${SOURCE_PATH}/platform")
add_library(smol_platform SHARED "${PLATFORM_PATH}/smol_platform.cpp")
target_include_directories(smol_platform PUBLIC "${SOURCE_PATH}/include")

#
# Engine
#
add_executable(smol ${WINDOWS_SUBSYSTEM_BUILD} "${SOURCE_PATH}/smol_main.cpp")
target_link_libraries(smol PRIVATE smol_platform)

